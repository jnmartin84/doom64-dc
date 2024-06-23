/* am_main.c -- automap */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

#define COLOR_RED     0xA40000FF
#define COLOR_GREEN   0x00C000FF
#define COLOR_BROWN   0x8A5C30ff
#define COLOR_YELLOW  0xCCCC00FF
#define COLOR_GREY    0x808080FF
#define COLOR_AQUA    0x3373B3FF

#define MAXSCALE	1500
#define MINSCALE	200

fixed_t am_box[4]; // 80063110
int am_plycolor;    // 80063120
int am_plyblink;    // 80063124

#define LINEWIDTH 2.0f

extern boolean M_BoxIntersect(fixed_t a[static 4], fixed_t b[static 4]);

void AM_DrawSubsectors(player_t *player, fixed_t cx, fixed_t cy, fixed_t bbox[static 4]);
void AM_DrawThings(fixed_t x, fixed_t y, angle_t angle, int color);
void AM_DrawLine(player_t *player, fixed_t bbox[static 4]);
void AM_DrawLineThings(fixed_t x, fixed_t y, angle_t angle, int color);


/*================================================================= */
/* */
/* Start up Automap */
/* */
/*================================================================= */
pvr_poly_hdr_t line_hdr;
pvr_poly_cxt_t line_cxt;

pvr_poly_hdr_t thing_hdr;
pvr_poly_cxt_t thing_cxt;
pvr_vertex_t __attribute__((aligned(32))) thing_verts[3];
pvr_vertex_t __attribute__((aligned(32))) line_verts[4];
int ever_started = 0;

void AM_Start(void) // 800004D8
{
	am_plycolor = 95;
	am_plyblink = 16;
}

/*
==================
=
= AM_Control
=
= Called by P_PlayerThink before any other player processing
=
= Button bits can be eaten by clearing them in ticbuttons[playernum]
==================
*/

#define MAXSENSIVITY    10
extern int last_joyx,last_joyy;
void AM_Control (player_t *player) // 800004F4
{
	int buttons, oldbuttons;

	buttons_t   *cbuttons;
	fixed_t     block[8];
	angle_t     angle;
	fixed_t     fs, fc;
	fixed_t     x, y, x1, y1, x2, y2;
	int         scale, sensitivity;
	int         i;

	if (gamepaused)
		return;

	cbuttons = BT_DATA[0];
	buttons = ticbuttons[0];
	oldbuttons = oldticbuttons[0];

	if (player->playerstate != PST_LIVE)
	{
		am_plycolor = 79;
		return;
	}

	if ((buttons & cbuttons->BT_MAP) && !(oldbuttons & cbuttons->BT_MAP))
	{
		if(player->automapflags & AF_SUBSEC)
		{
			player->automapflags &= ~AF_SUBSEC;
			player->automapflags |= AF_LINES;
		}
		else if(player->automapflags & AF_LINES)
		{
			player->automapflags &= ~AF_LINES;
		}
		else
		{
			player->automapflags |= AF_SUBSEC;
		}

		player->automapx = player->mo->x;
		player->automapy = player->mo->y;
	}

	if(!(player->automapflags & (AF_LINES|AF_SUBSEC)))
		return;

	/* update player flash */
	am_plycolor = (unsigned int)(am_plycolor + am_plyblink);
	if(am_plycolor < 80 || (am_plycolor >= 255))
	{
		am_plyblink = -am_plyblink;
	}

	if (!(buttons & cbuttons->BT_USE))
	{
		player->automapflags &= ~AF_FOLLOW;
		return;
	}

	if (!(player->automapflags & AF_FOLLOW))
	{
		player->automapflags |= AF_FOLLOW;
		player->automapx = player->mo->x;
		player->automapy = player->mo->y;

		M_ClearBox(am_box);

		block[2] = block[4] = (bmapwidth << 23 ) + bmaporgx;
		block[1] = block[3] = (bmapheight << 23) + bmaporgy;
		block[0] = block[6] = bmaporgx;
		block[5] = block[7] = bmaporgy;

		angle = (ANG90 - player->mo->angle) >> ANGLETOFINESHIFT;

		fs = finesine[angle];
		fc = finecosine[angle];

		for(i = 0; i < 8; i+=2)
		{
			x = (block[i]   - player->automapx) >> FRACBITS;
			y = (block[i+1] - player->automapy) >> FRACBITS;

			x1 = (x * fc);
			y1 = (y * fs);
			x2 = (x * fs);
			y2 = (y * fc);

			x = (x1 - y1) + player->automapx;
			y = (x2 + y2) + player->automapy;

			M_AddToBox(am_box, x, y);
		}
	}

	if (!(player->automapflags & AF_FOLLOW))
		return;

	scale = player->automapscale << 15;
	scale = (scale / 1500) << 8;

	/* Analyze analog stick movement (left / right) */
	sensitivity = last_joyx;//(int)(((buttons & 0xff00) >> 8) << 24) >> 24;

	if(sensitivity >= MAXSENSIVITY || sensitivity <= -MAXSENSIVITY)
	{
		player->automapx += (sensitivity * scale) / 80;
	}

	/* Analyze analog stick movement (up / down) */
	sensitivity = last_joyy;//(int)((buttons) << 24) >> 24;

	if(sensitivity >= MAXSENSIVITY || sensitivity <= -MAXSENSIVITY)
	{
		player->automapy += (sensitivity * scale) / 80;
	}

	/* X movement */
	if (player->automapx > am_box[BOXRIGHT])
	{
		player->automapx = am_box[BOXRIGHT];
	}
	else if (player->automapx < am_box[BOXLEFT])
	{
		player->automapx = am_box[BOXLEFT];
	}

	/* Y movement */
	if (player->automapy > am_box[BOXTOP])
	{
		player->automapy = am_box[BOXTOP];
	}
	else if (player->automapy < am_box[BOXBOTTOM])
	{
		player->automapy = am_box[BOXBOTTOM];
	}

	/* Zoom scale in */
	if (buttons & PAD_L_TRIG)
	{
		player->automapscale -= 32;

		if (player->automapscale < MINSCALE)
			player->automapscale = MINSCALE;
	}

	/* Zoom scale out */
	if (buttons & PAD_R_TRIG)
	{
		player->automapscale += 32;

		if (player->automapscale > MAXSCALE)
			player->automapscale = MAXSCALE;
	}

	ticbuttons[0] &= ~(cbuttons->BT_LEFT | cbuttons->BT_RIGHT |
					   cbuttons->BT_FORWARD | cbuttons->BT_BACK |
					   PAD_L_TRIG | PAD_R_TRIG | 0xffff);
}

