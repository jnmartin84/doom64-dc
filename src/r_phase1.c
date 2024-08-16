//Renderer phase 1 - BSP traversal

#include "doomdef.h"
#include "r_local.h"

extern short SwapShort(short dat);

int checkcoord[12][4] =
{
	{ 3, 0, 2, 1 }, /* Above,Left */
	{ 3, 0, 2, 0 }, /* Above,Center */
	{ 3, 1, 2, 0 }, /* Above,Right */
	{ 0, 0, 0, 0 },
	{ 2, 0, 2, 1 }, /* Center,Left */
	{ 0, 0, 0, 0 }, /* Center,Center */
	{ 3, 1, 3, 0 }, /* Center,Right */
	{ 0, 0, 0, 0 },
	{ 2, 0, 3, 1 }, /* Below,Left */
	{ 2, 1, 3, 1 }, /* Below,Center */
	{ 2, 1, 3, 0 }, /* Below,Right */
	{ 0, 0, 0, 0 }
};

void	R_RenderBSPNode(int bspnum);
boolean	R_CheckBBox(fixed_t bspcoord[4]);
void	R_Subsector(int num);
void	R_AddLine(seg_t *line);
void	R_AddSprite(subsector_t *sub);
void	R_RenderBSPNodeNoClip(int bspnum);


projectile_light_t projectile_lights[NUM_DYNLIGHT];
int lightidx = -1;

static void R_ResetProjectileLights(void) {
	lightidx = -1;
}

static void R_AddProjectileLight(fixed_t x, fixed_t y, fixed_t z, float rad, uint32_t lightc) {
	if (lightidx < (NUM_DYNLIGHT-1))
		lightidx++;
	else return;

	projectile_lights[lightidx].x = (float)(x >> 16);
	projectile_lights[lightidx].y = (float)(y >> 16);
	projectile_lights[lightidx].z = (float)(z >> 16);

	projectile_lights[lightidx].r = (float)((lightc >> 16)&255) / 255.0f;
	projectile_lights[lightidx].g = (float)((lightc >> 8)&255) / 255.0f;
	projectile_lights[lightidx].b = (float)(lightc&255) / 255.0f;

	projectile_lights[lightidx].radius = rad;
}

extern int player_shooting;
extern int player_light;
extern int player_last_weapon;

int player_light_fade = -1;
// Kick off the rendering process by initializing the solidsubsectors array and then
// starting the BSP traversal.
//
void R_BSP(void)
{
	int count;
	subsector_t **sub;


	validcount++;

	rendersky = false;

	numdrawsubsectors = 0;
	numdrawvissprites = 0;
	R_ResetProjectileLights();

	player_t *p;
	p = &players[0];
	
	// convoluted logic for making a light appear when a player shoots and then 
	// making it fade out over slightly different times for different weapons
	if (player_light) {
		if (player_shooting) {
			R_AddProjectileLight(p->mo->x, p->mo->y, players[0].viewz, 384 ,0xff7f7f7f);
			player_shooting = 0;
			goto skip_player_light;
		} else if (!player_shooting && player_light_fade == -1) {
			if (player_last_weapon == wp_pistol) {
				player_light_fade = 2;
			} else if (player_last_weapon == wp_shotgun) {
				player_light_fade = 4;
			} else if (player_last_weapon == wp_supershotgun) {
				player_light_fade = 6;
			} else if (player_last_weapon == wp_chaingun) {
				player_light_fade = 4;
			}
		}
		
		if (!player_shooting && player_light_fade != -1) {
			int scale_start = 0;
			if (player_last_weapon == wp_pistol) {
				scale_start = 3;
			} else if (player_last_weapon == wp_shotgun) {
				scale_start = 5;
			} else if (player_last_weapon == wp_supershotgun) {
				scale_start = 7;
			} else if (player_last_weapon == wp_chaingun) {
				scale_start = 5;
			}

			int8_t c = 0x7f - ((scale_start - player_light_fade - 1)*2);

			if (player_light_fade == 0) {
				player_light = 0;
				player_light_fade = -1;
				goto skip_player_light;
			} else {
				player_light_fade -= 1;
			}

			uint32_t color = 0xff00000 | ((c&0xff) << 16) | ((c&0xff) << 8) | (c&0xff);
			R_AddProjectileLight(p->mo->x, p->mo->y, players[0].viewz, 384 - ((scale_start - player_light_fade)*32), color);
		}
	}

skip_player_light:

	visspritehead = vissprites;

	endsubsector = solidsubsectors; /* Init the free memory pointer */
	D_memset(solidcols, 0, 320);

	if (camviewpitch == 0) {
		R_RenderBSPNode(numnodes - 1); /* Begin traversing the BSP tree for all walls in render range */
	} else {
		R_RenderBSPNodeNoClip(numnodes - 1); /* Begin traversing the BSP tree for all walls in render range */
		rendersky = true;
	}

	sub = solidsubsectors;
	count = numdrawsubsectors;
	while (count) {
		R_AddSprite(*sub);	// Render each sprite
		sub++;				// Inc the sprite pointer
		count--;
	}
}

