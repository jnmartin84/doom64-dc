/* st_main.c -- status bar */

#include "doomdef.h"
#include "st_main.h"
#include "r_local.h"

#include "st_faces.h"

sbflash_t flashCards[6]; // 800A8180
bool tryopen[6]; // 800A81E0

uint8_t *sfontlump; // 800A81F8
uint8_t *statuslump; // 800A81FC
int sumbolslump; // 800A8204

int err_text_x = 20; // 800A8208
int err_text_y = 20; // 800A820C

symboldata_t symboldata[] = // 8005B260
	{
		{ 120, 14, 13, 13 }, // 0
		{ 134, 14, 9, 13 }, // 1
		{ 144, 14, 14, 13 }, // 2
		{ 159, 14, 14, 13 }, // 3
		{ 174, 14, 16, 13 }, // 4
		{ 191, 14, 13, 13 }, // 5
		{ 205, 14, 13, 13 }, // 6
		{ 219, 14, 14, 13 }, // 7
		{ 234, 14, 14, 13 }, // 8
		{ 0, 29, 13, 13 }, // 9
		{ 67, 28, 14, 13 }, // -
		{ 36, 28, 15, 14 }, // %
		{ 28, 28, 7, 14 }, // !
		{ 14, 29, 6, 13 }, // .
		{ 52, 28, 13, 13 }, // ?
		{ 21, 29, 6, 13 }, // :
		{ 0, 0, 13, 13 }, // A
		{ 14, 0, 13, 13 }, // B
		{ 28, 0, 13, 13 }, // C
		{ 42, 0, 14, 13 }, // D
		{ 57, 0, 14, 13 }, // E
		{ 72, 0, 14, 13 }, // F
		{ 87, 0, 15, 13 }, // G
		{ 103, 0, 15, 13 }, // H
		{ 119, 0, 6, 13 }, // I
		{ 126, 0, 13, 13 }, // J
		{ 140, 0, 14, 13 }, // K
		{ 155, 0, 11, 13 }, // L
		{ 167, 0, 15, 13 }, // M
		{ 183, 0, 16, 13 }, // N
		{ 200, 0, 15, 13 }, // O
		{ 216, 0, 13, 13 }, // P
		{ 230, 0, 15, 13 }, // Q
		{ 246, 0, 13, 13 }, // R
		{ 0, 14, 14, 13 }, // S
		{ 15, 14, 14, 13 }, // T
		{ 30, 14, 13, 13 }, // U
		{ 44, 14, 15, 13 }, // V
		{ 60, 14, 15, 13 }, // W
		{ 76, 14, 15, 13 }, // X
		{ 92, 14, 13, 13 }, // Y
		{ 106, 14, 13, 13 }, // Z
		{ 83, 31, 10, 11 }, // a
		{ 93, 31, 10, 11 }, // b
		{ 103, 31, 11, 11 }, // c
		{ 114, 31, 11, 11 }, // d
		{ 125, 31, 11, 11 }, // e
		{ 136, 31, 11, 11 }, // f
		{ 147, 31, 12, 11 }, // g
		{ 159, 31, 12, 11 }, // h
		{ 171, 31, 4, 11 }, // i
		{ 175, 31, 10, 11 }, // j
		{ 185, 31, 11, 11 }, // k
		{ 196, 31, 9, 11 }, // l
		{ 205, 31, 12, 11 }, // m
		{ 217, 31, 13, 11 }, // n
		{ 230, 31, 12, 11 }, // o
		{ 242, 31, 11, 11 }, // p
		{ 0, 43, 12, 11 }, // q
		{ 12, 43, 11, 11 }, // r
		{ 23, 43, 11, 11 }, // s
		{ 34, 43, 10, 11 }, // t
		{ 44, 43, 11, 11 }, // u
		{ 55, 43, 12, 11 }, // v
		{ 67, 43, 13, 11 }, // w
		{ 80, 43, 13, 11 }, // x
		{ 93, 43, 10, 11 }, // y
		{ 103, 43, 11, 11 }, // z
		{ 0, 95, 108, 11 }, // Slider bar		0x74
		{ 108, 95, 6, 11 }, // Slider gem		0x75
		{ 0, 54, 32, 26 }, // Skull 1			0x76
		{ 32, 54, 32, 26 }, // Skull 2			0x77
		{ 64, 54, 32, 26 }, // Skull 3			0x78
		{ 96, 54, 32, 26 }, // Skull 4			0x79
		{ 128, 54, 32, 26 }, // Skull 5			0x7a
		{ 160, 54, 32, 26 }, // Skull 6			0x7b
		{ 192, 54, 32, 26 }, // Skull 7			0x7c
		{ 224, 54, 32, 26 }, // Skull 8			0x7d
		{ 134, 97, 7, 11 }, // Right arrow		0x7e
		{ 114, 95, 20, 18 }, // Select box		0x7f
		{ 105, 80, 15, 15 }, // Dpad left		0x80
		{ 120, 80, 15, 15 }, // Dpad right		0x81
		{ 135, 80, 15, 15 }, // Dpad up			0x82
		{ 150, 80, 15, 15 }, // Dpad down		0x83
		{ 45, 80, 15, 15 }, // C left button	0x84
		{ 60, 80, 15, 15 }, // C right button	0x85
		{ 75, 80, 15, 15 }, // C up button		0x86
		{ 90, 80, 15, 15 }, // C down button	0x87
		{ 165, 80, 15, 15 }, // L button		0x88
		{ 180, 80, 15, 15 }, // R button		0x89
		{ 0, 80, 15, 15 }, // A button			0x8a
		{ 15, 80, 15, 15 }, // B btton			0x8b
		{ 195, 80, 15, 15 }, // Z button		0x8c
		{ 30, 80, 15, 15 }, // Start button		0x8d
		{ 156, 96, 13, 13 }, // Down arrow		0x8e
		{ 143, 96, 13, 13 }, // Up arrow		0x8f
		{ 169, 96, 7, 13 }, // Left arrow		0x90
		//{134, 96,   7, 13}, // Right arrow Missing On Doom 64
	};