/*
==================
=
= AM_Drawer
=
= Draws the current frame to workingscreen
==================
*/
extern Matrix __attribute__((aligned(32))) R_ProjectionMatrix;

void AM_Drawer (void) // 800009AC
{
	player_t	*p;
	mobj_t		*mo;
	mobj_t		*next;
	fixed_t		xpos, ypos;
	fixed_t		ox, oy;
	fixed_t     c;
	fixed_t     s;
	angle_t     angle;
	int			color;
	int			scale;
	int         artflag;
	char        map_name[48];
	char		killcount[20]; // [Immorpher] Automap kill count
	char		itemcount[20]; // [Immorpher] Automap item count
	char		secretcount[20]; // [Immorpher] Automap secret count
	fixed_t     screen_box[4];
	fixed_t     boxscale;

	if (!ever_started) {
		pvr_poly_cxt_col(&thing_cxt, PVR_LIST_TR_POLY);
		pvr_poly_compile(&thing_hdr, &thing_cxt);
		for (int vn = 0; vn < 3; vn++) {
			thing_verts[vn].flags = PVR_CMD_VERTEX;
			thing_verts[vn].oargb = 0;
		}
		thing_verts[2].flags = PVR_CMD_VERTEX_EOL;
		ever_started = 1;
	}

	pvr_set_bg_color(0,0,0);

	p = &players[0];

	scale = (p->automapscale << 16);
	xpos = p->mo->x;
	ypos = p->mo->y;

	if (p->onground) {
		xpos += (quakeviewx >> 7);
		ypos += quakeviewy;
	}

	if (p->automapflags & AF_FOLLOW) {
		angle = (p->mo->angle + ANG270) >> ANGLETOFINESHIFT;
		ox = (p->automapx - xpos) >> 16;
		oy = (p->automapy - ypos) >> 16;
		xpos += ((ox * finecosine[angle]) - (oy * finesine[angle]));
		ypos += ((ox * finesine[angle]));
	}

	angle = p->mo->angle >> ANGLETOFINESHIFT;

	s = finesine[angle];
	c = finecosine[angle];

	Matrix __attribute__((aligned(32))) RotX;
	DoomRotateX(RotX, -1.0, 0.0); // -pi/2 rad

	Matrix __attribute__((aligned(32))) RotY;
	DoomRotateY(RotY, (float)s/65536.0f, (float)c/65536.0f);

	Matrix __attribute__((aligned(32))) ThenTrans;
	DoomTranslate(ThenTrans, -((float)xpos/65536.0f), -((float)scale/65536.0f), (float)ypos/65536.0f);

	mat_load(&R_ProjectionMatrix);
	mat_apply(&RotX);
	mat_apply(&RotY);
	mat_apply(&ThenTrans);

	boxscale = scale / 160;

	{
		fixed_t ts, tc;
		fixed_t cx, cy, tx, x, y;
		angle_t thingangle;

		thingangle = (ANG90 - p->mo->angle) >> ANGLETOFINESHIFT;
		ts = finesine[thingangle];
		tc = finecosine[thingangle];

		cx = FixedMul(320<<(FRACBITS-1), boxscale);
		cy = FixedMul(240<<(FRACBITS-1), boxscale);

		M_ClearBox(screen_box);

		for (int i = 0; i < 2; i++)
		{
			tx = i ? -cx : cx;
			x = ((s64) tx * (s64) tc + (s64) cy * (s64) ts) >> FRACBITS;
			y = ((s64) -tx * (s64) ts + (s64) cy * (s64) tc) >> FRACBITS;
			M_AddToBox(screen_box, x, y);
			M_AddToBox(screen_box, -x, -y);
		}

		screen_box[BOXTOP] += ypos;
		screen_box[BOXBOTTOM] += ypos;
		screen_box[BOXLEFT] += xpos;
		screen_box[BOXRIGHT] += xpos;
	}

	if (p->automapflags & AF_LINES) {
		AM_DrawLine(p, screen_box);
	} else {
		AM_DrawSubsectors(p, xpos, ypos, screen_box);
	}

	/* SHOW ALL MAP THINGS (CHEAT) */
	if (p->cheats & CF_ALLMAP) {
		for (mo = mobjhead.next; mo != &mobjhead; mo = next) {
			fixed_t bbox[4];
			next = mo->next;

			if (mo == p->mo)
				continue;  /* Ignore player */

			if (mo->flags & (MF_NOSECTOR|MF_RENDERLASER))
				continue;

			if (mo->flags & (MF_SHOOTABLE|MF_MISSILE))
				color = COLOR_RED;
			else
				color = COLOR_AQUA;

			bbox[BOXTOP   ] = mo->y + 0x2d413c; // sqrt(2) * 32;
			bbox[BOXBOTTOM] = mo->y - 0x2d413c;
			bbox[BOXRIGHT ] = mo->x + 0x2d413c;
			bbox[BOXLEFT  ] = mo->x - 0x2d413c;

			if (!M_BoxIntersect(bbox, screen_box))
				continue;

			if (p->automapflags & AF_LINES) {
				AM_DrawLineThings(mo->x, mo->y, mo->angle, color);
			} else {
				AM_DrawThings(mo->x, mo->y, mo->angle, color);
			}
		}
	}


	if (p->automapflags & AF_LINES) {
		/* SHOW PLAYERS */
		AM_DrawLineThings(p->mo->x, p->mo->y, p->mo->angle, am_plycolor << 16 | 0xff);
	} else {
		/* SHOW PLAYERS */
		AM_DrawThings(p->mo->x, p->mo->y, p->mo->angle, am_plycolor << 16 | 0xff);
	}

	if (enable_messages) {
		if (p->messagetic <= 0) {
			sprintf(map_name, "LEVEL %d: %s", gamemap, MapInfo[gamemap].name);
			ST_Message(2+HUDmargin,HUDmargin, map_name, 196 | 0xffffff00);
		} else {
			ST_Message(2+HUDmargin,HUDmargin, p->message, 196 | p->messagecolor);
		}
	}

	// [Immorpher] kill count
	if(MapStats) {
		sprintf(killcount, "KILLS: %d/%d", players[0].killcount, totalkills);
		ST_Message(2+HUDmargin, 212-HUDmargin, killcount, 196 | 0xffffff00);
		sprintf(itemcount, "ITEMS: %d/%d", players[0].itemcount, totalitems);
		ST_Message(2+HUDmargin, 222-HUDmargin, itemcount, 196| 0xffffff00);
		sprintf(secretcount, "SECRETS: %d/%d", players[0].secretcount, totalsecret);
		ST_Message(2+HUDmargin, 232-HUDmargin, secretcount, 196 | 0xffffff00);
	}

	xpos = 297-HUDmargin;
	artflag = 4;
	do
	{
		if ((players->artifacts & artflag) != 0) {
			if (artflag == 4) {
				BufferedDrawSprite(MT_ITEM_ARTIFACT3, &states[S_559], 0, 0xffffff80, xpos, 266-HUDmargin);
			} else if (artflag == 2) {
				BufferedDrawSprite(MT_ITEM_ARTIFACT2, &states[S_551], 0, 0xffffff80, xpos, 266-HUDmargin);
			} else if (artflag == 1) {
				BufferedDrawSprite(MT_ITEM_ARTIFACT1, &states[S_543], 0, 0xffffff80, xpos, 266-HUDmargin);
			}

			xpos -= 40;
		}
		artflag >>= 1;
	} while (artflag != 0);
}

