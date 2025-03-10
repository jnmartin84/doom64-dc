/* D_main.c  */
#include "i_main.h"
#include "doomdef.h"
#include "p_spec.h"
#include "r_local.h"
#include <stdatomic.h>

//
// PROTOTYPES
//

void D_DoomMain(void);
int P_Random(void);
int M_Random(void);
int I_Random(void);
void M_ClearRandom(void);

//
// MACROS
//

#define NS_PER_VBL 16666667

//
// GLOBALS
//

// M_Random
// Returns a 0-255 number
const unsigned char rndtable[256] = {
	0,   8,	  109, 220, 222, 241, 149, 107, 75,  248, 254, 140, 16,	 66,
	74,  21,  211, 47,  80,	 242, 154, 27,	205, 128, 161, 89,  77,	 36,
	95,  110, 85,  48,  212, 140, 211, 249, 22,  79,  200, 50,  28,	 188,
	52,  140, 202, 120, 68,	 145, 62,  70,	184, 190, 91,  197, 152, 224,
	149, 104, 25,  178, 252, 182, 202, 182, 141, 197, 4,   81,  181, 242,
	145, 42,  39,  227, 156, 198, 225, 193, 219, 93,  122, 175, 249, 0,
	175, 143, 70,  239, 46,	 246, 163, 53,	163, 109, 168, 135, 2,	 235,
	25,  92,  20,  145, 138, 77,  69,  166, 78,  176, 173, 212, 166, 113,
	94,  161, 41,  50,  239, 49,  111, 164, 70,  60,  2,   37,  171, 75,
	136, 156, 11,  56,  42,	 146, 138, 229, 73,  146, 77,  61,  98,	 196,
	135, 106, 63,  197, 195, 86,  96,  203, 113, 101, 170, 247, 181, 113,
	80,  250, 108, 7,   255, 237, 129, 226, 79,  107, 112, 166, 103, 241,
	24,  223, 239, 120, 198, 58,  60,  82,	128, 3,	  184, 66,  143, 224,
	145, 224, 81,  206, 163, 45,  63,  90,	168, 114, 59,  33,  159, 95,
	28,  139, 123, 98,  125, 196, 15,  70,	194, 253, 54,  14,  109, 226,
	71,  17,  161, 93,  186, 87,  244, 138, 20,  52,  123, 251, 26,	 36,
	17,  46,  52,  231, 232, 76,  31,  221, 84,  37,  216, 165, 212, 106,
	197, 242, 98,  43,  39,	 175, 254, 145, 190, 84,  118, 222, 187, 136,
	120, 163, 236, 249
};

// [Immorpher] - table to optionally boost brightness
unsigned char lightcurve[256] = {
	0,   1,	  3,   4,   6,	 7,   9,   11,	13,  14,  16,  18,  20,
	22,  24,  26,  28,  30,	 32,  34,  36,	38,  40,  42,  45,  47,
	49,  51,  53,  55,  58,	 60,  62,  64,	66,  69,  71,  73,  75,
	77,  80,  82,  84,  86,	 89,  91,  93,	95,  97,  100, 102, 104,
	106, 109, 111, 113, 115, 117, 120, 122, 124, 126, 128, 130, 133,
	135, 137, 139, 141, 143, 145, 148, 150, 152, 154, 156, 158, 160,
	162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182, 184, 186,
	188, 190, 192, 194, 195, 197, 199, 201, 202, 202, 203, 204, 205,
	205, 206, 207, 208, 208, 209, 210, 210, 211, 212, 212, 213, 214,
	214, 215, 216, 216, 217, 218, 218, 219, 219, 220, 221, 221, 222,
	222, 223, 223, 224, 224, 225, 226, 226, 227, 227, 228, 228, 229,
	229, 230, 230, 231, 231, 231, 232, 232, 233, 233, 234, 234, 235,
	235, 235, 236, 236, 237, 237, 237, 238, 238, 239, 239, 239, 240,
	240, 240, 241, 241, 241, 242, 242, 242, 243, 243, 243, 244, 244,
	244, 245, 245, 245, 245, 246, 246, 246, 247, 247, 247, 247, 248,
	248, 248, 248, 249, 249, 249, 249, 249, 250, 250, 250, 250, 250,
	251, 251, 251, 251, 251, 252, 252, 252, 252, 252, 252, 252, 253,
	253, 253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254,
	254, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255
};