//
// Recursively descend through the BSP, classifying nodes according to the
// player's point of view, and render subsectors in view.
//
static boolean R_RenderBspSubsector(int bspnum)
{
	if (bspnum & NF_SUBSECTOR) {
		if (bspnum == -1)
			R_Subsector(0);
		else
			R_Subsector(bspnum & (~NF_SUBSECTOR));

		return true;
	}
	return false;	
}

// BSP stack algorithm borrowed from GBADoom 
// https://github.com/doomhack/GBADoom/blob/3bd2cf34fb66dd60f9b66efe25cc1d6945a21f74/source/r_hotpath.iwram.c#L2791
#define MAX_BSP_DEPTH 128

// RenderBSPNode
// Renders all subsectors below a given node,
// traversing subtree recursively.
// Just call with BSP root.
//
// Non recursive version.
// constant stack space used and easier to
// performance profile.
static int bspstack[MAX_BSP_DEPTH];

void R_RenderBSPNode(int bspnum)
{
	node_t *bsp;
	int side;
	fixed_t dx, dy;
	fixed_t left, right;

	int sp = 0;

	while (true) {
		while (!R_RenderBspSubsector(bspnum)) {
			if (sp == MAX_BSP_DEPTH)
				break;

			bsp = &nodes[bspnum];

			dx = (viewx - bsp->line.x);
			dy = (viewy - bsp->line.y);

			left = (bsp->line.dy >> 16) * (dx >> 16);
			right = (dy >> 16) * (bsp->line.dx >> 16);

			if (right < left)
				side = 0; // front side
			else
				side = 1; // back side

			bspstack[sp++] = bspnum;
			bspstack[sp++] = side;

			bspnum = bsp->children[side];
		}

		if (sp == 0) {
			// back at root node and not visible. All done!
			return;
		}

		// Back sides.
		side = bspstack[--sp];
		bspnum = bspstack[--sp];
		bsp = &nodes[bspnum];

		// Possibly divide back space.
		// Walk back up the tree until we find
		// a node that has a visible backspace.
		while (!R_CheckBBox (bsp->bbox[side^1])) {
			if (sp == 0) {
				// back at root node and not visible. All done!
				return;
			}

			// Back side next.
			side = bspstack[--sp];
			bspnum = bspstack[--sp];

			bsp = &nodes[bspnum];
		}

		bspnum = bsp->children[side^1];		
	}
}

