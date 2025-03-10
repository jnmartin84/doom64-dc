/* G_game.c  */

#include "doomdef.h"
#include "p_local.h"

void G_PlayerReborn(int player);

void G_DoReborn(int playernum);

void G_DoLoadLevel(void);

gameaction_t gameaction;
skill_t gameskill;
int gamemap;
int nextmap; /* the map to go to after the stats */

player_t players[MAXPLAYERS];

int consoleplayer; /* player taking events and displaying  */
int displayplayer; /* view being displayed  */
int totalkills, totalitems, totalsecret; /* for intermission  */

bool demoplayback;
int *demo_p = NULL, *demobuffer = NULL;

mapthing_t playerstarts[MAXPLAYERS];

/*
==============
=
= G_DoLoadLevel
=
==============
*/

void G_DoLoadLevel(void)
{
	if (((gameaction == 7) || (gameaction == 4)) ||
		(players[0].playerstate == PST_DEAD)) {
		players[0].playerstate = PST_REBORN;
	}

	P_SetupLevel(gamemap, gameskill);
	gameaction = ga_nothing;
}

/*
==============================================================================

						PLAYER STRUCTURE FUNCTIONS

also see P_SpawnPlayer in P_Mobj
==============================================================================
*/

/*
====================
=
= G_PlayerFinishLevel
=
= Can when a player completes a level
====================
*/

void G_PlayerFinishLevel(int player) // 80004598
{
	player_t *p;

	p = &players[player];

	memset(p->f_powers, 0, sizeof(p->f_powers));
	memset(p->powers, 0, sizeof(p->powers));
	memset(p->cards, 0, sizeof(p->cards));
	p->mo->flags &= ~MF_SHADOW; /* cancel invisibility  */
	p->extralight = 0; /* cancel gun flashes  */
	p->f_damagecount = 0;
	p->damagecount = 0; /* no palette changes  */
	p->f_bonuscount = 0;
	p->bonuscount = 0;
	p->f_bfgcount = 0;
	p->bfgcount = 0;
	p->automapflags = 0;
	p->messagetic = 0;
	p->messagetic1 = 0; // [Immorpher] Clear messages
	p->messagetic2 = 0; // [Immorpher] Clear messages
	p->messagetic3 = 0; // [Immorpher] Clear messages
}

/*
====================
=
= G_PlayerReborn
=
= Called after a player dies
= almost everything is cleared and initialized
====================
*/

int gobalcheats = 0; // [GEC]

void G_PlayerReborn(int player) // 80004630
{
	player_t *p;

	p = &players[player];
	memset(p, 0, sizeof(*p));

	p->usedown = p->attackdown = true; // don't do anything immediately
	p->playerstate = PST_LIVE;
	p->health = MAXHEALTH;
	p->readyweapon = p->pendingweapon = wp_pistol;
	p->weaponowned[wp_fist] = true;
	p->weaponowned[wp_pistol] = true;
	p->ammo[am_clip] = 50;
	p->maxammo[am_clip] = maxammo[am_clip];
	p->maxammo[am_shell] = maxammo[am_shell];
	p->maxammo[am_cell] = maxammo[am_cell];
	p->maxammo[am_misl] = maxammo[am_misl];

	p->cheats |= gobalcheats; // [GEC] Apply global cheat codes
}

/*============================================================================  */

/*
====================
=
= G_CompleteLevel
=
====================
*/

void G_CompleteLevel(void) // 800046E4
{
	gameaction = ga_completed;
}

/*
====================
=
= G_InitNew
=
====================
*/

extern int ActualConfiguration[13];
mobj_t emptymobj;

void G_InitNew(skill_t skill, int map, gametype_t gametype)
{
	(void)gametype;
	/* free all tags except the PU_STATIC tag */
	Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

	M_ClearRandom();

	/* force players to be initialized upon first level load */
	players[0].playerstate = PST_REBORN;

	/* these may be reset by I_NetSetup */
	gameskill = skill;
	gamemap = map;

	memset(&emptymobj, 0, sizeof(emptymobj));
	players[0].mo = &emptymobj; /* for net consistancy checks */

	demoplayback = false;

	BT_DATA[0] = (buttons_t *)ActualConfiguration;

	G_InitSkill(skill);
}