int card_x[6] = { (78 << 2), (89 << 2), (100 << 2),
		  (78 << 2), (89 << 2), (100 << 2) }; // 8005b870

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY 96
// Number of status faces.
#define ST_NUMPAINFACES 5
#define ST_NUMSTRAIGHTFACES 3
#define ST_NUMTURNFACES 2
#define ST_NUMSPECIALFACES 3

#define ST_FACESTRIDE \
	(ST_NUMSTRAIGHTFACES + ST_NUMTURNFACES + ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES 2

#define ST_NUMFACES (ST_FACESTRIDE * ST_NUMPAINFACES + ST_NUMEXTRAFACES)

#define ST_TURNOFFSET (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE (ST_NUMPAINFACES * ST_FACESTRIDE)
#define ST_DEADFACE (ST_GODFACE + 1)
#define ST_EVILGRINCOUNT (2 * TICRATE)
#define ST_STRAIGHTFACECOUNT (TICRATE / 2)
#define ST_TURNCOUNT (1 * TICRATE)
#define ST_OUCHCOUNT (1 * TICRATE)
#define ST_RAMPAGEDELAY (2 * TICRATE)

#define ST_MUCHPAIN 20
// used to use appopriately pained face
static int st_oldhealth = -1;

// count until face changes
static int st_facecount = 0;

// current face index, used by w_faces
static int st_faceindex = 0;

unsigned char *faces[ST_NUMFACES];

void ST_updateFaceWidget(void);

void ST_Init(void) // 80029BA0
{
	// these get pre-converted in r_data.c now
	//  sfontlump = (uint8_t *)W_CacheLumpName("SFONT",PU_STATIC,dec_jag);
	//  statuslump = (uint8_t *)W_CacheLumpName("STATUS",PU_STATIC,dec_jag);
	//  sumbolslump = W_GetNumForName("SYMBOLS");

	int facenum;
	// face states
	facenum = 0;

	faces[facenum++] = STFST00_bits;

	faces[facenum++] = STFST01_bits;
	faces[facenum++] = STFST02_bits;

	faces[facenum++] = STFTR00_bits;
	faces[facenum++] = STFTL00_bits;

	faces[facenum++] = STFOUCH0_bits;
	faces[facenum++] = STFEVL0_bits;
	faces[facenum++] = STFKILL0_bits;

	faces[facenum++] = STFST10_bits;
	faces[facenum++] = STFST11_bits;
	faces[facenum++] = STFST12_bits;

	faces[facenum++] = STFTR10_bits;
	faces[facenum++] = STFTL10_bits;

	faces[facenum++] = STFOUCH1_bits;
	faces[facenum++] = STFEVL1_bits;
	faces[facenum++] = STFKILL1_bits;

	faces[facenum++] = STFST20_bits;
	faces[facenum++] = STFST21_bits;
	faces[facenum++] = STFST22_bits;

	faces[facenum++] = STFTR20_bits;
	faces[facenum++] = STFTL20_bits;

	faces[facenum++] = STFOUCH2_bits;
	faces[facenum++] = STFEVL2_bits;
	faces[facenum++] = STFKILL2_bits;

	faces[facenum++] = STFST30_bits;
	faces[facenum++] = STFST31_bits;
	faces[facenum++] = STFST32_bits;

	faces[facenum++] = STFTR30_bits;
	faces[facenum++] = STFTL30_bits;

	faces[facenum++] = STFOUCH3_bits;
	faces[facenum++] = STFEVL3_bits;
	faces[facenum++] = STFKILL3_bits;

	faces[facenum++] = STFST40_bits;
	faces[facenum++] = STFST41_bits;
	faces[facenum++] = STFST42_bits;

	faces[facenum++] = STFTR40_bits;
	faces[facenum++] = STFTL40_bits;

	faces[facenum++] = STFOUCH4_bits;
	faces[facenum++] = STFEVL4_bits;
	faces[facenum++] = STFKILL4_bits;

	faces[facenum++] = STFGOD0_bits;
	faces[facenum++] = STFDEAD0_bits;
}

// used for evil grin
static bool oldweaponsowned[NUMWEAPONS];

// a random number per tick
static int st_randomnumber;

void ST_InitEveryLevel(void) // 80029C00
{
	player_t *plyr = &players[0];
	infraredFactor = 0;
	quakeviewy = 0;
	quakeviewx = 0;
	camviewpitch = 0;
	flashCards[0].active = false;
	flashCards[1].active = false;
	flashCards[2].active = false;
	flashCards[3].active = false;
	flashCards[4].active = false;
	flashCards[5].active = false;
	tryopen[0] = false;
	tryopen[1] = false;
	tryopen[2] = false;
	tryopen[3] = false;
	tryopen[4] = false;
	tryopen[5] = false;
	for (int i = 0; i < NUMWEAPONS; i++)
		oldweaponsowned[i] = plyr->weaponowned[i];
}

/*
====================
=
= ST_Ticker
=
====================
*/
extern int force_vmu_refresh;
void ST_Ticker(void) // 80029C88
{
	player_t *player;
	int ind;

	player = &players[0];

	/* */
	/* Countdown time for the message */
	/* */
	player->messagetic--;
	player->messagetic1--; // [Immorpher] decriment message buffer
	player->messagetic2--; // [Immorpher] decriment message buffer
	player->messagetic3--; // [Immorpher] decriment message buffer

	/* */
	/* Tried to open a CARD or SKULL door? */
	/* */
	for (ind = 0; ind < NUMCARDS; ind++) {
		if (tryopen[ind]) {
			/* CHECK FOR INITIALIZATION */
			tryopen[ind] = false;
			flashCards[ind].active = true;
			flashCards[ind].delay = FLASHDELAY - 1;
			flashCards[ind].times = FLASHTIMES + 1;
			flashCards[ind].doDraw = false;
		} else if (flashCards[ind].active && !--flashCards[ind].delay) {
			/* MIGHT AS WELL DO TICKING IN THE SAME LOOP! */
			flashCards[ind].delay = FLASHDELAY - 1;
			flashCards[ind].doDraw ^= 1;
			if (!--flashCards[ind].times)
				flashCards[ind].active = false;
			if (flashCards[ind].doDraw && flashCards[ind].active)
				S_StartSound(NULL, sfx_itemup);
		}
	}

	/* */
	/* Do flashes from damage/items */
	/* */
	if (cameratarget == player->mo) {
		ST_UpdateFlash();
	}

	if (demoplayback == false && (force_vmu_refresh || menu_settings.VmuDisplay)) {
		st_randomnumber = I_Random();

		ST_updateFaceWidget();

		// Update VMU
		I_VMUFB(force_vmu_refresh);

		if (force_vmu_refresh)
			force_vmu_refresh = 0;
	}
}

/*
====================
=
= ST_Drawer
=
====================
*/

pvr_sprite_hdr_t status_shdr;
pvr_sprite_cxt_t status_scxt;
pvr_sprite_txr_t status_stxr;

void ST_Drawer(void) // 80029DC0
{
	player_t *player;
	weapontype_t weapon;
	int ammo, ind, ms_alpha;

	float x1, x2, y1, y2, u1, u2, v1, v2;

	status_stxr.flags = PVR_CMD_VERTEX_EOL;
	status_stxr.az = 8.9999999f;
	status_stxr.bz = 8.9999999f;
	status_stxr.cz = 8.9999999f;
	status_stxr.dummy = 0;

	player = &players[0];

	/* */
	/* Draw Text Message */
	/* */

	// [Immorpher] only display messages and calculate if global tic is active
	if ((menu_settings.enable_messages) && players[0].messagetic > 0) {
		// [Immorpher] new global tic indicates new message to add
		if (players[0].messagetic != players[0].messagetic1) {
			// Sequentially shift messages to lower states
			players[0].message3 = players[0].message2;
			players[0].messagetic3 = players[0].messagetic2;
			players[0].messagecolor3 = players[0].messagecolor2;

			players[0].message2 = players[0].message1;
			players[0].messagetic2 = players[0].messagetic1;
			players[0].messagecolor2 = players[0].messagecolor1;

			players[0].message1 = players[0].message;
			players[0].messagetic1 = players[0].messagetic;
			players[0].messagecolor1 = players[0].messagecolor;
		}
		// display message 1
		if (players[0].messagetic1 > 0) {
			// set message alpha
			ms_alpha = players[0].messagetic1 * 8;
			if (ms_alpha >= 196)
				ms_alpha = 196;

			ST_Message(2 + menu_settings.HUDmargin, menu_settings.HUDmargin, players[0].message1,
				ms_alpha | players[0].messagecolor1, ST_BELOW_OVL); // display message
		}

		// display message 2
		if (players[0].messagetic2 > 0) {
			// set message alpha
			ms_alpha = players[0].messagetic2 * 8;
			if (ms_alpha >= 196)
				ms_alpha = 196;

			ST_Message(2 + menu_settings.HUDmargin, 10 + menu_settings.HUDmargin, players[0].message2,
				ms_alpha | players[0].messagecolor2, ST_BELOW_OVL); // display message
		}

		// display message 3
		if (players[0].messagetic3 > 0) {
			// set message alpha
			ms_alpha = players[0].messagetic3 * 8;
			if (ms_alpha >= 196)
				ms_alpha = 196;

			ST_Message(2 + menu_settings.HUDmargin, 20 + menu_settings.HUDmargin, players[0].message3,
				ms_alpha | players[0].messagecolor3, ST_BELOW_OVL); // display message
		}
	}

	if (menu_settings.HUDopacity) {
		uint32_t HUDcolor = PACKRGBA(224, 0, 0, menu_settings.HUDopacity);

		/* */
		/* Gray color */
		/* */
		{
			int xpos = 2 + menu_settings.HUDmargin;
			int ypos = 218 - menu_settings.HUDmargin;
			int hw = (42 - 2);
			int hh = (224 - 218);
			x1 = xpos * (float)RES_RATIO;
			x2 = (xpos + hw) * (float)RES_RATIO;
			y1 = ypos * (float)RES_RATIO;
			y2 = (ypos + hh) * (float)RES_RATIO;
			u1 = (float)0.0f;
			u2 = (float)hw / 128.0f;
			v1 = (float)0.0f;
			v2 = (float)hh / 16.0f;

			status_shdr.argb = D64_PVR_PACK_COLOR(
					0x80, 0x80, 0x80,
					menu_settings.HUDopacity);
			status_stxr.ax = x1;
			status_stxr.ay = y2;
			status_stxr.bx = x1;
			status_stxr.by = y1;
			status_stxr.cx = x2;
			status_stxr.cy = y1;
			status_stxr.dx = x2;
			status_stxr.dy = y2;
			status_stxr.auv = PVR_PACK_16BIT_UV(u1, v2);
			status_stxr.buv = PVR_PACK_16BIT_UV(u1, v1);
			status_stxr.cuv = PVR_PACK_16BIT_UV(u2, v1);

			pvr_list_prim(PVR_LIST_TR_POLY, &status_shdr,
						sizeof(pvr_sprite_hdr_t));
			pvr_list_prim(PVR_LIST_TR_POLY, &status_stxr,
						sizeof(pvr_sprite_txr_t));
		}
		/* */
		/* Armor */
		/* */
		{
			int xpos = 280 - menu_settings.HUDmargin;
			int ypos = 218 - menu_settings.HUDmargin;
			int hw = (316 - 280);
			int hh = (224 - 218);
			x1 = xpos * (float)RES_RATIO;
			x2 = (xpos + hw) * (float)RES_RATIO;
			y1 = ypos * (float)RES_RATIO;
			y2 = (ypos + hh) * (float)RES_RATIO;
			u1 = 40.0f / 128.0f;
			u2 = (float)(40.0f + hw) / 128.0f;
			v1 = 0.0f;
			v2 = (float)hh / 16.0f;

			status_stxr.ax = x1;
			status_stxr.ay = y2;
			status_stxr.bx = x1;
			status_stxr.by = y1;
			status_stxr.cx = x2;
			status_stxr.cy = y1;
			status_stxr.dx = x2;
			status_stxr.dy = y2;
			status_stxr.auv = PVR_PACK_16BIT_UV(u1, v2);
			status_stxr.buv = PVR_PACK_16BIT_UV(u1, v1);
			status_stxr.cuv = PVR_PACK_16BIT_UV(u2, v1);

			pvr_list_prim(PVR_LIST_TR_POLY, &status_stxr,
						sizeof(pvr_sprite_txr_t));
		}
		/* */
		/* White color */
		/* */
		/* */
		/* Cards & skulls */
		/* */
		for (ind = 0; ind < NUMCARDS; ind++) {
			if (player->cards[ind] || (flashCards[ind].active &&
						   flashCards[ind].doDraw)) {
				/* */
				/* Draw Keys Graphics */
				/* */
				int xpos = card_x[ind] >> 2;
				int ypos = 230 - menu_settings.HUDmargin;
				int hw = 9;
				int hh = 10;
				x1 = (float)xpos * (float)RES_RATIO;
				x2 = (float)(xpos + hw) * (float)RES_RATIO;
				y1 = (float)ypos * (float)RES_RATIO;
				y2 = (float)(ypos + hh) * (float)RES_RATIO;
				u1 = (float)(ind * 9) / 128.0f;
				u2 = (float)((float)(ind * 9) + hw) / 128.0f;
				v1 = 6.0f / 16.0f;
				v2 = (6.0f + (float)hh) / 16.0f;

				status_shdr.argb = D64_PVR_PACK_COLOR(
						0xff, 0xff, 0xff,
						menu_settings.HUDopacity);
				status_stxr.ax = x1;
				status_stxr.ay = y2;
				status_stxr.bx = x1;
				status_stxr.by = y1;
				status_stxr.cx = x2;
				status_stxr.cy = y1;
				status_stxr.dx = x2;
				status_stxr.dy = y2;
				status_stxr.auv = PVR_PACK_16BIT_UV(u1, v2);
				status_stxr.buv = PVR_PACK_16BIT_UV(u1, v1);
				status_stxr.cuv = PVR_PACK_16BIT_UV(u2, v1);

				pvr_list_prim(PVR_LIST_TR_POLY, &status_shdr,
							sizeof(pvr_sprite_hdr_t));
				pvr_list_prim(PVR_LIST_TR_POLY, &status_stxr,
							sizeof(pvr_sprite_txr_t));
			}
		}

		/* */
		/* Ammo */
		/* */
		weapon = player->pendingweapon;

		if (weapon == wp_nochange)
			weapon = player->readyweapon;

#ifdef OSDSHOWFPS
		ammo = (int)last_fps;
		ST_Message(148, 227 - menu_settings.HUDmargin - 10, "FPS", 0x80808000 | menu_settings.HUDopacity, ST_BELOW_OVL);
		ST_DrawNumber(160, 227 - menu_settings.HUDmargin, ammo, 0, PACKRGBA(224, 0, 0, menu_settings.HUDopacity), ST_BELOW_OVL);
#else
		if (weaponinfo[weapon].ammo != am_noammo) {
			if (menu_settings.ColoredHUD) {
				switch (weaponinfo[weapon].ammo) {
				case am_clip:
					HUDcolor = PACKRGBA(96, 96, 128, menu_settings.HUDopacity);
					break;
				case am_shell:
					HUDcolor = PACKRGBA(196, 32, 0, menu_settings.HUDopacity);
					break;
				case am_cell:
					HUDcolor = PACKRGBA(0, 96, 128, menu_settings.HUDopacity);
					break;
				default:
					HUDcolor = PACKRGBA(164, 96, 0, menu_settings.HUDopacity);
					break;
				}
			}

			ammo = player->ammo[weaponinfo[weapon].ammo];
			if (ammo < 0)
				ammo = 0;

			ST_DrawNumber(160, 227 - menu_settings.HUDmargin, ammo, 0, HUDcolor, ST_BELOW_OVL);
		}
#endif

		/* */
		/* Health */
		/* */
		if (menu_settings.ColoredHUD) {
			if (player->health <= 67)
				HUDcolor = PACKRGBA(224 - 96 * player->health / 67, 128 * player->health / 67, 0, menu_settings.HUDopacity);
			else if (player->health <= 133)
				HUDcolor = PACKRGBA(256 - 256 * player->health / 133, 128, 64 * player->health / 133 - 32, menu_settings.HUDopacity);
			else
				HUDcolor = PACKRGBA(0, 256 - 192 * player->health / 200, 288 * player->health / 200 - 160, menu_settings.HUDopacity);
		}

		ST_DrawNumber(22 + menu_settings.HUDmargin, 227 - menu_settings.HUDmargin, player->health, 0, HUDcolor, ST_BELOW_OVL);

		/* */
		/* Armor */
		/* */
		if (menu_settings.ColoredHUD) {
			if (player->armorpoints == 0)
				HUDcolor = PACKRGBA(224, 0, 0, menu_settings.HUDopacity);
			else if (player->armortype == 1)
				HUDcolor = PACKRGBA(0, 128, 64, menu_settings.HUDopacity);
			else
				HUDcolor = PACKRGBA(0, 64, 128, menu_settings.HUDopacity);
		}

		ST_DrawNumber(298 - menu_settings.HUDmargin, 227 - menu_settings.HUDmargin, player->armorpoints, 0, HUDcolor, ST_BELOW_OVL);
	}
}

#define ST_FONTWHSIZE 8
pvr_ptr_t pvrfont;

pvr_sprite_hdr_t font_shdr;
pvr_sprite_cxt_t font_scxt;
pvr_sprite_txr_t font_stxr;

void ST_Message(int x, int y, char *text, uint32_t color, int prio)
{
	uint8_t c;
	int s, t;
	int xpos, ypos;

	float x1, x2, y1, y2, u1, u2, v1, v2;

	if (!text || text[0] == '\0') {
		return;
	}

	font_stxr.flags = PVR_CMD_VERTEX_EOL;
	if (prio == ST_ABOVE_OVL) {
		font_stxr.az = 10.0000001f;
		font_stxr.bz = 10.0000001f;
		font_stxr.cz = 10.0000001f;
	} else {
		font_stxr.az = 8.9999999f;
		font_stxr.bz = 8.9999999f;
		font_stxr.cz = 8.9999999f;
	}
	font_stxr.dummy = 0;
	font_shdr.argb = D64_PVR_REPACK_COLOR(color);

	ypos = y;
	xpos = x;

	pvr_list_prim(PVR_LIST_TR_POLY, &font_shdr, sizeof(pvr_sprite_hdr_t));

	while (*text) {
		c = *text;

		if (c == '\n') {
			ypos += (ST_FONTWHSIZE + 1);
			xpos = x;
		} else {
			if (c >= 'a' && c <= 'z')
				c -= (26 + 6);

			if (c >= '!' && c <= '_') {
				if ((c - '!') < 32)
					t = 0;
				else
					t = ST_FONTWHSIZE;

				s = ((c - '!') & ~32) * ST_FONTWHSIZE;

				x1 = (float)xpos * (float)RES_RATIO;
				x2 = (float)(xpos + ST_FONTWHSIZE) * (float)RES_RATIO;
				y1 = (float)ypos * (float)RES_RATIO;
				y2 = (float)(ypos + ST_FONTWHSIZE) * (float)RES_RATIO;
				u1 = (float)s / 256.0f;
				u2 = (float)(s + ST_FONTWHSIZE) / 256.0f;
				v1 = (float)t / 16.0f;
				v2 = (float)(t + ST_FONTWHSIZE) / 16.0f;

				font_stxr.ax = x1;
				font_stxr.ay = y2;
				font_stxr.bx = x1;
				font_stxr.by = y1;
				font_stxr.cx = x2;
				font_stxr.cy = y1;
				font_stxr.dx = x2;
				font_stxr.dy = y2;
				font_stxr.auv = PVR_PACK_16BIT_UV(u1, v2);
				font_stxr.buv = PVR_PACK_16BIT_UV(u1, v1);
				font_stxr.cuv = PVR_PACK_16BIT_UV(u2, v1);
				pvr_list_prim(PVR_LIST_TR_POLY, &font_stxr, sizeof(pvr_sprite_txr_t));
			}
			xpos += ST_FONTWHSIZE;
		}
		text++;
	}
}

void ST_DrawNumber(int x, int y, int value, int mode, uint32_t color, int prio)
{
	int index, width, i;
	int number[16];

	width = 0;
	for (index = 0; index < 16; index++) {
		number[index] = value % 10;
		width += symboldata[number[index]].w;

		value /= 10;
		if (!value)
			break;
	}

	switch (mode) {
	case 0: /* Center */
		x -= (width / 2);
	case 1: /* Right */
		while (index >= 0) {
			ST_DrawSymbol(x, y, number[index], color, prio);
			x += symboldata[number[index]].w;
			index--;
		}
		break;
	case 2: /* Left */
		i = 0;
		while (index >= 0) {
			x -= symboldata[number[i]].w;
			ST_DrawSymbol(x, y, number[i], color, prio);
			i++;
			index--;
		}
		break;
	default:
		break;
	}
}

void ST_DrawString(int x, int y, char *text, uint32_t color, int prio)
{
	unsigned char c;
	int xpos, ypos, index;

	xpos = x;
	if (xpos <= -1)
		xpos = ST_GetCenterTextX(text);

	ypos = y;

	while (*text) {
		int cur_y;

		c = *text;
		// [jnmartin84] newline support :-)
		if (c == '\n') {
			ypos += 16;
			xpos = x + 8;
			text++;
			continue;
		}

		cur_y = ypos;

		if (c >= 'A' && c <= 'Z') {
			index = (c - 'A') + 16;
		} else if (c >= 'a' && c <= 'z') {
			index = (c - 'a') + 42;
			cur_y = ypos + 2;
		} else if (c >= '0' && c <= '9') {
			index = (c - '0') + 0;
		} else if (c == '!') {
			index = 12;
			cur_y = ypos - 1;
		} else if (c == '-') {
			index = 10;
		} else if (c == '.') {
			index = 13;
		} else if (c == ':') {
			index = 15;
		} else if (c == '?') {
			index = 14;
		} else if (c == '%') {
			index = 11;
		} else if (c >= FIRST_SYMBOL && c <= LAST_SYMBOL) {
			index = (c - '0');
		} else {
			xpos += 6; /* space */
			text++;
			continue;
		}

		ST_DrawSymbol(xpos, cur_y, index, color, prio);
		xpos += symboldata[index].w;

		text++;
	}
}

int ST_GetCenterTextX(char *text)
{
	unsigned char c;
	int xpos, index;

	xpos = 0;
	while (*text) {
		c = *text;

		if (c >= 'A' && c <= 'Z') {
			index = (c - 'A') + 16;
		} else if (c >= 'a' && c <= 'z') {
			index = (c - 'a') + 42;
		} else if (c >= '0' && c <= '9') {
			index = (c - '0') + 0;
		} else if (c == '!') {
			index = 12;
		} else if (c == '-') {
			index = 10;
		} else if (c == '.') {
			index = 13;
		} else if (c == ':') {
			index = 15;
		} else if (c == '?') {
			index = 14;
		} else if (c == '%') {
			index = 11;
		} else if (c >= FIRST_SYMBOL && c <= LAST_SYMBOL) {
			index = (c - '0');
		} else {
			xpos += 6; /* space */
			text++;
			continue;
		}

		xpos += symboldata[index].w;

		text++;
	}

	return (320 - xpos) / 2;
}

#define ST_MAXDMGCOUNT 144
#define ST_MAXSTRCOUNT 32
#define ST_MAXBONCOUNT 100

void ST_UpdateFlash(void)
{
	player_t *plyr;
	int cnt;
	int bzc;
	int bnc;

	plyr = &players[0];

	if ((plyr->f_powers[pw_infrared] < 120) && infraredFactor) {
		infraredFactor -= 4;
		if (infraredFactor < 0) {
			infraredFactor = 0;
		}

		P_RefreshBrightness();
	}

	if (plyr->f_powers[pw_invulnerability] >= 61 || ((int)plyr->f_powers[pw_invulnerability]) & 8) {
		/* invulnerability flash (white) */
		FlashEnvColor = PACKRGBA(128, 128, 128, 255);
	} else if ((int)plyr->f_bfgcount > 0) {
		/* bfg flash (green)*/
		FlashEnvColor = PACKRGBA(0, (int)plyr->f_bfgcount, 0, 255);
	} else {
		/* damage and strength flash (red) */
		cnt = (int)plyr->f_damagecount;

		if (cnt) {
			if ((cnt + 16) > ST_MAXDMGCOUNT)
				cnt = ST_MAXDMGCOUNT;
		}

		if (plyr->f_powers[pw_strength] <= ST_MAXSTRCOUNT) {
			/* slowly fade the berzerk out */
			bzc = (int)plyr->f_powers[pw_strength];

			if (bzc == 1)
				bzc = 0;
		} else {
			bzc = ST_MAXSTRCOUNT;
		}

		if ((cnt != 0) || (bzc != 0)) {
			FlashEnvColor = PACKRGBA(0, 0, 0, 0);
			if (bzc < cnt) {
				if (cnt != 0)
					FlashEnvColor = PACKRGBA(cnt, 0, 0, 255);
			} else {
				if (bzc != 0)
					FlashEnvColor = PACKRGBA(bzc, 0, 0, 255);
			}
		} else if (plyr->f_powers[pw_ironfeet] >= 61 || ((int)plyr->f_powers[pw_ironfeet]) & 8) {
			/* suit flash (green/yellow) */
			FlashEnvColor = PACKRGBA(0, 32, 4, 255);
		} else if ((int)plyr->f_bonuscount > 0) {
			/* bonus flash (yellow) */
			//cnt = FlashBrightness*((plyr->bonuscount+7)/8)/32;
			cnt = FLASH_BRIGHTNESS * (((int)plyr->f_bonuscount + 7) / 8) / 32;

			if (cnt > ST_MAXBONCOUNT)
				cnt = ST_MAXBONCOUNT;

			//bnc = (cnt << 2) + cnt << 1;
			bnc = cnt * 10;

			FlashEnvColor = PACKRGBA(bnc, bnc, cnt, 255);
		} else {
			/* Default Flash */
			FlashEnvColor = PACKRGBA(0, 0, 0, 0);
		}
	}
}

extern uint16_t *symbols16;
extern float recip_symw;
extern float recip_symh;
extern int symbols16_size;
extern int rawsymbol_w;
extern int rawsymbol_h;
pvr_ptr_t pvr_symbols;

pvr_sprite_hdr_t symbols_shdr;
pvr_sprite_cxt_t symbols_scxt;
pvr_sprite_txr_t symbols_stxr;

void ST_DrawSymbol(int xpos, int ypos, int index, uint32_t color, int prio)
{
	symboldata_t *symbol;

	symbol = &symboldata[index];
	uint32_t symw = symbol->w;
	uint32_t symh = symbol->h;

	float x1 = xpos;
	float x2 = x1 + symw;

	float yd = 0.0f;
	float y1 = ypos;
	if (ypos < 0) {
		y1 = 0.0f;
		yd = -ypos;
	}
	float y2 = y1 + (symh - yd);

	float u1, u2, v1, v2;

	u1 = (float)symbol->x * recip_symw;
	if (ypos < 0)
		v1 = (float)((float)symbol->y + yd) * recip_symh;
	else
		v1 = (float)symbol->y * recip_symh;

	u2 = (float)((float)symbol->x + (float)symbol->w) * recip_symw;
	v2 = (float)((float)symbol->y + (float)symbol->h) * recip_symh;

	symbols_stxr.flags = PVR_CMD_VERTEX_EOL;
	if (prio == ST_ABOVE_OVL) {
		symbols_stxr.az = 10.0000001f;
		symbols_stxr.bz = 10.0000001f;
		symbols_stxr.cz = 10.0000001f;
	} else {
		symbols_stxr.az = 8.9999999f;
		symbols_stxr.bz = 8.9999999f;
		symbols_stxr.cz = 8.9999999f;
	}
	symbols_stxr.dummy = 0;
	symbols_shdr.argb = D64_PVR_REPACK_COLOR(color);

	x1 *= (float)RES_RATIO;
	x2 *= (float)RES_RATIO;
	y1 *= (float)RES_RATIO;
	y2 *= (float)RES_RATIO;

	symbols_stxr.ax = x1;
	symbols_stxr.ay = y2;
	symbols_stxr.bx = x1;
	symbols_stxr.by = y1;
	symbols_stxr.cx = x2;
	symbols_stxr.cy = y1;
	symbols_stxr.dx = x2;
	symbols_stxr.dy = y2;
	symbols_stxr.auv = PVR_PACK_16BIT_UV(u1, v2);
	symbols_stxr.buv = PVR_PACK_16BIT_UV(u1, v1);
	symbols_stxr.cuv = PVR_PACK_16BIT_UV(u2, v1);
	pvr_list_prim(PVR_LIST_TR_POLY, &symbols_shdr, sizeof(pvr_sprite_hdr_t));
	pvr_list_prim(PVR_LIST_TR_POLY, &symbols_stxr, sizeof(pvr_sprite_txr_t));
}

int ST_calcPainOffset(void)
{
	static int lastcalc;
	static int oldhealth = -1;

	player_t *plyr = &players[0];
	int health;

	health = plyr->health > 100 ? 100 : plyr->health;

	if (health != oldhealth) {
		lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
		oldhealth = health;
	}

	return lastcalc;
}

void ST_drawVMUFace(void)
{
	I_VMUUpdateFace(faces[st_faceindex], force_vmu_refresh);
}

//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
void ST_updateFaceWidget(void)
{
	player_t *plyr = &players[0];

	int i;
	angle_t badguyangle;
	angle_t diffang;
	static int lastattackdown = -1;
	static int priority = 0;
	static int last_priority = -1;
	bool doevilgrin;

	int new_faceindex = 0;

	if (force_vmu_refresh) {
		ST_drawVMUFace();
		return;
	}

	if (priority < 10) {
		// dead
		if (!plyr->health) {
			last_priority = priority;
			priority = 9;
			new_faceindex = ST_DEADFACE;
			if (new_faceindex != st_faceindex) {
				st_faceindex = new_faceindex;
				ST_drawVMUFace();
			}
			st_facecount = 1;
		}
	}

	if (priority < 9) {
		if (plyr->f_bonuscount > 0) {
			// picking up bonus
			doevilgrin = false;

			for (i = 0; i < NUMWEAPONS; i++) {
				if (oldweaponsowned[i] != plyr->weaponowned[i]) {
					doevilgrin = true;
					oldweaponsowned[i] = plyr->weaponowned[i];
				}
			}

			if (doevilgrin) {
				// evil grin if just picked up weapon
				last_priority = priority;
				priority = 8;
				st_facecount = ST_EVILGRINCOUNT;
				new_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
				if (new_faceindex != st_faceindex) {
					st_faceindex = new_faceindex;
					ST_drawVMUFace();
				}
			}
		}
	}

	if (priority < 8) {
		if ((int)plyr->f_damagecount && plyr->attacker && plyr->attacker != plyr->mo) {
			last_priority = priority;
			// being attacked
			priority = 7;

			if (st_oldhealth - plyr->health > ST_MUCHPAIN) {
				st_facecount = ST_TURNCOUNT;
				new_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
				if (new_faceindex != st_faceindex) {
					st_faceindex = new_faceindex;
					ST_drawVMUFace();
				}
			} else {
				badguyangle = R_PointToAngle2(plyr->mo->x, plyr->mo->y, plyr->attacker->x, plyr->attacker->y);

				if (badguyangle > plyr->mo->angle) {
					// whether right or left
					diffang = badguyangle - plyr->mo->angle;
					i = diffang > ANG180;
				} else {
					// whether left or right
					diffang = plyr->mo->angle - badguyangle;
					i = diffang <= ANG180;
				} // confusing, aint it?

				st_facecount = ST_TURNCOUNT;
				new_faceindex = ST_calcPainOffset();

				if (diffang < ANG45) // head-on
					new_faceindex += ST_RAMPAGEOFFSET;
				else if (i) // turn face right
					new_faceindex += ST_TURNOFFSET;
				else // turn face left
					new_faceindex += ST_TURNOFFSET + 1;

				if (new_faceindex != st_faceindex) {
					st_faceindex = new_faceindex;
					ST_drawVMUFace();
				}
			}
		}
	}

	if (priority < 7) {
		// getting hurt because of your own damn stupidity
		if ((int)plyr->f_damagecount) {
			if (st_oldhealth - plyr->health > ST_MUCHPAIN) {
				last_priority = priority;
				priority = 7;
				st_facecount = ST_TURNCOUNT;
				new_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
				if (new_faceindex != st_faceindex) {
					st_faceindex = new_faceindex;
					ST_drawVMUFace();
				}
			} else {
				last_priority = priority;
				priority = 6;
				st_facecount = ST_TURNCOUNT;
				new_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
				if (new_faceindex != st_faceindex) {
					st_faceindex = new_faceindex;
					ST_drawVMUFace();
				}
			}
		}
	}

	if (priority < 6) {
		// rapid firing
		if (plyr->attackdown) {
			if (lastattackdown == -1) {
				lastattackdown = ST_RAMPAGEDELAY;
			} else if (!--lastattackdown) {
				last_priority = priority;
				priority = 5;
				new_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
				if (new_faceindex != st_faceindex) {
					st_faceindex = new_faceindex;
					ST_drawVMUFace();
				}
				st_facecount = 1;
				lastattackdown = 1;
			}
		} else {
			lastattackdown = -1;
		}
	}

	if (priority < 5) {
		// invulnerability
		if ((plyr->cheats & CF_GODMODE) || (plyr->f_powers[pw_invulnerability] > 0)) {
			last_priority = priority;
			priority = 4;

			st_faceindex = ST_GODFACE;

			if (last_priority != priority) {
				ST_drawVMUFace();
			}
			st_facecount = 1;
		}
	}

	// look left or look right if the facecount has timed out
	if (!st_facecount) {
		new_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
		if (new_faceindex != st_faceindex) {
			st_faceindex = new_faceindex;
			ST_drawVMUFace();
		}

		st_facecount = ST_STRAIGHTFACECOUNT;
		last_priority = priority;
		priority = 0;
	}

	st_facecount--;
}
