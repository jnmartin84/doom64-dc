#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

boolean gamepaused = true; // 800A6270

/*
===============================================================================

								THINKERS

All thinkers should be allocated by Z_Malloc so they can be operated on uniformly.  The actual
structures will vary in size, but the first element must be thinker_t.

Mobjs are similar to thinkers, but kept seperate for more optimal list
processing
===============================================================================
*/

thinker_t	thinkercap;	/* both the head and tail of the thinker list */    //80096378
mobj_t		mobjhead;	/* head and tail of mobj list */                    //800A8C74,

/*
===============
=
= P_AddThinker
=
= Adds a new thinker at the end of the list
=
===============
*/

void P_AddThinker (thinker_t *thinker)
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

void P_RemoveThinker (thinker_t *thinker)
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

void P_RunThinkers (void)
{
	thinker_t	*currentthinker;

	currentthinker = thinkercap.next;
	if (thinkercap.next != &thinkercap) {
		while (currentthinker != &thinkercap) {
			if (currentthinker->function == (think_t)-1) {
				// time to remove it
				currentthinker->next->prev = currentthinker->prev;
				currentthinker->prev->next = currentthinker->next;
				Z_Free (currentthinker);
			} else {
				if (currentthinker->function) {
					currentthinker->function (currentthinker);
				}
			}
			currentthinker = currentthinker->next;
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
void P_CheckCheats (void)
{
	unsigned int buttons;
	int exit;

	buttons = ticbuttons[0] & 0xffff0000;

	if (!gamepaused) {
		if ((buttons & PAD_START) && !(oldticbuttons[0] & PAD_START)) {
			gamepaused = true;

			S_PauseSound();

			lastticon = ticon;

			MenuCall = M_MenuTitleDrawer;
			MenuItem = Menu_Game;
			cursorpos = 0;

			if (FeaturesUnlocked == false)
				itemlines = NUM_MENU_GAME - 1;
			else
				itemlines = NUM_MENU_GAME;  // Enable cheat menu

			MenuIdx = 0;
			text_alpha = 255;
			MenuAnimationTic = 0;
		}

		return;
	}

	exit = M_MenuTicker();

	if (exit)
		M_MenuClearCall();

	if (exit == ga_warped) {
		gameaction = ga_warped;
	} else if (exit == ga_exitdemo) {
		gameaction = ga_exitdemo;
	} else if (exit == ga_restart) {
		gameaction = ga_restart;
	} else if (exit == ga_exit) {
		gamepaused = false;
		S_ResumeSound();
		ticon = lastticon;
		ticsinframe = lastticon >> 2;
	}
}

void G_DoReborn (int playernum);//extern

/*
=================
=
= P_Ticker
=
=================
*/

//extern functions
void P_CheckSights (void);
void P_RunMobjBase (void);

int P_Ticker (void)//80021A00
{
	player_t *pl;

	gameaction = ga_nothing;

	//
	// check for pause and cheats
	//
	P_CheckCheats();

	if ((!gamepaused) && (gamevbls < gametic))
	{
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

extern Matrix __attribute__((aligned(32))) R_ProjectionMatrix;
extern Matrix __attribute__((aligned(32))) R_ModelMatrix;

void P_Drawer (void) // 80021AC8
{
	I_ClearFrame();

	mat_load(&R_ProjectionMatrix);
	mat_apply(&R_ModelMatrix);

	if (players[0].automapflags & (AF_LINES|AF_SUBSEC)) {
		AM_Drawer();
	} else {
		R_RenderPlayerView();

		if (demoplayback == false)
			ST_Drawer();
	}

	if (MenuCall) {
		M_DrawOverlay(0, 0, 320, 240, 96);
		MenuCall();
	}

	I_DrawFrame();
}

extern void T_FadeInBrightness(fadebright_t *fb);
extern int start_time;  // 80063390
extern int end_time;    // 80063394

void P_Start (void) // 80021C50
{
	fadebright_t *fb;

	DrawerStatus = 1;

	if (gamemap == 33) {  /* Add by default God Mode in player  */
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
	P_ActivateLineByTag(999, players[0].mo);

	start_time = ticon;

	MusicID = MapInfo[gamemap].MusicSeq-92;
	S_StartMusic(MapInfo[gamemap].MusicSeq);
}

extern int plasma_channel;
extern int plasma_loop_channel;

extern int lump_frame[(575+310)];
extern int used_lumps[(575+310)];
extern int used_lump_idx;
extern int del_idx;
extern int donebefore;
extern pvr_ptr_t pvr_troo[MAX_CACHED_SPRITES];

void P_Stop (int exit) // 80021D58
{
	/* [d64] stop plasma buzz */
//	S_StopSound(0, sfx_electric);
	if (plasma_channel != -1)
		snd_sfx_stop(plasma_channel);
	if (plasma_loop_channel != -1)
		snd_sfx_stop(plasma_loop_channel);
	plasma_channel = -1;
	plasma_loop_channel = -1;


	end_time = ticon;
	gamepaused = false;
	DrawerStatus = 0;

	G_PlayerFinishLevel(0);

	/* free all tags except the PU_STATIC tag */
	Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

	if ((gamemap != 33) || (exit == 8))
		S_StopMusic();

	S_ResetSound();

	if (donebefore) {
		for (int i=0;i<(575+310);i++) {
			if (used_lumps[i] != -1) {
				pvr_mem_free(pvr_troo[used_lumps[i]]);
			}
		}
	}
	memset(used_lumps, 0xff, sizeof(int)*(575+310));
	memset(lump_frame, 0xff, sizeof(int)*(575+310));
	used_lump_idx = 0;
	del_idx = 0;

	if ((demoplayback) && (exit == 8))
		I_WIPE_FadeOutScreen();
	else
		I_WIPE_MeltScreen();

	S_StopAll();
}


