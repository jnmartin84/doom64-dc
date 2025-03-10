#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

bool gamepaused = true; // 800A6270

/*
===============================================================================

								THINKERS

All thinkers should be allocated by Z_Malloc so they can be operated on uniformly.  The actual
structures will vary in size, but the first element must be thinker_t.

Mobjs are similar to thinkers, but kept seperate for more optimal list
processing
===============================================================================
*/

thinker_t thinkercap;
/* both the head and tail of the thinker list */ //80096378
mobj_t mobjhead; /* head and tail of mobj list */ //800A8C74,

/*
===============
=
= P_AddThinker
=
= Adds a new thinker at the end of the list
=
===============
*/

void P_AddThinker(thinker_t *thinker)
{
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
}

/*
===============
=
= P_RemoveThinker
=
= Deallocation is lazy -- it will not actually be freed until its
= thinking turn comes up
=
===============
*/

void P_RemoveThinker(thinker_t *thinker)
{
	thinker->function = (think_t)-1;

	if (thinker == macrothinker) {
		// [D64] New lines
		macrothinker = NULL;
	}
}

/*
===============
=
= P_RunThinkers
=
===============
*/

void P_RunThinkers(void)
{
	// fix use-after-free
	// https://github.com/Immorpher/doom64ultra/commit/dae9e1e50c505089c3194c71a5a502a3ba67a2e3
	thinker_t *currentthinker, *next;

	currentthinker = thinkercap.next;
	if (thinkercap.next != &thinkercap) {
		while (currentthinker != &thinkercap) {
			if (currentthinker->function == (think_t)-1) {
				// time to remove it
				next = currentthinker->next;
				currentthinker->next->prev = currentthinker->prev;
				currentthinker->prev->next = currentthinker->next;
				Z_Free(currentthinker);
			} else {
				if (currentthinker->function) {
#if RANGECHECK
					if (!arch_valid_text_address((uintptr_t)currentthinker->function))
						I_Error("tried to call invalid think_t %08x", (uintptr_t)currentthinker->function);
#endif
					((void (*)(void *))currentthinker->function)(currentthinker);
				}
				// get next pointer after in case the thinker added another one
				next = currentthinker->next;
			}
			currentthinker = next;
#if RANGECHECK
			if (!arch_valid_address((uintptr_t)currentthinker))
				I_Error("bad current thinker %08x", (uintptr_t)currentthinker);
#endif
		}
	}
}

/*
==============
=
= P_CheckCheats
=
==============
*/
#define NUM_MENU_GAME 5
void P_CheckCheats(void)
{
	unsigned int buttons;
	int exit;

	buttons = ticbuttons[0] & 0xffff0000;

	if (!gamepaused) {
		if ((buttons & PAD_START) && !(oldticbuttons[0] & PAD_START)) {
			gamepaused = true;
			in_menu = 1;

			S_PauseSound();
			Z_Defragment(mainzone);
			lastticon = ticon;

			MenuCall = M_MenuTitleDrawer;
			MenuItem = Menu_Game;
			cursorpos = 0;

			if (FeaturesUnlocked == false)
				itemlines = NUM_MENU_GAME - 1;
			else
				itemlines = NUM_MENU_GAME; // Enable cheat menu

			MenuIdx = 0;
			text_alpha = 255;
			MenuAnimationTic = 0;
		}

		return;
	}

	exit = M_MenuTicker();

	if (exit)
		M_MenuClearCall(ga_nothing);

	if (exit == ga_warped) {
		gameaction = ga_warped;
	} else if (exit == ga_exitdemo) {
		gameaction = ga_exitdemo;
	} else if (exit == ga_restart) {
		gameaction = ga_restart;
	} else if (exit == ga_exit) {
		in_menu = 0;
		gamepaused = false;
		S_ResumeSound();
		ticon = lastticon;
		ticsinframe = lastticon >> 2;
	}
}

void G_DoReborn(int playernum); //extern

/*
=================
=
= P_RecordOldPositions
=
=================
*/
static void P_RecordOldPositions(void)
{
	int i;
	mobj_t *mo;

	for (mo = mobjhead.next; mo != &mobjhead; mo = mo->next) {
		mo->old_x = mo->x;
		mo->old_y = mo->y;
		mo->old_z = mo->z;
	}

	// [Striker] Probably slower than I'd like it to be, but will do for now
	// until I add an "interp queue", similar to how thinkers are set up.
	for (i = 0; i < numsectors; i++) {
		sectors[i].old_floorheight = sectors[i].floorheight;
		sectors[i].old_ceilingheight = sectors[i].ceilingheight;
	}
}

/*
=================
=
= P_Ticker
=
=================
*/

//extern functions
void P_CheckSights(void);
void P_RunMobjBase(void);

// new logic for stopping and restarting plasma buzz when pause state changes
static int restore_plc = 0;
static int pause_changed = 0;
static int last_paused = 0;