// [Immorpher] - table for maximum light curve that is a quarter of a circle
unsigned char lightmax[256] = {
	0,   23,  32,  39,  45,	 50,  55,  59,	63,  67,  71,  74,  77,
	80,  83,  86,  89,  92,	 94,  97,  99,	101, 104, 106, 108, 110,
	112, 114, 116, 118, 120, 122, 124, 125, 127, 129, 131, 132, 134,
	136, 137, 139, 140, 142, 143, 145, 146, 148, 149, 150, 152, 153,
	154, 156, 157, 158, 159, 161, 162, 163, 164, 165, 167, 168, 169,
	170, 171, 172, 173, 174, 175, 177, 178, 179, 180, 181, 182, 183,
	184, 185, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 194,
	195, 196, 197, 198, 199, 199, 200, 201, 202, 202, 203, 204, 205,
	205, 206, 207, 208, 208, 209, 210, 210, 211, 212, 212, 213, 214,
	214, 215, 216, 216, 217, 218, 218, 219, 219, 220, 221, 221, 222,
	222, 223, 223, 224, 224, 225, 226, 226, 227, 227, 228, 228, 229,
	229, 230, 230, 231, 231, 231, 232, 232, 233, 233, 234, 234, 235,
	235, 235, 236, 236, 237, 237, 237, 238, 238, 239, 239, 239, 240,
	240, 240, 241, 241, 241, 242, 242, 242, 243, 243, 243, 244, 244,
	244, 245, 245, 245, 245, 246, 246, 246, 247, 247, 247, 247, 248,
	248, 248, 248, 249, 249, 249, 249, 249, 250, 250, 250, 250, 250,
	251, 251, 251, 251, 251, 252, 252, 252, 252, 252, 252, 252, 253,
	253, 253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254,
	254, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255
};

buttons_t *BT_DATA[MAXPLAYERS];

int rndindex = 0;
int prndindex = 0;
int irndindex = 0; // [Immorpher] New random index

int gametic;
int gamevbls;
int lastticon;
int ticsinframe;
int ticon;

int oldticbuttons[MAXPLAYERS];
int ticbuttons[MAXPLAYERS];
int vblsinframe[MAXPLAYERS];

float f_gametic;
float f_gamevbls;
// new field for tic-based interpolation
float f_lastgametic;
float f_lastticon;
float f_ticsinframe;
float f_ticon;
float f_vblsinframe[MAXPLAYERS];
float last_fps = 0.0;

pvr_dr_state_t dr_state;

unsigned vbls_index = 0;
uint8_t wrapped = 0;
uint64_t framecount = 0;

//
// EXTERNS
//

extern atomic_int rdpmsg;
extern bool run_hectic_demo;
extern int early_error;

//
// D_DoomMain
//

void D_DoomMain(void)
{
	int exit;
	I_Init();
	Z_Init();
	W_Init();
	R_Init();
	ST_Init();
	S_Init();

	early_error = 0;

	gamevbls = 0;
	gametic = 0;
	ticsinframe = 0;
	ticon = 0;
	ticbuttons[0] = 0;
	oldticbuttons[0] = 0;

	D_SplashScreen();

	// give users a chance to delete old settings file first
	M_ResetSettings(&menu_settings);
	// refresh brightness after setting
	P_RefreshBrightness();

	while (true) {
		exit = D_TitleMap();

		if (exit != ga_exit) {
			exit = D_RunDemo("DEMO1LMP", sk_medium, 3);
			if (exit != ga_exit) {
				exit = D_RunDemo("DEMO2LMP", sk_medium, 9);
				if (exit != ga_exit) {
					exit = D_RunDemo("DEMO3LMP", sk_medium, 17);
					if (exit != ga_exit) {
						if (run_hectic_demo) {
							run_hectic_demo = false;
							exit = D_RunDemo("DEMO4LMP", sk_medium, 32);
						}

						if (exit != ga_exit) {
							exit = D_Credits();

							if (exit != ga_exit) {
								continue;
							}
						}
					}
				}
			}
		}

		do {
			exit = M_RunTitle();
		} while (exit != ga_timeout);
	}
}

//
// P_Random
//

int P_Random(void)
{
	prndindex = (prndindex + 1) & 0xff;
	return rndtable[prndindex];
}

//
// M_Random
//

int M_Random(void)
{
	rndindex = (rndindex + 1) & 0xff;
	return rndtable[rndindex];
}

//
// I_Random  
// [Immorpher] new randomizer
//

int I_Random(void)
{
	irndindex = (irndindex + 1) & 0xff;
	// [Immorpher] travels opposite direction!
	return rndtable[255 - irndindex];
}

//
// M_ClearRandom
//

void M_ClearRandom(void)
{
	// [Immorpher] new random index doesn't get reset
	rndindex = prndindex = 0;
}

//
// MiniLoop
// #pragma attempt to not inline everything it calls
// RANGECHECK-guarded calls are checking heap consistency
// they found a bug in KOS, they stay for the future :-)
//