//
// Checks BSP node/subtree bounding box. Returns true if some part of the bbox
// might be visible.
//
boolean R_CheckBBox(fixed_t bspcoord[4])
{
	int boxx;
	int boxy;
	int boxpos;

	fixed_t x1, y1, x2, y2;
	byte *solid_cols;
	int vx1, vy1, vx2, vy2, delta;
	int Xstart, Xend;

	// find the corners of the box that define the edges from current viewpoint
	if (viewx < bspcoord[BOXLEFT])
		boxx = 0;
	else if (viewx <= bspcoord[BOXRIGHT])
		boxx = 1;
	else
		boxx = 2;

	if (viewy > bspcoord[BOXTOP])
		boxy = 0;
	else if (viewy >= bspcoord[BOXBOTTOM])
		boxy = 1;
	else
		boxy = 2;

	boxpos = (boxy << 2) + boxx;
	if (boxpos == 5)
		return true;

	x1 = bspcoord[checkcoord[boxpos][0]];
	y1 = bspcoord[checkcoord[boxpos][1]];
	x2 = bspcoord[checkcoord[boxpos][2]];
	y2 = bspcoord[checkcoord[boxpos][3]];

	vx1 = FixedMul(viewsin, x1 - viewx) - FixedMul(viewcos, y1 - viewy);
	vy1 = FixedMul(viewcos, x1 - viewx) + FixedMul(viewsin, y1 - viewy);
	vx2 = FixedMul(viewsin, x2 - viewx) - FixedMul(viewcos, y2 - viewy);
	vy2 = FixedMul(viewcos, x2 - viewx) + FixedMul(viewsin, y2 - viewy);

	if ((vx1 < -vy1) && (vx2 < -vy2))
		return false;

	if ((vy1 < vx1) && (vy2 < vx2))
		return false;

	if ((((vx2 >> 16) * (vy1 >> 16)) - ((vx1 >> 16) * (vy2 >> 16))) < 2)
		return true;

	if ((vy1 <= 0) && (vy2 <= 0))
		return false;

	if (vx1 < -vy1) {
		delta = (vx1 + vy1);
		delta = FixedDiv2(delta, ((delta - vx2) - vy2));
		delta = FixedMul(delta, (vy2 - vy1));

		vy1 += delta;
		vx1 = -vy1;
	}

	if (vy2 < vx2) {
		delta = (vx1 - vy1);
		delta = FixedDiv2(delta, ((delta - vx2) + vy2));
		delta = FixedMul(delta, (vy2 - vy1));
		vx2 = delta + vy1;
		vy2 = vx2;
	}

	// multiply by 320 
	// is (x*256) + (x*64)
	// (x << 8) + (x << 6)
	// by 160
	// is (x*128) + (x*32)
	// ((x << 7) + (x << 5)
//	Xstart = ((FixedDiv2(vx1, vy1) * 160) >> 16) + 160;
//	Xend   = ((FixedDiv2(vx2, vy2) * 160) >> 16) + 160;

	fixed_t vxovery1 = FixedDiv2(vx1, vy1);
	fixed_t vxovery2 = FixedDiv2(vx2, vy2);

//	Xstart = (((vxovery1 << 7) + (vxovery1 << 5)) >> 16) + 160;
//	Xend   = (((vxovery2 << 7) + (vxovery2 << 5)) >> 16) + 160;

	Xstart = (((vxovery1 >> 9) + (vxovery1 >> 11))) + 160;
	Xend   = (((vxovery2 >> 9) + (vxovery2 >> 11))) + 160;


	if (Xstart < 0)
		Xstart = 0;

	if (Xend >= 320)
		Xend = 320;

	solid_cols = &solidcols[Xstart];
	while (Xstart < Xend) {
		if (*solid_cols == 0)
			return true;
		solid_cols++;
		Xstart++;
	}

	return false;
}

//
// Determine floor/ceiling planes, add sprites of things in sector,
// draw one or more segments.
//

void R_Subsector(int num) // 8002451C
{
	subsector_t *sub;
	seg_t       *line;
	int          count;

#ifdef RANGECHECK
	if (num >= numsubsectors) {
		I_Error("R_Subsector: ss %i with numss = %i", num, numsubsectors);
	}
#endif

	if (numdrawsubsectors < MAXSUBSECTORS) {
		numdrawsubsectors++;

		sub = &subsectors[num];
		sub->drawindex = numdrawsubsectors;

		*endsubsector = sub; // copy subsector
		endsubsector++;

		frontsector = sub->sector;

		line = &segs[sub->firstline];
		count = sub->numlines;

		do {
			R_AddLine(line);	/* Render each line */
			++line;				/* Inc the line pointer */
		} while (--count);		/* All done? */
	}
}