static int ever_nightmared = 0;
void G_InitSkill(skill_t skill) // [Immorpher] initialize skill
{
	if (skill >= sk_nightmare) {
		ever_nightmared = 1;
		// Faster enemies
		states[S_054].tics = 4; // S_SARG_ATK1
		states[S_055].tics = 4; // S_SARG_ATK2
		states[S_056].tics = 4; // S_SARG_ATK3
		mobjinfo[MT_DEMON1].speed = 24; // MT_DEMON1
		mobjinfo[MT_DEMON2].speed = 24; // MT_DEMON2
		mobjinfo[MT_MANCUBUS].speed = 14;
		mobjinfo[MT_POSSESSED1].speed = 14;
		mobjinfo[MT_POSSESSED2].speed = 14;
		mobjinfo[MT_IMP1].speed = 14;
		mobjinfo[MT_IMP2].speed = 20;
		mobjinfo[MT_CACODEMON].speed = 14;
		mobjinfo[MT_BRUISER1].speed = 14;
		mobjinfo[MT_BRUISER2].speed = 14;
		mobjinfo[MT_SKULL].speed = 14;
		mobjinfo[MT_BABY].speed = 18;
		mobjinfo[MT_CYBORG].speed = 20;
		mobjinfo[MT_PAIN].speed = 12;
		mobjinfo[MT_RESURRECTOR].speed = 32;

		// Faster Projectiles
		mobjinfo[MT_PROJ_BRUISER1].speed = 20; // MT_BRUISERSHOT
		mobjinfo[MT_PROJ_BRUISER2].speed = 20; // MT_BRUISERSHOT2
		mobjinfo[MT_PROJ_HEAD].speed = 30; // MT_HEADSHOT value like Doom 64 Ex
		mobjinfo[MT_PROJ_IMP1].speed = 20; // MT_TROOPSHOT
		mobjinfo[MT_PROJ_IMP2].speed = 30; // [Immorpher] reduced it from 35 to 30, hard to dodge otherwise

		// [Immorpher] Thinner enemies
		mobjinfo[MT_DEMON1].radius = 38 * FRACUNIT;
		mobjinfo[MT_DEMON1].height = 81 * FRACUNIT;
		mobjinfo[MT_DEMON2].radius = 38 * FRACUNIT;
		mobjinfo[MT_DEMON2].height = 81 * FRACUNIT;
		mobjinfo[MT_MANCUBUS].radius = 54 * FRACUNIT;
		mobjinfo[MT_MANCUBUS].height = 105 * FRACUNIT;
		mobjinfo[MT_POSSESSED1].radius = 19 * FRACUNIT;
		mobjinfo[MT_POSSESSED1].height = 79 * FRACUNIT;
		mobjinfo[MT_POSSESSED2].radius = 19 * FRACUNIT;
		mobjinfo[MT_POSSESSED2].height = 79 * FRACUNIT;
		mobjinfo[MT_IMP1].radius = 27 * FRACUNIT;
		mobjinfo[MT_IMP1].height = 86 * FRACUNIT;
		mobjinfo[MT_IMP2].radius = 27 * FRACUNIT;
		mobjinfo[MT_IMP2].height = 86 * FRACUNIT;
		mobjinfo[MT_CACODEMON].radius = 40 * FRACUNIT;
		mobjinfo[MT_CACODEMON].height = 88 * FRACUNIT;
		mobjinfo[MT_BRUISER1].radius = 24 * FRACUNIT;
		mobjinfo[MT_BRUISER1].height = 98 * FRACUNIT;
		mobjinfo[MT_BRUISER2].radius = 24 * FRACUNIT;
		mobjinfo[MT_BRUISER2].height = 98 * FRACUNIT;
		mobjinfo[MT_SKULL].radius = 26 * FRACUNIT;
		mobjinfo[MT_SKULL].height = 62 * FRACUNIT;
		mobjinfo[MT_BABY].radius = 62 * FRACUNIT;
		mobjinfo[MT_BABY].height = 78 * FRACUNIT;
		mobjinfo[MT_CYBORG].radius = 59 * FRACUNIT;
		mobjinfo[MT_CYBORG].height = 168 * FRACUNIT;
		mobjinfo[MT_PAIN].radius = 55 * FRACUNIT;
		mobjinfo[MT_PAIN].height = 98 * FRACUNIT;
		mobjinfo[MT_RESURRECTOR].radius = 71 * FRACUNIT;
		mobjinfo[MT_RESURRECTOR].height = 148 * FRACUNIT;

		// Less pain
		mobjinfo[MT_DEMON1].painchance = 90;
		mobjinfo[MT_DEMON2].painchance = 90;
		mobjinfo[MT_MANCUBUS].painchance = 40;
		mobjinfo[MT_POSSESSED1].painchance = 100;
		mobjinfo[MT_POSSESSED2].painchance = 95;
		mobjinfo[MT_IMP1].painchance = 100;
		mobjinfo[MT_IMP2].painchance = 64;
		mobjinfo[MT_CACODEMON].painchance = 64;
		mobjinfo[MT_BRUISER1].painchance = 25;
		mobjinfo[MT_BRUISER2].painchance = 25;
		mobjinfo[MT_SKULL].painchance = 128;
		mobjinfo[MT_BABY].painchance = 64;
		mobjinfo[MT_CYBORG].painchance = 10;
		mobjinfo[MT_PAIN].painchance = 64;
		mobjinfo[MT_RESURRECTOR].painchance = 25;
	} else if (ever_nightmared) {
		// Restore enemy speeds
		states[S_054].tics = 8; // S_SARG_ATK1
		states[S_055].tics = 8; // S_SARG_ATK2
		states[S_056].tics = 8; // S_SARG_ATK3
		mobjinfo[MT_DEMON1].speed = 12; // MT_SERGEANT
		mobjinfo[MT_DEMON2].speed = 12; // MT_SERGEANT2
		mobjinfo[MT_MANCUBUS].speed = 8;
		mobjinfo[MT_POSSESSED1].speed = 8;
		mobjinfo[MT_POSSESSED2].speed = 8;
		mobjinfo[MT_IMP1].speed = 8;
		mobjinfo[MT_IMP2].speed = 16;
		mobjinfo[MT_CACODEMON].speed = 8;
		mobjinfo[MT_BRUISER1].speed = 8;
		mobjinfo[MT_BRUISER2].speed = 8;
		mobjinfo[MT_SKULL].speed = 8;
		mobjinfo[MT_BABY].speed = 12;
		mobjinfo[MT_CYBORG].speed = 16;
		mobjinfo[MT_PAIN].speed = 8;
		mobjinfo[MT_RESURRECTOR].speed = 30;

		// Restore projectile speeds
		mobjinfo[MT_PROJ_BRUISER1].speed = 15; // MT_BRUISERSHOT
		mobjinfo[MT_PROJ_BRUISER2].speed = 15; // MT_BRUISERSHOT2
		mobjinfo[MT_PROJ_HEAD].speed = 20; // MT_HEADSHOT
		mobjinfo[MT_PROJ_IMP1].speed = 10; // MT_TROOPSHOT
		mobjinfo[MT_PROJ_IMP2].speed = 20; // MT_TROOPSHOT2

		// Restore Enemy Size
		mobjinfo[MT_DEMON1].radius = 44 * FRACUNIT;
		mobjinfo[MT_DEMON1].height = 100 * FRACUNIT;
		mobjinfo[MT_DEMON2].radius = 50 * FRACUNIT;
		mobjinfo[MT_DEMON2].height = 100 * FRACUNIT;
		mobjinfo[MT_MANCUBUS].radius = 60 * FRACUNIT;
		mobjinfo[MT_MANCUBUS].height = 108 * FRACUNIT;
		mobjinfo[MT_POSSESSED1].radius = 32 * FRACUNIT;
		mobjinfo[MT_POSSESSED1].height = 87 * FRACUNIT;
		mobjinfo[MT_POSSESSED2].radius = 32 * FRACUNIT;
		mobjinfo[MT_POSSESSED2].height = 87 * FRACUNIT;
		mobjinfo[MT_IMP1].radius = 42 * FRACUNIT;
		mobjinfo[MT_IMP1].height = 94 * FRACUNIT;
		mobjinfo[MT_IMP2].radius = 42 * FRACUNIT;
		mobjinfo[MT_IMP2].height = 94 * FRACUNIT;
		mobjinfo[MT_CACODEMON].radius = 55 * FRACUNIT;
		mobjinfo[MT_CACODEMON].height = 90 * FRACUNIT;
		mobjinfo[MT_BRUISER1].radius = 24 * FRACUNIT;
		mobjinfo[MT_BRUISER1].height = 100 * FRACUNIT;
		mobjinfo[MT_BRUISER2].radius = 24 * FRACUNIT;
		mobjinfo[MT_BRUISER2].height = 100 * FRACUNIT;
		mobjinfo[MT_SKULL].radius = 28 * FRACUNIT;
		mobjinfo[MT_SKULL].height = 64 * FRACUNIT;
		mobjinfo[MT_BABY].radius = 64 * FRACUNIT;
		mobjinfo[MT_BABY].height = 80 * FRACUNIT;
		mobjinfo[MT_CYBORG].radius = 70 * FRACUNIT;
		mobjinfo[MT_CYBORG].height = 170 * FRACUNIT;
		mobjinfo[MT_PAIN].radius = 60 * FRACUNIT;
		mobjinfo[MT_PAIN].height = 112 * FRACUNIT;
		mobjinfo[MT_RESURRECTOR].radius = 80 * FRACUNIT;
		mobjinfo[MT_RESURRECTOR].height = 150 * FRACUNIT;

		// Restore pains
		mobjinfo[MT_DEMON1].painchance = 180;
		mobjinfo[MT_DEMON2].painchance = 180;
		mobjinfo[MT_MANCUBUS].painchance = 80;
		mobjinfo[MT_POSSESSED1].painchance = 200;
		mobjinfo[MT_POSSESSED2].painchance = 170;
		mobjinfo[MT_IMP1].painchance = 200;
		mobjinfo[MT_IMP2].painchance = 128;
		mobjinfo[MT_CACODEMON].painchance = 128;
		mobjinfo[MT_BRUISER1].painchance = 50;
		mobjinfo[MT_BRUISER2].painchance = 50;
		mobjinfo[MT_SKULL].painchance = 256;
		mobjinfo[MT_BABY].painchance = 128;
		mobjinfo[MT_CYBORG].painchance = 20;
		mobjinfo[MT_PAIN].painchance = 128;
		mobjinfo[MT_RESURRECTOR].painchance = 50;
	}
}
/*============================================================================  */