//#pragma GCC push_options
//#pragma GCC optimize ("-O1")
int MiniLoop(void (*start)(void), void (*stop)(int), int (*ticker)(void), void (*drawer)(void))
{
	// high resolution frame timing
	uint64_t dstart = 0;
	uint64_t dend = 0;
	int exit;
	int buttons;

	gameaction = ga_nothing;
	gamevbls = 0;
	gametic = 0;
	ticon = 0;
	ticsinframe = 0;
	vbls_index = 0;
	f_gamevbls = 0;
	f_gametic = 0;
	f_ticon = 0;
	f_ticsinframe = 0;

	// setup (cache graphics, etc)
	if (start)
		start();

	drawsync1 = 0;
	drawsync2 = vsync;

	uint32_t last_delta;

	while (true) {
#if RANGECHECK
		Z_CheckZone(mainzone);
#endif
		// used to disable and restore interpolation setting when paused
		int interp = menu_settings.Interpolate;

		last_delta = (uint32_t)((uint64_t)(dend - dstart));

		dstart = perf_cntr_timer_ns();

		float last_vbls = (float)last_delta / (float)NS_PER_VBL;

		if (gamepaused || vbls_index == 0) {
			last_vbls = (global_render_state.fps_uncap) ? 1 : 2;
			vbls_index = 1;
		}

		if (last_vbls < 1)
			last_vbls = 1;

		if (demoplayback || !global_render_state.fps_uncap) {
			vblsinframe[0] = drawsync1;
			f_vblsinframe[0] = vblsinframe[0];
		} else {
			f_vblsinframe[0] = last_vbls;
		}

		// this was in the wrong place before
		// now it is only one frame behind instead of two
		// this fixes some lag
		if (f_vblsinframe[0] <= 1.0f)
			last_fps = 60.0f;
		else
			last_fps = 60.0f / f_vblsinframe[0];

		// get buttons for next tic
		oldticbuttons[0] = ticbuttons[0];
		buttons = I_GetControllerData();

#if RANGECHECK
		Z_CheckZone(mainzone);
#endif

		ticbuttons[0] = buttons;

		// Read|Write demos
		if (demoplayback) {
			if (buttons & (ALL_JPAD | ALL_BUTTONS)) {
				exit = ga_exit;
				break;
			}

			buttons = *demobuffer++;
			ticbuttons[0] = buttons;

			if ((buttons & PAD_START) || (((uintptr_t)demobuffer - (uintptr_t)demo_p) >= 16000)) {
				exit = ga_exitdemo;
				break;
			}
		}

		// the following if/else blocks compute the same state in different ways
		// the if block maintains original 30 fps update (int update, copy to float)
		// the else is for uncapped update rate (float update, copy to int)
		if (demoplayback || !global_render_state.fps_uncap) {
			ticon += vblsinframe[0];
			if (ticsinframe < (ticon >> 1)) {
				gametic += 1;
				ticsinframe = (ticon >> 1);
			}

			f_ticon = ticon;
			f_ticsinframe = ticsinframe;
			f_gametic = gametic;
		} else {
			f_ticon += f_vblsinframe[0];
			if (f_ticsinframe < (f_ticon * 0.5f)) {
				f_gametic += f_vblsinframe[0] * 0.5f;
				f_ticsinframe = (f_ticon * 0.5f);
			}

			ticon = (int)f_ticon;
			ticsinframe = (int)f_ticsinframe;
			gametic = (int)f_gametic;
		}

		if (gamepaused || ((int)f_gamevbls < (int)f_gametic)) {
			f_lastgametic = f_gametic;
		}

		if (disabledrawing == false) {
			// no interpolation on first frame (zero delta) or in demo
			if (demoplayback || (!gamepaused && last_delta == 0))
				menu_settings.Interpolate = 0;

			exit = ticker();
#if RANGECHECK
			Z_CheckZone(mainzone);
#endif

			if (demoplayback || (!gamepaused && last_delta == 0))
				menu_settings.Interpolate = interp;

			if (exit != ga_nothing)
				break;

			pvr_wait_ready();
			pvr_scene_begin();
			pvr_list_begin(PVR_LIST_OP_POLY);
			pvr_dr_init(&dr_state);
#if RANGECHECK
			Z_CheckZone(mainzone);
#endif

			if (demoplayback || (!gamepaused && last_delta == 0))
				menu_settings.Interpolate = 0;

			drawer();
#if RANGECHECK
			Z_CheckZone(mainzone);
#endif

			if (demoplayback || (!gamepaused && last_delta == 0))
				menu_settings.Interpolate = interp;
#if RANGECHECK
			Z_CheckZone(mainzone);
#endif

			pvr_list_finish();
#if RANGECHECK
			Z_CheckZone(mainzone);
#endif

			pvr_scene_finish();
#if RANGECHECK
			// this is the check that found a KOS bug
			Z_CheckZone(mainzone);
#endif

			// see `I_SystemTicker` in `i_main.c`
			rdpmsg = 1;
		}

		gamevbls = gametic;
		f_gamevbls = f_gametic;

		framecount += 1;

		dend = perf_cntr_timer_ns();
	}

	if (stop) {
		stop(exit);
	}

	oldticbuttons[0] = ticbuttons[0];

	return exit;
}
//#pragma GCC pop_options