//
// Clips the given segment and adds any visible pieces to the line list.
//
void R_AddLine(seg_t *line)
{
	sector_t *backsector;
	vertex_t *vrt, *vrt2;
	int x1, y1, x2, y2, count;
	int Xstart, Xend, delta;
	byte *solid_cols;

	line->flags &= ~1;

	vrt = line->v1;
	if (vrt->validcount != validcount) {
		x1 = FixedMul(viewsin, (vrt->x - viewx)) - FixedMul(viewcos,(vrt->y - viewy));
		y1 = FixedMul(viewcos, (vrt->x - viewx)) + FixedMul(viewsin,(vrt->y - viewy));

		vrt->vx = x1;
		vrt->vy = y1;

		vrt->validcount = validcount;
	} else {
		x1 = vrt->vx;
		y1 = vrt->vy;
	}

	vrt2 = line->v2;
	if (vrt2->validcount != validcount) {
		x2 = FixedMul(viewsin, (vrt2->x - viewx)) - FixedMul(viewcos,(vrt2->y - viewy));
		y2 = FixedMul(viewcos, (vrt2->x - viewx)) + FixedMul(viewsin,(vrt2->y - viewy));

		vrt2->vx = x2;
		vrt2->vy = y2;

		vrt2->validcount = validcount;
	} else {
		x2 = vrt2->vx;
		y2 = vrt2->vy;
	}

	if ((x1 < -y1) && (x2 < -y2))
		return;

	if ((y1 < x1) && (y2 < x2))
		return;

	if ((y1 < ((8*FRACUNIT)+1)) && (y2 < ((8*FRACUNIT)+1)))
		return;

	if ((((x2 >> 16) * (y1 >> 16)) - ((x1 >> 16) * (y2 >> 16))) <= 0)
		return;

	if (y1 < (8*FRACUNIT))
	{
		delta = FixedDiv2(((8*FRACUNIT) - y1), (y2 - y1));
		delta = FixedMul(delta, (x2 - x1));
		x1 += delta;
		y1 = (8*FRACUNIT);
	}
	else if (y2 < (8*FRACUNIT))
	{
		delta = FixedDiv2(((8*FRACUNIT) - y2), (y1 - y2));
		delta = FixedMul(delta, (x1 - x2));
		x2 += delta;
		y2 = (8*FRACUNIT);
	}

	// multiply by 320 
	// is (x*256) + (x*64)
	// (x << 8) + (x << 6)
	// by 160
	// is (x*128) + (x*32)
	// ((x << 7) + (x << 5)
//	Xstart = ((FixedDiv2(x1, y1) * 160) >> 16) + 160;
//	Xend   = ((FixedDiv2(x2, y2) * 160) >> 16) + 160;

	fixed_t xovery1 = FixedDiv2(x1, y1);
	fixed_t xovery2 = FixedDiv2(x2, y2);

//	Xstart = (((xovery1 << 7) + (xovery1 << 5)) >> 16) + 160;
//	Xend   = (((xovery2 << 7) + (xovery2 << 5)) >> 16) + 160;

	Xstart = (((xovery1 >> 9) + (xovery1 >> 11))) + 160;
	Xend   = (((xovery2 >> 9) + (xovery2 >> 11))) + 160;
	
	if (Xstart < 0)
		Xstart = 0;

	if (Xend >= 320)
		Xend = 320;

	if (Xstart != Xend) {
		solid_cols = &solidcols[Xstart];
		count = Xstart;
		while (count < Xend) {
			if (*solid_cols == 0) {
				line->flags |= 1;
				line->linedef->flags |= ML_MAPPED;
				break;
			}
			solid_cols++;
			count++;
		}

		if (frontsector->ceilingpic == -1) {
			rendersky = true;
		}

		if (!(line->linedef->flags & (ML_DONTOCCLUDE|ML_DRAWMASKED))) {
			backsector = line->backsector;

			if(!backsector ||
				backsector->ceilingheight <= frontsector->floorheight ||
				backsector->floorheight   >= frontsector->ceilingheight ||
				backsector->floorheight   == backsector->ceilingheight) { // New line on Doom 64
				solid_cols = &solidcols[Xstart];
				while (Xstart < Xend) {
					*solid_cols = 1;
					solid_cols++;
					Xstart += 1;
				}
			}
		}
	}
}