void R_RenderPlane(leaf_t *leaf, int numverts, 
int zpos, int texture, int xpos, int ypos, 
int color, int ceiling, int lightlevel, int alpha);

static boolean AM_DrawSubsector(player_t *player, int bspnum)
{
	subsector_t *sub;
	sector_t *sec;

	if(!(bspnum & NF_SUBSECTOR))
		return false;

	sub = &subsectors[bspnum & (~NF_SUBSECTOR)];

	if(!sub->drawindex && !player->powers[pw_allmap] && !(player->cheats & CF_ALLMAP))
		return true;

	sec = sub->sector;

	if((sec->flags & MS_HIDESSECTOR) || (sec->floorpic == -1))
		return true;

	R_RenderPlane(&leafs[sub->leaf], sub->numverts, 0,
				  textures[sec->floorpic], 0, 0,
				  lights[sec->colors[1]].rgba, 0, sec->lightlevel, 255);

	return true;
}
/*
==================
=
= AM_DrawSubsectors
=
==================
*/
// Nova took advantage of the GBA Doom stack rendering to improve the automap rendering speed
#define MAX_BSP_DEPTH 128

void AM_DrawSubsectors(player_t *player, fixed_t cx, fixed_t cy, fixed_t bbox[static 4]) // 800012A0
{
	int sp = 0;
	node_t *bsp;
	int     side;
	fixed_t    dx, dy;
	fixed_t    left, right;
	int bspnum = numnodes - 1;
	int bspstack[MAX_BSP_DEPTH];

	globallump = -1;

	while(true) {
		while (!AM_DrawSubsector(player, bspnum)) {
			if(sp == MAX_BSP_DEPTH) {
				break;
			}
			
			bsp = &nodes[bspnum];
			dx = (cx - bsp->line.x);
			dy = (cy - bsp->line.y);

			left = (bsp->line.dy >> 16) * (dx >> 16);
			right = (dy >> 16) * (bsp->line.dx >> 16);

			if (right < left) {
				side = 0;        /* front side */
			} else {
				side = 1;        /* back side */
			}

			bspstack[sp++] = bspnum;
			bspstack[sp++] = side;

			bspnum = bsp->children[side];

		}

		if(sp == 0) {
			//back at root node and not visible. All done!
			return;
		}

		//Back sides.
		side = bspstack[--sp];
		bspnum = bspstack[--sp];
		bsp = &nodes[bspnum];

		// Possibly divide back space.
		//Walk back up the tree until we find
		//a node that has a visible backspace.
		while(!M_BoxIntersect (bbox, bsp->bbox[side^1])) {
			if (sp == 0) {
				//back at root node and not visible. All done!
				return;
			}

			//Back side next.
			side = bspstack[--sp];
			bspnum = bspstack[--sp];

			bsp = &nodes[bspnum];
		}

		bspnum = bsp->children[side^1];
	}
}

