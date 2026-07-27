/* D_screens.c */

#include "i_main.h"
#include "doomdef.h"
#include "r_local.h"
#include "st_main.h"

int D_RunDemo(char *name, skill_t skill, int map)
{
	int lump;
	int exit;

	demo_p = Z_Alloc(16000, PU_STATIC, NULL);
	memset(demo_p, 0, 16000);

	lump = W_GetNumForName(name);
	W_ReadLump(lump, demo_p, dec_d64);

	// demo data needs endian-swapping
	for (int i = 0; i < 4000; i++)
		demo_p[i] = Swap32(demo_p[i]);

	exit = G_PlayDemoPtr(skill, map);
	Z_Free(demo_p);

	return exit;
}

int D_TitleMap(void)
{
	int exit;

	D_OpenControllerPak();

	demo_p = Z_Alloc(16000, PU_STATIC, NULL);
	memset(demo_p, 0, 16000);
	memcpy(demo_p, DefaultConfiguration, 13 * sizeof(int));
	exit = G_PlayDemoPtr(sk_medium, 33);
	Z_Free(demo_p);

	return exit;
}

int D_WarningTicker(void)
{
	static int last_f_gametic = 0;

	if (((int)f_gamevbls < (int)f_gametic) && !(((int)f_gametic) & 7)) {
		// get used to seeing this idiom everywhere
		// it is forcing the original gametic update behavior for menu tics
		if (last_f_gametic != (int)f_gametic) {
			last_f_gametic = (int)f_gametic;
			MenuAnimationTic = (MenuAnimationTic + 1) & 7;
		}
	}

	return 0;
}

void D_DrawWarning(void)
{
	I_ClearFrame();

	if (MenuAnimationTic & 1)
		ST_DrawString(-1, 30, "WARNING!", 0xc00000ff, ST_ABOVE_OVL);

	ST_DrawString(-1, 60, "dreamcast controller", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 80, "is not connected.", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 120, "please turn off your", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 140, "dreamcast system.", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 180, "plug in your dreamcast", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 200, "controller and turn it on.", 0xffffffff, ST_ABOVE_OVL);

	I_DrawFrame();
}

int D_LegalTicker(void)
{
	if ((ticon - last_ticon) >= 150) { // 5 * TICRATE
		text_alpha -= 8;
		if (text_alpha < 0) {
			text_alpha = 0;
			return 8;
		}
	}
	return 0;
}

void D_DrawLegal(void)
{
	I_ClearFrame();

	M_DrawBackground(USLEGAL, text_alpha);

	if (FilesUsed > -1)
		ST_DrawString(-1, 200, "hold \x8d to manage vmu", text_alpha | 0xffffff00, ST_ABOVE_OVL);

	I_DrawFrame();
}

int D_NoPakTicker(void)
{
	if ((ticon - last_ticon) >= 180) // 6 * TICRATE
		return 8;

	return 0;
}

void D_DrawNoPak(void)
{
	I_ClearFrame();

	ST_DrawString(-1, 40, "no vmu.", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 60, "your game cannot", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 80, "be saved.", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 120, "please turn off your", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 140, "dreamcast system", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 160, "before inserting a", 0xffffffff, ST_ABOVE_OVL);
	ST_DrawString(-1, 180, "vmu.", 0xffffffff, ST_ABOVE_OVL);

	I_DrawFrame();
}

void D_SplashScreen(void)
{
	// Check if any dreamcast controller is connected
	// if not connected, it will show the Warning screen
	if (!maple_devices[maple_controller])
		MiniLoop(NULL, NULL, D_WarningTicker, D_DrawWarning);

	/* */
	/* Check if the n64 controller Pak is connected */
	/* */
	I_CheckControllerPak();

	/* */
	/* if not connected, it will show the NoPak screen */
	/* */
	if (FilesUsed < 0) {
		last_ticon = 0;
		MiniLoop(NULL, NULL, D_NoPakTicker, D_DrawNoPak);
	}

	// show the legals screen
	text_alpha = 0xff;
	last_ticon = 0;
	MiniLoop(NULL, NULL, D_LegalTicker, D_DrawLegal);
}

static int cred_step;
static int cred1_alpha;
static int cred2_alpha;
static int cred_next;

int D_Credits(void)
{
	int exit;

	cred_next = 0;
	cred1_alpha = 0;
	cred2_alpha = 0;
	cred_step = 0;
	exit = MiniLoop(NULL, NULL, D_CreditTicker, D_CreditDrawer);

	// if you exit while screens are up you can end up with colored background in main menu
	pvr_set_bg_color(0, 0, 0);

	return exit;
}

int D_CreditTicker(void)
{
	if (((uint32_t)ticbuttons[0] >> 16) != 0)
		return ga_exit;

	if ((cred_next == 0) || (cred_next == 1)) {
		if (cred_step == 0) {
			cred1_alpha += 8;
			if (cred1_alpha >= 255) {
				cred1_alpha = 0xff;
				cred_step = 1;
			}
		} else if (cred_step == 1) {
			cred2_alpha += 8;
			if (cred2_alpha >= 255) {
				cred2_alpha = 0xff;
				last_ticon = ticon;
				cred_step = 2;
			}
		} else if (cred_step == 2) {
			if ((ticon - last_ticon) >= 180) // 6 * TICRATE
				cred_step = 3;
		} else {
			cred1_alpha -= 8;
			cred2_alpha -= 8;
			if (cred1_alpha < 0) {
				cred_next += 1;
				cred1_alpha = 0;
				cred2_alpha = 0;
				cred_step = 0;
			}
		}
	} else if (cred_next == 2)
		return ga_exitdemo;

	return ga_nothing;
}

void D_CreditDrawer(void)
{
	int color;

	I_ClearFrame();

	if (cred_next == 0) {
		// Set Background Color (Dark Blue)
		color = (cred1_alpha * 16) / 255;
		pvr_set_bg_color(0, 0, (float)color / 255.0f);

		M_DrawBackground(IDCRED1, cred1_alpha);
		M_DrawBackground(IDCRED2, cred2_alpha);
	} else {
		if ((cred_next == 1) || (cred_next == 2)) {
			// Set Background Color (Dark Grey)
			float fcol;
			color = (cred1_alpha * 30) / 255;
			fcol = (float)color / 255.0f;
			pvr_set_bg_color(fcol, fcol, fcol);

			M_DrawBackground(WMSCRED1, cred1_alpha);
			M_DrawBackground(WMSCRED2, cred2_alpha);
		}
	}

	I_DrawFrame();
}

void D_OpenControllerPak(void)
{
	unsigned int oldbuttons;

	oldbuttons = I_GetControllerData();

	if (((oldbuttons & 0xffff0000) == PAD_START) && (I_CheckControllerPak() == 0)) {
		MenuCall = M_ControllerPakDrawer;
		linepos = 0;
		cursorpos = 0;
		in_menu = 1;
		MiniLoop(M_FadeInStart, M_MenuClearCall, M_ScreenTicker, M_MenuGameDrawer);
		in_menu = 0;
		I_WIPE_FadeOutScreen();
	}
}