void R_AddSprite(subsector_t *sub) // 80024A98
{
	byte *data;
	mobj_t *thing;
	spritedef_t		*sprdef;
	spriteframe_t	*sprframe;

	subsector_t     *pSub;
	subsector_t     *CurSub;
	vissprite_t     *VisSrpCur, *VisSrpCurTmp;
	vissprite_t     *VisSrpNew;

	angle_t         ang;
	unsigned int    rot;
	boolean         flip;
	int             lump;
	fixed_t         tx, tz;
	fixed_t         x, y;

	sub->vissprite = NULL;

	for (thing = sub->sector->thinglist; thing; thing = thing->snext)
	{
		if (thing->subsector != sub)
			continue;

		if (numdrawvissprites >= MAXVISSPRITES)
			break;

		if (thing->flags & MF_RENDERLASER)
		{
			visspritehead->zdistance = MAXINT;
			visspritehead->thing = thing;
			visspritehead->next = sub->vissprite;
			sub->vissprite = visspritehead;

			R_AddProjectileLight(thing->x, thing->y, thing->z, 304, 0x00ff0000);

			visspritehead++;
			numdrawvissprites++;
		}
		else
		{
			// transform origin relative to viewpoint
			x = (thing->x - viewx) >> 16;
			y = (thing->y - viewy) >> 16;
			tx = ((viewsin * x) - (viewcos * y)) >> 16;
			tz = ((viewcos * x) + (viewsin * y)) >> 16;

			// thing is behind view plane?
			if (tz < MINZ)
				continue;

			// too far off the side?
			if (tx > (tz << 1) || tx < -(tz << 1))
				continue;

			sprdef = &sprites[thing->sprite];
			sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

			if (sprframe->rotate != 0)
			{
				ang = R_PointToAngle2(viewx, viewy, thing->x, thing->y);
				rot = ((ang - thing->angle) + ((unsigned int)(ANG45 / 2) * 9)) >> 29;
				lump = sprframe->lump[rot];
				flip = (boolean)(sprframe->flip[rot]);
			}
			else
			{
				lump = sprframe->lump[0];
				flip = (boolean)(sprframe->flip[0]);
			}

			visspritehead->zdistance = tz;
			visspritehead->thing = thing;
			visspritehead->lump = lump;
			visspritehead->flip = flip;
			visspritehead->next = NULL;
			visspritehead->sector = sub->sector;

			data = (byte *)W_CacheLumpNum(lump, PU_CACHE, dec_jag);

			CurSub = sub;
			if (tz < MAXZ)
			{
				if (thing->flags & (MF_CORPSE|MF_SHOOTABLE))
				{
					x = (( SwapShort(((spriteN64_t*)data)->width) >> 1) * viewsin);
					y = (( SwapShort(((spriteN64_t*)data)->width) >> 1) * viewcos);

					pSub = R_PointInSubsector((thing->x - x), (thing->y + y));
					if ((pSub->drawindex) && (pSub->drawindex < sub->drawindex)) {
						CurSub = pSub;
					}

					pSub = R_PointInSubsector((thing->x + x), (thing->y - y));
					if ((pSub->drawindex) && (pSub->drawindex < CurSub->drawindex)) {
						CurSub = pSub;
					}
				}
			}

			VisSrpCur = CurSub->vissprite;
			VisSrpNew = NULL;

			if (VisSrpCur)
			{
				VisSrpCurTmp = VisSrpCur;
				while ((VisSrpCur = VisSrpCurTmp, tz < VisSrpCur->zdistance))
				{
					VisSrpCur = VisSrpCurTmp->next;
					VisSrpNew = VisSrpCurTmp;

					if (VisSrpCur == NULL)
						break;

					VisSrpCurTmp = VisSrpCur;
				}
			}

			if (VisSrpNew)
				VisSrpNew->next = visspritehead;
			else
				CurSub->vissprite = visspritehead;

			int random_factor = I_Random()%24;

			// red keycard / skull key
			if(lump == 188 || lump == 208) {
				int r = 255 - random_factor;

				uint32_t color = (r << 16);
				
				R_AddProjectileLight(thing->x, thing->y, thing->z, 112, color);
			}

			// yellow keycard / skull key
			if(lump == 189 || lump == 210) {
				int r = 255 - random_factor;
				int g = 255 - random_factor;

				uint32_t color = (r << 16) | (g << 8);
				
				R_AddProjectileLight(thing->x, thing->y, thing->z, 112, color);
			}

			// blue keycard / skull key
			if(lump == 190 || lump == 209) {
				int b = 255 - random_factor;

				uint32_t color = b;
				
				R_AddProjectileLight(thing->x, thing->y, thing->z, 112, color);
			}

#if 0
			// rad suit
			if(lump >= 73 && lump <= 74) {
				int g = 127;

				if (lump == 74) {
					g = 96;
				}
				uint32_t color = (g << 8);
				
				R_AddProjectileLight(thing->x, thing->y, thing->z, 32, color);
			}

			// green armor
			if(lump >= 179 && lump <= 180) {
				int g = 127;

				if (lump == 180) {
					g = 96;
				}
				uint32_t color = (g << 8);
				
				R_AddProjectileLight(thing->x, thing->y, thing->z, 32, color);
			}

			// blue armor
			if(lump >= 181 && lump <= 182) {
				int b = 127;

				if (lump == 181) {
					b = 96;
				}
				uint32_t color = b;
				
				R_AddProjectileLight(thing->x, thing->y, thing->z, 32, color);
			}
#endif

			// rockets and barrels
			if(lump >= 211 && lump <= 220) {
				// 255 127 0
				float radius = 304;
				int r = 255 - random_factor;
				int g = 127 - random_factor;

				if (lump > 215) {
					r /= ((lump - 216)/2) + 1;
					g /= ((lump - 216)/2) + 1;
					radius += 24;
				}
				uint32_t color = (r << 16) | (g << 8);
				
				// 216 to 220 are when it hits and disappears
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// normal imp
			if(lump >= 238 && lump <= 246) {
				// 255 127 0
				float radius = 256;
				
				int r = 255 - random_factor;
				int g = 127 - random_factor;

				if (lump > 240) {
					r /= ((lump - 241)/2) + 1;
					g /= ((lump - 241)/2) + 1;
					radius += 24;
				}
				uint32_t color = (r << 16) | (g << 8);
				
				// 241 to 246 are when it hits and disappears
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// nightmare imp
			if(lump >= 247 && lump <= 255) {
				float radius = 256;
				int r = 0x8a - random_factor;
				int g = 0x2b - random_factor;
				int b = 0xe2 - random_factor;

				if (lump > 249) {
					r /= ((lump - 250)/2) + 1;
					g /= ((lump - 250)/2) + 1;
					b /= ((lump - 250)/2) + 1;
					radius += 24;
				}
				uint32_t color = (r << 16) | (g << 8) | b;
				
				// 250 to 255 are when it hits and disappears
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// hell knight
			if(lump >= 256 && lump <= 269) {
				float radius = 256;
				int g = 255 - random_factor;
				// 264
				if (lump > 263) {
					g /= ((lump - 264)/2) + 1;
					radius += 24;
				}
				uint32_t color = (g << 8);
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// baron of hell
			if(lump >= 270 && lump <= 283) {
				float radius = 256;
				int r = 255 - random_factor;
				// 278
				if (lump > 277) {
					r /= ((lump - 278)/2) + 1;
					radius += 24;
				}
				uint32_t color = (r << 16);
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// mancubus???
			if(lump >= 284 && lump <= 304) {
				float radius = 256;

				int r = 255 - random_factor;
				int g = 127 - random_factor;

				if (lump > 298) {
					r /= ((lump - 299)/2) + 1;
					g /= ((lump - 299)/2) + 1;
					radius += 24;
				}
				uint32_t color = (r << 16) | (g << 8);
				
				// 299 to 304 are when it hits and disappears
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// cacodemon
			if(lump >= 305 && lump <= 314) {
				// 255 63 0
					float radius = 256;

				int r = 255 - random_factor;
				int g = 63 - random_factor;

				if (lump > 307) {
					r /= ((lump - 308)/2) + 1;
					g /= ((lump - 308)/2) + 1;
					radius += 24;
				}
				uint32_t color = (r << 16) | (g << 8);
				
				// 308 to 314 are when it hits and disappears
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// bfg
			if(lump >= 315 && lump <= 322) {
				float radius = 304;
				int g = 255 - random_factor;
				// 317
				if (lump > 316) {
					g /= ((lump - 317)/2) + 1;
					radius += 24;
				}
				uint32_t color = (g << 8);
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// plasma
			if(lump >= 323 && lump <= 330) {
				float radius = 304;
				int b = 255 - random_factor;
				// 325
				if (lump > 324) {
					b /= ((lump - 325)/2) + 1;
					radius += 24;
				}
				uint32_t color = b;
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// spider shot
			if(lump >= 331 && lump <= 338) {
				float radius = 224;
				int r = 0x8a - random_factor;
				int g = 0xa3 - random_factor;
				int b = 0xfa - random_factor;
				// 333
				if (lump > 332) {
					r /= ((lump - 333)/2) + 1;
					g /= ((lump - 333)/2) + 1;
					b /= ((lump - 333)/2) + 1;
					radius += 24;
				}
				uint32_t color = (r << 16) | (g << 8) | b;
				
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			// skul
			if(lump >= 619 && lump <= 658) {
				// 255 127 0
				float radius = 224;
				int r = 128 - random_factor;
				int g = 63 - random_factor;

				if (lump > 649) {
					r /= ((lump - 650)/2) + 1;
					g /= ((lump - 650)/2) + 1;
					radius += 24;
				}
				uint32_t color = (r << 16) | (g << 8);
				
				R_AddProjectileLight(thing->x, thing->y, thing->z, radius, color);
			}

			visspritehead->next = VisSrpCur;

			numdrawvissprites++;
			visspritehead++;
		}
	}
}

void R_RenderBSPNodeNoClip(int bspnum) // 80024E64
{
	subsector_t *sub;
	seg_t       *line;
	int          count;
	node_t      *bsp;
	int          side;
	fixed_t	     dx, dy;
	fixed_t	     left, right;

	while(!(bspnum & NF_SUBSECTOR))
	{
		bsp = &nodes[bspnum];

		// Decide which side the view point is on.
		//side = R_PointOnSide(viewx, viewy, bsp);
		dx = (viewx - bsp->line.x);
		dy = (viewy - bsp->line.y);

		left = (bsp->line.dy >> 16) * (dx >> 16);
		right = (dy >> 16) * (bsp->line.dx >> 16);

		if (right < left)
			side = 1;		/* back side */
		else
			side = 0;		/* front side */

		R_RenderBSPNodeNoClip(bsp->children[side ^ 1]);

		bspnum = bsp->children[side];
	}

	// subsector with contents
	// add all the drawable elements in the subsector

	numdrawsubsectors++;

	sub = &subsectors[bspnum & ~NF_SUBSECTOR];
	sub->drawindex = numdrawsubsectors;

	*endsubsector = sub;//copy subsector
	endsubsector++;

	frontsector = sub->sector;

	line = &segs[sub->firstline];
	count = sub->numlines;
	do
	{
		line->flags |= 1;	/* Render each line */
		++line;				/* Inc the line pointer */
	} while (--count);		/* All done? */
}