/*
==================
=
= AM_DrawLine
=
==================
*/
extern d64Vertex_t *dVTX[4];
extern d64Triangle_t dT1, dT2;

void AM_DrawLineThings(fixed_t x, fixed_t y, angle_t angle, int color) {
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_t cxt;
	pvr_vertex_t __attribute__((aligned(32))) verts[12];
	pvr_vertex_t *vert = verts;

	for (int vn = 0; vn < 12; vn++) {
		verts[vn].flags = PVR_CMD_VERTEX;
		verts[vn].argb = D64_PVR_REPACK_COLOR(color);
		verts[vn].oargb = 0;
	}
	verts[3].flags = PVR_CMD_VERTEX_EOL;
	verts[7].flags = PVR_CMD_VERTEX_EOL;
	verts[11].flags = PVR_CMD_VERTEX_EOL;
	pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
	pvr_poly_compile(&hdr, &cxt);

	dVTX[0] = &(dT1.dVerts[0]);
	dVTX[1] = &(dT1.dVerts[1]);
	dVTX[2] = &(dT1.dVerts[2]);

	angle_t ang;

	ang = (angle) >> ANGLETOFINESHIFT;

	dVTX[0]->v.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	dVTX[0]->v.y = 0.0f;
	dVTX[0]->v.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);

	ang = (angle + 0xA0000000) >> ANGLETOFINESHIFT;

	dVTX[1]->v.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	dVTX[1]->v.y = 0.0f;
	dVTX[1]->v.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);	

	ang = (angle + 0x60000000) >> ANGLETOFINESHIFT;

	dVTX[2]->v.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	dVTX[2]->v.y = 0.0f;
	dVTX[2]->v.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);	

	transform_vert(dVTX[0]);
	transform_vert(dVTX[1]);
	transform_vert(dVTX[2]);
	perspdiv(dVTX[0]);
	perspdiv(dVTX[1]);
	perspdiv(dVTX[2]);	

	d64Vertex_t *v0,*v1;
	int lvert = 0;
	int lhori = 0;

	pvr_list_prim(PVR_LIST_OP_POLY, &hdr, sizeof(hdr));
	{
				int x1,y1,x2,y2;
				
				if ((int)(dVTX[0]->v.x) == (int)(dVTX[1]->v.x)) {
					lvert = 1;
					if((int)(dVTX[0]->v.y) > (int)(dVTX[1]->v.y)) {
						v0 = dVTX[1];
						v1 = dVTX[0];
					} else {
						v0 = dVTX[0];
						v1 = dVTX[1];
					}
				} else if ((int)(dVTX[0]->v.y) == (int)(dVTX[1]->v.y)) {
					lhori = 1;
					if((int)(dVTX[0]->v.x) < (int)(dVTX[1]->v.x)) {					
						v0 = dVTX[0];
						v1 = dVTX[1];
					} else {
						v0 = dVTX[1];
						v1 = dVTX[0];
					}
				} else if((int)(dVTX[0]->v.x) < (int)(dVTX[1]->v.x)) {
						v0 = dVTX[0];
						v1 = dVTX[1];
				} else if((int)(dVTX[0]->v.x) > (int)(dVTX[1]->v.x)) {
						v0 = dVTX[1];
						v1 = dVTX[0];
				}
				
				x1 = v0->v.x;
				y1 = v0->v.y;

				x2 = v1->v.x;
				y2 = v1->v.y;
				
				//https://dcemulation.org/index.php?title=PowerVR_Introduction
				//thank you bluecrab thank you blackaura
				if (lhori) {
					x1 -= (LINEWIDTH*0.5f);
					x2 -= (LINEWIDTH*0.5f);
					y1 -= (LINEWIDTH*0.5f);
					y2 -= (LINEWIDTH*0.5f);
					
					vert->x = x1;
					vert->y = y1 + LINEWIDTH;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x1;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;

					vert->x = x2;
					vert->y = y2 + LINEWIDTH;
					vert->z = v1->v.z;
					vert++;

					vert->x = x2;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;
				} else if (lvert) {
					x1 -= (LINEWIDTH*0.5f);
					x2 -= (LINEWIDTH*0.5f);
					y1 -= (LINEWIDTH*0.5f);
					y2 -= (LINEWIDTH*0.5f);

					vert->x = x2;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;

					vert->x = x1;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x2 + LINEWIDTH;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;
					
					vert->x = x1 + LINEWIDTH;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;
				} else {
					// https://devcry.heiho.net/html/2017/20170820-opengl-line-drawing.html
					float dx = x2 - x1;
					float dy = y2 - y1;

					float hlw_invmag = frsqrt((dx*dx) + (dy*dy)) * (LINEWIDTH*0.5f);
					float nx = -dy * hlw_invmag;
					float ny = dx * hlw_invmag;

					vert->x = x1 + nx;
					vert->y = y1 + ny;
					vert->z = v0->v.z;
					vert++;

					vert->x = x1 - nx;
					vert->y = y1 - ny;
					vert->z = v1->v.z;
					vert++;
					
					vert->x = x2 + nx;
					vert->y = y2 + ny;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x2 - nx;
					vert->y = y2 - ny;
					vert->z = v1->v.z;
					vert++;
				}
			}
			lvert = 0;
			lhori = 0;
			{
				int x1,y1,x2,y2;
				
				if ((int)(dVTX[1]->v.x) == (int)(dVTX[2]->v.x)) {
					lvert = 1;
					if((int)(dVTX[1]->v.y) > (int)(dVTX[2]->v.y)) {
						v0 = dVTX[2];
						v1 = dVTX[1];
					} else {
						v0 = dVTX[1];
						v1 = dVTX[2];
					}
				} else if ((int)(dVTX[1]->v.y) == (int)(dVTX[2]->v.y)) {
					lhori = 1;
					if((int)(dVTX[1]->v.x) < (int)(dVTX[2]->v.x)) {					
						v0 = dVTX[1];
						v1 = dVTX[2];
					} else {
						v0 = dVTX[2];
						v1 = dVTX[1];
					}
				} else if((int)(dVTX[1]->v.x) < (int)(dVTX[2]->v.x)) {
						v0 = dVTX[1];
						v1 = dVTX[2];
				} else if((int)(dVTX[1]->v.x) > (int)(dVTX[2]->v.x)) {
						v0 = dVTX[2];
						v1 = dVTX[1];
				}
				
				x1 = v0->v.x;
				y1 = v0->v.y;

				x2 = v1->v.x;
				y2 = v1->v.y;

				if (lhori) {
					x1 -= (LINEWIDTH*0.5f);
					x2 -= (LINEWIDTH*0.5f);
					y1 -= (LINEWIDTH*0.5f);
					y2 -= (LINEWIDTH*0.5f);
					
					vert->x = x1;
					vert->y = y1 + LINEWIDTH;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x1;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;

					vert->x = x2;
					vert->y = y2 + LINEWIDTH;
					vert->z = v1->v.z;
					vert++;

					vert->x = x2;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;
				} else if (lvert) {
					x1 -= (LINEWIDTH*0.5f);
					x2 -= (LINEWIDTH*0.5f);
					y1 -= (LINEWIDTH*0.5f);
					y2 -= (LINEWIDTH*0.5f);

					vert->x = x2;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;

					vert->x = x1;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x2 + LINEWIDTH;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;
					
					vert->x = x1 + LINEWIDTH;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;
				} else {
					float dx = x2 - x1;
					float dy = y2 - y1;

					float hlw_invmag = frsqrt((dx*dx) + (dy*dy)) * (LINEWIDTH*0.5f); float nx = -dy * hlw_invmag; float ny = dx * hlw_invmag;

					vert->x = x1 + nx;
					vert->y = y1 + ny;
					vert->z = v0->v.z;
					vert++;

					vert->x = x1 - nx;
					vert->y = y1 - ny;
					vert->z = v1->v.z;
					vert++;
					
					vert->x = x2 + nx;
					vert->y = y2 + ny;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x2 - nx;
					vert->y = y2 - ny;
					vert->z = v1->v.z;
					vert++;
				}
			}
			lvert = 0;
			lhori = 0;
			{
				int x1,y1,x2,y2;
				
				if ((int)(dVTX[2]->v.x) == (int)(dVTX[0]->v.x)) {
					lvert = 1;
					if((int)(dVTX[2]->v.y) > (int)(dVTX[0]->v.y)) {
						v0 = dVTX[0];
						v1 = dVTX[2];
					} else {
						v0 = dVTX[2];
						v1 = dVTX[0];
					}
				} else if ((int)(dVTX[2]->v.y) == (int)(dVTX[0]->v.y)) {
					lhori = 1;
					if((int)(dVTX[2]->v.x) < (int)(dVTX[0]->v.x)) {					
						v0 = dVTX[2];
						v1 = dVTX[0];
					} else {
						v0 = dVTX[0];
						v1 = dVTX[2];
					}
				} else if((int)(dVTX[2]->v.x) < (int)(dVTX[0]->v.x)) {
						v0 = dVTX[2];
						v1 = dVTX[0];
				} else if((int)(dVTX[2]->v.x) > (int)(dVTX[0]->v.x)) {
						v0 = dVTX[0];
						v1 = dVTX[2];
				}
				
				x1 = v0->v.x;
				y1 = v0->v.y;

				x2 = v1->v.x;
				y2 = v1->v.y;
				
				pvr_list_prim(PVR_LIST_OP_POLY, &hdr, sizeof(hdr));
				if (lhori) {
					x1 -= (LINEWIDTH*0.5f);
					x2 -= (LINEWIDTH*0.5f);
					y1 -= (LINEWIDTH*0.5f);
					y2 -= (LINEWIDTH*0.5f);
					
					vert->x = x1;
					vert->y = y1 + LINEWIDTH;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x1;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;

					vert->x = x2;
					vert->y = y2 + LINEWIDTH;
					vert->z = v1->v.z;
					vert++;

					vert->x = x2;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;
				} else if (lvert) {
					x1 -= (LINEWIDTH*0.5f);
					x2 -= (LINEWIDTH*0.5f);
					y1 -= (LINEWIDTH*0.5f);
					y2 -= (LINEWIDTH*0.5f);

					vert->x = x2;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;

					vert->x = x1;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x2 + LINEWIDTH;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;
					
					vert->x = x1 + LINEWIDTH;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;
				} else {
					float dx = x2 - x1;
					float dy = y2 - y1;

					float hlw_invmag = frsqrt((dx*dx) + (dy*dy)) * (LINEWIDTH*0.5f);
					float nx = -dy * hlw_invmag;
					float ny = dx * hlw_invmag;

					vert->x = x1 + nx;
					vert->y = y1 + ny;
					vert->z = v0->v.z;
					vert++;

					vert->x = x1 - nx;
					vert->y = y1 - ny;
					vert->z = v1->v.z;
					vert++;
					
					vert->x = x2 + nx;
					vert->y = y2 + ny;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x2 - nx;
					vert->y = y2 - ny;
					vert->z = v1->v.z;
					vert++;
				}
	}
	pvr_list_prim(PVR_LIST_OP_POLY, &verts, sizeof(pvr_vertex_t)*12);
}