/*
=================
=
= G_RunGame
=
= The game should allready have been initialized or laoded
=================
*/
extern int extra_episodes;
void G_RunGame(void) // 80004794
{
	int last_level;

	while (1) {

		/* load a level */
		G_DoLoadLevel();

		// [Immorpher] run introduction text screen
		if (menu_settings.runintroduction && menu_settings.StoryText == true) { 
			MiniLoop(F_StartIntermission, F_StopIntermission, F_TickerIntermission, F_DrawerIntermission);
			menu_settings.runintroduction = false; // [Immorpher] only run it once!
		}

		/* run a level until death or completion */
		MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);

		if (gameaction == ga_warped)
			continue; /* skip intermission */

		if ((gameaction == ga_died) || (gameaction == ga_restart))
			continue; /* died, so restart the level */

		if (gameaction == ga_exitdemo)
			return;

		if (extra_episodes == 2 && startmap >= 41)
			last_level = 49;
		else if (extra_episodes == 2 && startmap >= 34 && startmap <= 41)
			last_level = LOST_LASTLEVEL;
		else if (extra_episodes == 1 && startmap >= 41)
			last_level = 49;
		else
			last_level = ABS_LASTLEVEL;

		// toxin refinery, secret exit to military base
		if (gamemap == 43 && nextmap == 9) {
			last_level = 50;
			nextmap = 49;
		}

		// phobos anomaly, exit to finale
		if (gamemap == 48) {
			nextmap = 49;
		}

		// military base, exit to e1m4
		if (gamemap == 49) {
			nextmap = 44;
		}

		/* run a stats intermission - [Immorpher] Removed Hectic exception */
		MiniLoop(IN_Start, IN_Stop, IN_Ticker, IN_Drawer);

		/* run the intermission if needed */
		if ((((gamemap == 8) && (nextmap == 9)) ||
			((gamemap == 4) && (nextmap == 29)) ||
			((gamemap == 12) && (nextmap == 30)) ||
			((gamemap == 18) && (nextmap == 31)) ||
			((gamemap == 1) && (nextmap == 32))) && menu_settings.StoryText == true) {

			MiniLoop(F_StartIntermission, F_StopIntermission, F_TickerIntermission, F_DrawerIntermission);

			if (gameaction == ga_warped)
				continue; /* skip intermission */

			if (gameaction == ga_restart)
				continue;

			if (gameaction == ga_exitdemo)
				return;
		} else if(FUNLEVEL(gamemap)) {
			gameaction = ga_exitdemo;
			return;
		} else {
			if (nextmap >= last_level) {
				/* run the finale if needed */
				MiniLoop(F_Start, F_Stop, F_Ticker, F_Drawer);

				if (gameaction == ga_warped)
					continue; /* skip intermission */

				if (gameaction == ga_restart)
					continue;
				else
					return;
			}
		}

		/* Set Next Level */
		gamemap = nextmap;
	}
}

int G_PlayDemoPtr(int skill, int map) // 800049D0
{
	int exit;
	int config[13];
	int sensitivity;

	demobuffer = demo_p;

	/* copy key configuration */
	memcpy(config, ActualConfiguration, sizeof(config));

	/* set new key configuration */
	memcpy(ActualConfiguration, demobuffer, sizeof(config));

	/* copy analog m_sensitivity */
	sensitivity = menu_settings.M_SENSITIVITY;

	/* set new analog m_sensitivity */
	menu_settings.M_SENSITIVITY = demobuffer[13] + 27;

	/* skip analog and key configuration */
	demobuffer += 14;

	/* play demo game */
	G_InitNew(skill, map, gt_single);
	G_DoLoadLevel();
	demoplayback = true;
	exit = MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);
	demoplayback = false;

	/* restore key configuration */
	memcpy(ActualConfiguration, config, sizeof(config));

	/* restore analog m_sensitivity */
	menu_settings.M_SENSITIVITY = sensitivity;

	/* free all tags except the PU_STATIC tag */
	Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

	return exit;
}