static int pause_start_time = 0;
static int pause_end_time = 0;

extern int time_paused;

int P_Ticker(void) //80021A00
{
	player_t *pl;

	gameaction = ga_nothing;

	// new logic for stopping and restarting plasma buzz when pause state changes
	if (gamepaused) {
		if (last_paused == 0) {
			last_paused = 1;
			pause_changed = 1;
			pause_start_time = rtc_unix_secs();
		} else if (last_paused == 1) {
			last_paused = 1;
			pause_changed = 0;
		}

		if (pause_changed) {
			if (plasma_loop_channel != -1) {
				restore_plc = 1;
				P_StopElectricLoop();
			}
		}
	} else {
		if (last_paused == 0) {
			last_paused = 0;
			pause_changed = 0;
		} else if (last_paused == 1) {
			last_paused = 0;
			pause_changed = 1;
			pause_end_time = rtc_unix_secs();
			time_paused += (unsigned)(pause_end_time - pause_start_time);
		}

		if (restore_plc) {
			if (plasma_loop_channel == -1) {
				P_StartElectricLoop();
			}

			restore_plc = 0;
		}
	}

	//
	// check for pause and cheats
	//
	P_CheckCheats();

//	if ((!gamepaused) && (gamevbls < gametic)) {
	if ((!gamepaused) && ((int)f_gamevbls < (int)f_gametic)) {
//		if (menu_settings.Interpolate)
		P_RecordOldPositions();

		P_RunThinkers();
		P_CheckSights();
		P_RunMobjBase();

		P_UpdateSpecials();
		P_RunMacros();

		ST_Ticker(); // update status bar
	}

	//
	// run player actions
	//
	pl = players;

	if (pl->playerstate == PST_REBORN)
		gameaction = ga_died;

	AM_Control(pl);
	P_PlayerThink(pl);


	return gameaction; // may have been set to ga_died, ga_completed, or ga_secretexit
}

/*
=============
=
= P_Drawer
=
= draw current display
=============
*/

extern Matrix  R_ProjectionMatrix;
extern Matrix  R_ModelMatrix;

void P_Drawer(void) // 80021AC8
{
	I_ClearFrame();

	mat_load(&R_ProjectionMatrix);
	mat_apply(&R_ModelMatrix);

	if (players[0].automapflags & (AF_LINES | AF_SUBSEC)) {
		AM_Drawer();
	} else {
		R_RenderPlayerView();

		if (demoplayback == false)
			ST_Drawer();
	}

	if (MenuCall) {
		M_DrawOverlay();
		MenuCall();
	}

	I_DrawFrame();
}

extern void T_FadeInBrightness(fadebright_t *fb);
extern int start_time;
extern int end_time;

void P_Start(void)
{
	fadebright_t *fb;

	DrawerStatus = 1;

	if (gamemap == 33) { /* Add by default God Mode in player  */
		players[0].cheats |= CF_GODMODE;
	} else if (gamemap == 32) {
		/* Remove by default God Mode in player  */
		players[0].cheats &= ~CF_GODMODE;
	}
	gamepaused = false;
	validcount = 1;

	AM_Start();
	M_ClearRandom();

	/* do a nice little fade in effect */
	fb = Z_Malloc(sizeof(*fb), PU_LEVSPEC, 0);
	P_AddThinker(&fb->thinker);
	fb->thinker.function = T_FadeInBrightness;
	fb->factor = 0;

	/* autoactivate line specials */
#if RANGECHECK
	P_ActivateLineByTag(999, players[0].mo, 0);
#else
	P_ActivateLineByTag(999, players[0].mo);
#endif

	// overflows in 2086, oh well
	start_time = rtc_unix_secs();
	time_paused = 0;

	MusicID = MapInfo[gamemap].MusicSeq - 92;
	S_StartMusic(MapInfo[gamemap].MusicSeq);

	S_SetSoundVolume(menu_settings.SfxVolume);
	S_SetMusicVolume(menu_settings.MusVolume);

	restore_plc = 0;
	pause_changed = 0;
	last_paused = 0;
	in_menu = 0;
}

void P_Stop(int exit) // 80021D58
{
	in_menu = 1;
	/* [d64] stop plasma buzz */
	//	S_StopSound(0, sfx_electric);
	if (plasma_loop_channel != -1)
		P_StopElectricLoop();

	// overflows in 2086, oh well
	end_time = rtc_unix_secs();
	gamepaused = false;
	DrawerStatus = 0;

	G_PlayerFinishLevel(0);

	/* free all tags except the PU_STATIC tag */
	Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

	if ((gamemap != 33) || (exit == 8))
		S_StopMusic();

	S_ResetSound();

	if ((demoplayback) && (exit == 8))
		I_WIPE_FadeOutScreen();
	else
		I_WIPE_MeltScreen();

	S_StopAll();

	restore_plc = 0;
	pause_changed = 0;
	last_paused = 0;
}