void AM_DrawLine(player_t *player, fixed_t bbox[static 4])
{
	line_t *l;
	int i, color;
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_t cxt;
	pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
	pvr_poly_compile(&hdr, &cxt);

	dVTX[0] = &(dT1.dVerts[0]);
	dVTX[1] = &(dT1.dVerts[1]);
	dVTX[2] = &(dT1.dVerts[2]);
	dVTX[3] = &(dT2.dVerts[2]);

	l = lines;
	for (i = 0; i < numlines; i++, l++) {
		if(l->flags & ML_DONTDRAW)
			continue;

		if (!M_BoxIntersect(bbox, l->bbox))
			continue;

		if(((l->flags & ML_MAPPED) || player->powers[pw_allmap]) || (player->cheats & CF_ALLMAP)) {
			I_CheckGFX();

			/* */
			/* Figure out color */
			/* */
			color = COLOR_BROWN;

			if((player->powers[pw_allmap] || (player->cheats & CF_ALLMAP)) && !(l->flags & ML_MAPPED))
				color = COLOR_GREY;
			else if (l->flags & ML_SECRET)
				color = COLOR_RED;
			else if(l->special && !(l->flags & ML_HIDEAUTOMAPTRIGGER))
				color = COLOR_YELLOW;
			else if (!(l->flags & ML_TWOSIDED)) /* ONE-SIDED LINE */
				color = COLOR_RED;
				
			float x1 = (float)(l->v1->x >> 16);
			float x2 = (float)(l->v2->x >> 16);

			float z1 = -((float)(l->v1->y >> 16));
			float z2 = -((float)(l->v2->y >> 16));

			dVTX[0]->v.x = x1;
			dVTX[2]->v.x = x2;
			dVTX[0]->v.y = dVTX[2]->v.y = 0.0f;
			dVTX[0]->v.z = z1;
			dVTX[2]->v.z = z2;

			transform_vert(dVTX[0]);
			transform_vert(dVTX[2]);
			perspdiv(dVTX[0]);
			perspdiv(dVTX[2]);

			pvr_vertex_t __attribute__((aligned(32))) verts[4];
			for (int vn = 0; vn < 4; vn++) {
				verts[vn].flags = PVR_CMD_VERTEX;
				verts[vn].argb = D64_PVR_REPACK_COLOR(color);
				verts[vn].oargb = 0;
			}
			verts[3].flags = PVR_CMD_VERTEX_EOL;

			pvr_vertex_t *vert = verts;
			{
				d64Vertex_t *v0,*v1;
				int lvert = 0;
				int lhori = 0;
				
				int x1,y1,x2,y2;
				
				if ((int)(dVTX[0]->v.x) == (int)(dVTX[2]->v.x)) {
					lvert = 1;
					if((int)(dVTX[0]->v.y) > (int)(dVTX[2]->v.y)) {
						v0 = dVTX[2];
						v1 = dVTX[0];
					} else {
						v0 = dVTX[0];
						v1 = dVTX[2];
					}
				} else if ((int)(dVTX[0]->v.y) == (int)(dVTX[2]->v.y)) {
					lhori = 1;
					if((int)(dVTX[0]->v.x) < (int)(dVTX[2]->v.x)) {					
						v0 = dVTX[0];
						v1 = dVTX[2];
					} else {
						v0 = dVTX[2];
						v1 = dVTX[0];
					}
				} else if((int)(dVTX[0]->v.x) < (int)(dVTX[2]->v.x)) {
						v0 = dVTX[0];
						v1 = dVTX[2];
				} else if((int)(dVTX[0]->v.x) > (int)(dVTX[2]->v.x)) {
						v0 = dVTX[2];
						v1 = dVTX[0];
				}
				
				x1 = v0->v.x;
				y1 = v0->v.y;

				x2 = v1->v.x;
				y2 = v1->v.y;
				
				pvr_list_prim(PVR_LIST_OP_POLY, &hdr, sizeof(hdr));
				if (lhori) {
					x1 -= (LINEWIDTH*0.5f);
					x2 -= (LINEWIDTH*0.5f);
					y1 -= (LINEWIDTH*0.5f);
					y2 -= (LINEWIDTH*0.5f);
					
					vert->x = x1;
					vert->y = y1 + LINEWIDTH;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x1;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;

					vert->x = x2;
					vert->y = y2 + LINEWIDTH;
					vert->z = v1->v.z;
					vert++;

					vert->x = x2;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;
				} else if (lvert) {
					x1 -= (LINEWIDTH*0.5f);
					x2 -= (LINEWIDTH*0.5f);
					y1 -= (LINEWIDTH*0.5f);
					y2 -= (LINEWIDTH*0.5f);

					vert->x = x2;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;

					vert->x = x1;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x2 + LINEWIDTH;
					vert->y = y2;
					vert->z = v1->v.z;
					vert++;
					
					vert->x = x1 + LINEWIDTH;
					vert->y = y1;
					vert->z = v0->v.z;
					vert++;
				} else {
					float dx = x2 - x1;
					float dy = y2 - y1;

					float hlw_invmag = frsqrt((dx*dx) + (dy*dy)) * (LINEWIDTH*0.5f); 
					float nx = -dy * hlw_invmag; 
					float ny = dx * hlw_invmag;

					vert->x = x1 + nx;
					vert->y = y1 + ny;
					vert->z = v0->v.z;
					vert++;

					vert->x = x1 - nx;
					vert->y = y1 - ny;
					vert->z = v1->v.z;
					vert++;
					
					vert->x = x2 + nx;
					vert->y = y2 + ny;
					vert->z = v0->v.z;
					vert++;
					
					vert->x = x2 - nx;
					vert->y = y2 - ny;
					vert->z = v1->v.z;
					vert++;
				}
				pvr_list_prim(PVR_LIST_OP_POLY, &verts, sizeof(pvr_vertex_t)*4);
			}
		}
	}
}

