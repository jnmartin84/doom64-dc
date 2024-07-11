/* D_main.c  */
#include "i_main.h"
#include "doomdef.h"
#include "p_spec.h"
#include "r_local.h"

#include <kos.h>

int gamevbls;
int gametic;
int ticsinframe;
int ticon;
int lastticon;
int vblsinframe[MAXPLAYERS];
int ticbuttons[MAXPLAYERS];
int oldticbuttons[MAXPLAYERS];

buttons_t   *BT_DATA[MAXPLAYERS];

extern boolean run_hectic_demo;

void D_DoomMain(void)
{
    int exit;
    I_Init();
    Z_Init();
    W_Init();
    R_Init();
    ST_Init();
    S_Init();

    gamevbls = 0;
    gametic = 0;
    ticsinframe = 0;
    ticon = 0;
    ticbuttons[0] = 0;
    oldticbuttons[0] = 0;

    D_SplashScreen();

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
        } while(exit != ga_timeout);
    }
}

// M_Random
// Returns a 0-255 number
const unsigned char rndtable[256] = 
{
	0, 8, 109, 220, 222, 241, 149, 107, 75, 248, 254, 140, 16, 66,
	74, 21, 211, 47, 80, 242, 154, 27, 205, 128, 161, 89, 77, 36,
	95, 110, 85, 48, 212, 140, 211, 249, 22, 79, 200, 50, 28, 188,
	52, 140, 202, 120, 68, 145, 62, 70, 184, 190, 91, 197, 152, 224,
	149, 104, 25, 178, 252, 182, 202, 182, 141, 197, 4, 81, 181, 242,
	145, 42, 39, 227, 156, 198, 225, 193, 219, 93, 122, 175, 249, 0,
	175, 143, 70, 239, 46, 246, 163, 53, 163, 109, 168, 135, 2, 235,
	25, 92, 20, 145, 138, 77, 69, 166, 78, 176, 173, 212, 166, 113,
	94, 161, 41, 50, 239, 49, 111, 164, 70, 60, 2, 37, 171, 75,
	136, 156, 11, 56, 42, 146, 138, 229, 73, 146, 77, 61, 98, 196,
	135, 106, 63, 197, 195, 86, 96, 203, 113, 101, 170, 247, 181, 113,
	80, 250, 108, 7, 255, 237, 129, 226, 79, 107, 112, 166, 103, 241,
	24, 223, 239, 120, 198, 58, 60, 82, 128, 3, 184, 66, 143, 224,
	145, 224, 81, 206, 163, 45, 63, 90, 168, 114, 59, 33, 159, 95,
	28, 139, 123, 98, 125, 196, 15, 70, 194, 253, 54, 14, 109, 226,
	71, 17, 161, 93, 186, 87, 244, 138, 20, 52, 123, 251, 26, 36,
	17, 46, 52, 231, 232, 76, 31, 221, 84, 37, 216, 165, 212, 106,
	197, 242, 98, 43, 39, 175, 254, 145, 190, 84, 118, 222, 187, 136,
	120, 163, 236, 249
};

int rndindex = 0;
int prndindex = 0;
int irndindex = 0; // [Immorpher] New random index

int P_Random(void)
{
	prndindex = (prndindex + 1) & 0xff;
	return rndtable[prndindex];
}

int M_Random(void)
{
	rndindex = (rndindex + 1) & 0xff;
	return rndtable[rndindex];
}

int I_Random(void) // [Immorpher] new randomizer
{
	irndindex = (irndindex + 1) & 0xff;
	return rndtable[255-irndindex]; // [Immorpher] travels opposite direction!
}

void M_ClearRandom(void)
{
	rndindex = prndindex = 0; // [Immorpher] new random index doesn't get reset
}

uint64_t framecount = 0;
extern uint8_t __attribute__((aligned(32))) op_buf[VERTBUF_SIZE];
extern uint8_t __attribute__((aligned(32))) tr_buf[VERTBUF_SIZE];

extern int last_Ltrig;
extern int last_Rtrig;

static uint64_t start_time, end_time;

int MiniLoop(void(*start)(void), void(*stop)(),
             int(*ticker)(void), void(*drawer)(void))
{
	int exit;
	int buttons;

	gameaction = ga_nothing;
	gamevbls = 0;
	gametic = 0;
	ticon = 0;
	ticsinframe = 0;

	// setup (cache graphics, etc)
	if (start)
		start();

	drawsync1 = 0;
	drawsync2 = vsync;

	while (true) {
		start_time = timer_us_gettime64();
		vblsinframe[0] = drawsync1;

		// get buttons for next tic
		oldticbuttons[0] = ticbuttons[0];

		buttons = I_GetControllerData();
		ticbuttons[0] = buttons;

		// Read|Write demos
		if (demoplayback) {
			//last_Ltrig = 255;
			//last_Rtrig = 255;

			if (buttons & (ALL_JPAD|ALL_BUTTONS)) {
				exit = ga_exit;
				break;
			}

			buttons = *demobuffer++;

			if ((buttons & PAD_START) || (((uintptr_t)demobuffer - (uintptr_t)demo_p) >= 16000)) {
				exit = ga_exitdemo;
				break;
			}

			ticbuttons[0] = buttons;
		}

		ticon += vblsinframe[0];
		if (ticsinframe < (ticon >> 1)) {
			gametic += 1;
			ticsinframe = (ticon >> 1);
		}

		S_UpdateSounds();

		if (disabledrawing == false) {
			exit = ticker();
			if (exit != ga_nothing)
				break;

			vid_waitvbl();
			pvr_scene_begin();
			pvr_set_vertbuf(PVR_LIST_OP_POLY, op_buf, VERTBUF_SIZE);
			pvr_set_vertbuf(PVR_LIST_TR_POLY, tr_buf, VERTBUF_SIZE);

			drawer();

			pvr_scene_finish();
			pvr_wait_ready();
		}

		gamevbls = gametic;

		// best effort 30fps cap when frame time is less than 1/30th of a second
		end_time = timer_us_gettime64();
		if((end_time - start_time) < 33333)
			usleep(33333 - (end_time - start_time));

		framecount += 1;
	}

	I_GetScreenGrab();

	if (stop)
		stop(exit);

	oldticbuttons[0] = ticbuttons[0];

	return exit;
}