/*
==================
=
= AM_DrawThings
=
==================
*/

void AM_DrawThings(fixed_t x, fixed_t y, angle_t angle, int color) // 80001834
{
	angle_t ang = angle >> ANGLETOFINESHIFT;
	pvr_vertex_t *vert = thing_verts;

	for (int i=0;i<3;i++) {
		thing_verts[i].argb = D64_PVR_REPACK_COLOR(color);
	}

	dVTX[0] = &(dT1.dVerts[0]);
	dVTX[1] = &(dT1.dVerts[1]);
	dVTX[2] = &(dT1.dVerts[2]);

	dVTX[0]->v.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	dVTX[0]->v.y = 0.0f;
	dVTX[0]->v.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);

	ang = (angle + 0xA0000000) >> ANGLETOFINESHIFT;

	dVTX[1]->v.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	dVTX[1]->v.y = 0.0f;
	dVTX[1]->v.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);	

	ang = (angle + 0x60000000) >> ANGLETOFINESHIFT;

	dVTX[2]->v.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	dVTX[2]->v.y = 0.0f;
	dVTX[2]->v.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);	

	transform_vert(dVTX[0]);
	transform_vert(dVTX[1]);
	transform_vert(dVTX[2]);
	perspdiv(dVTX[0]);
	perspdiv(dVTX[1]);
	perspdiv(dVTX[2]);

	pvr_list_prim(PVR_LIST_TR_POLY, &thing_hdr, sizeof(thing_hdr));

	vert->x = dVTX[0]->v.x;
	vert->y = dVTX[0]->v.y;
	vert->z = dVTX[0]->v.z + 0.5;
	vert++;

	vert->x = dVTX[1]->v.x;
	vert->y = dVTX[1]->v.y;
	vert->z = dVTX[1]->v.z + 0.5;
	vert++;
					
	vert->x = dVTX[2]->v.x;
	vert->y = dVTX[2]->v.y;
	vert->z = dVTX[2]->v.z + 0.5;

	pvr_list_prim(PVR_LIST_TR_POLY, &thing_verts, sizeof(pvr_vertex_t)*3);
}