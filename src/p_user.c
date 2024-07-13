/* P_user.c */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

#define MAXMOCKTIME 900 // [Immorpher] Reduced this by half for the fun!
int deathmocktics;

// Bonus mock texts from the Doom 64 community!
#define MK_TXT01t1	"MOTHER DEMON: HAHAHAHA!"
#define MK_TXT02t1	"MOTHER DEMON: YOU SHOULDN'T HAVE\nDONE THAT."
#define MK_TXT03t1	"MIDWAY: TRY AN EASIER LEVEL..."
#define MK_TXT04t1	"MIDWAY: WOW! LOOK AT THOSE DEMON\nFEET."
#define MK_TXT05t1	"MIDWAY: I PLAY DOOM AND I CAN'T GET\nUP."
#define MK_TXT06t1	"DEMON: OUCH! THAT HAD TO HURT."
#define MK_TXT07t1	"DEMON: LOOK AT ME! I'M FLAT!"
#define MK_TXT08t1	"MIDWAY: THANKS FOR PLAYING!"
#define MK_TXT09t1	"MOTHER DEMON: YOU LAZY @&$#!"
#define MK_TXT10t1	"MOTHER DEMON: HAVE YOU HAD ENOUGH?"
#define MK_TXT11t1	"MIDWAY: THE DEMONS GAVE YOU THE\nBOOT!"
#define MK_TXT12t1	"MIDWAY: THE LEAD DEMON VANQUISHED\nYOU!"
#define MK_TXT13t1	"IMMORPHER: WELCOME TO THE IQ64\nCLUB! MIGHT AS WELL JOIN THE GOOFS\nON THE DOOM 64 DISCORD SERVER!"
#define MK_TXT14t1	"IRL RANDOM HAJILE: HMMM, THAT'S ONE\nDOOMED SPACE MARINE."
#define MK_TXT15t1	"RETROTOUR: NO RUSH. JUST RESTART\nWHEN YOU'RE READY."
#define MK_TXT16t1	"SCD: GO BACK TO QUAKE, YA MORON!"
#define MK_TXT17t1	"WHINNY: I BET YOU STILL SLEEP WITH\nYOUR NIGHT LIGHT, WAAAAH!!!"
#define MK_TXT18t1	"UMLAUT: IF YOU'D LIKE TO JOIN US,\nYOU'RE ALWAYS WELCOME, DEAD OR\nALIVE. MUAHAHAHA..."
#define MK_TXT19t1	"TAUFAN99: NOW YOU KNOW THIS IS NO\nSUPERHERO SHOW."
#define MK_TXT20t1	"QUASIOTTER: EVERY TIME YOU DIE,\nA PUPPY IS BORN. THE DEMONS ARE\nJUST TRYING TO MAKE MORE PUPPIES."
#define MK_TXT21t1	"GEC: HOT OR COLD, WHICH SIDE DO YOU\nWANT TO BE ON?"
#define MK_TXT22t1	"WOLF MCBEARD: IT'S OK, I WON'T TELL\nANYONE YOU CALL ME DADDY."
#define MK_TXT23t1	"POOPSTER: NO MORE SEQUELS FOR\nYOU.... YET."
#define MK_TXT24t1	"WHYBMONOTACRAB: IF ONLY YOU COULD\nTALK TO THESE DEMONS, THEN PERHAPS\nYOU COULD TRY AND MAKE FRIENDS WITH\nTHEM, FORM ALLIANCES..."
#define MK_TXT25t1	"CAPTAIN CALEB: YOU'VE HAD OVER\nTWENTY YEARS, AND YOU STILL SUCK\nAT THIS GAME?"
#define MK_TXT26t1	"DUKE: WHAT ARE YOU WAITING FOR,\nCHRISTMAS?"
#define MK_TXT27t1	"COLLECTONIAN: UGH, YOU FORGOT\nAGAIN!? LAST TIME: ROCKETS ARE\nFOR KILLING DEMONS, NOT YOURSELF!"
#define MK_TXT28t1	"NEVANDER: THAT LOOKED LIKE IT HURT.\nWELL, WHAT ARE YOU WAITING FOR?\nTRY AGAIN AND KICK THEIR ASSES!"
#define MK_TXT29t1	"SCWIBA: YO IMMORPHER, I SAID NO\nMORE DOOM 64! WHY AM I EVEN IN\nTHIS MOD?!"
#define MK_TXT30t1	"GRAV: YOU KNOW YOU CAN BEAT THIS\nGAME IN 30 MINUTES RIGHT? WHAT'S\nTAKING SO LONG?"
#define MK_TXT31t1	"HARDCORE_GAMER: THIS IS WHAT\nHAPPENS WHEN YOU'VE HAD TOO\nMANY CORONAS."
#define MK_TXT32t1	"ISANN KEKET: ON YOUR FEET, MARINE.\nSLAYERS NEVER SLEEP!"
#define MK_TXT33t1	"AMUSED BRIAN: WELL THAT DIDN'T GO\nACCORDING TO PLAN..."
#define MK_TXT34t1	"SECTOR666: IF YOU HIDE ON THE FLOOR\nFOREVER AT LEAST YOU WON'T DIE...\nWHOOPS, TOO LATE..."
#define MK_TXT35t1	"DEXIAZ: IT'S OFFICIAL! YOU SUCK AT\nPLAYING MAPS!"
#define MK_TXT36t1	"Z0K: IF YOU DROPPED SOME WEAPONS,\nYOU COULD HAVE DODGED, FATSO!"
#define MK_TXT37t1	"PUZZLEWELL: THEY'LL BURY YOU IN A\nLUNCHBOX (WAIT WRONG GAME...)."
#define MK_TXT38t1	"KMXEXII: I CAN ONLY HOPE THAT THIS\nDEATH WAS IN SOME WAY LOST SOUL OR\nPAIN ELEMENTAL RELATED."
#define MK_TXT39t1	"ENDLESS: ENDLESSLY DYING IS NOT\nA GOOD STRATEGY."
#define MK_TXT40t1	"ERROR: BACK TO THE DRAWING BOARD...\nTOO BAD YOU CAN'T DRAW."
#define MK_TXT41t1	"BLUETHUNDER: SO... YOU'RE GONNA DIE\nNOW... SERIOUSLY?!!"
#define MK_TXT42t1	"HYACSHO: MAYBE YOU SHOULD PLAY\nDOOM 1993 INSTEAD?"
#define MK_TXT43t1	"NOWANDTHEN64: OF ALL THE ATTEMPTS\nTHAT HAVE BEEN MADE AT THIS GAME,\nTHAT WAS CERTAINLY ONE OF THEM."
#define MK_TXT44t1	"SIXHUNDREDSIXTEEN: SURE LET THEM\nKILL YOU! WHAT A %#$@ING DISASTER..."
#define MK_TXT45t1	"MIKE_C: DON'T BLAME THE CONTROLLER,\nYOU BIG WUSS!"
#define MK_TXT46t1	"GLENN PLANT: THE RISING SUN WILL\nEVENTUALLY SET, A NEWBORN'S LIFE\nWILL FADE. FROM SUN TO MOON, MOON\nTO SUN. GIVE PEACEFUL REST TO THE\nLIVING DEAD."
#define MK_TXT47t1	"MOTHERLOAD: YOUR GREAT GRANDFATHER\nWAS IN A REAL WAR AND YOU'RE TOO\nMUCH OF A $#@&! TO BEAT THIS LEVEL."
#define MK_TXT48t1	"GERARDO: WHAT?? FEELING SLEEPY NOW?\nYOU CONSIDER YOURSELF SLAYER AND\nYOU WANT TO REST? SHAME ON YOU!"
#define MK_TXT49t1	"HEADSHOT: YOU'VE HUMOURED THE\nDEMONS GREATLY WITH YOUR\nARROGANCE AND CONTEMPT..."
#define MK_TXT50t1	"ZELLLOOO: WE HAVE PINKY,\nBUT WHERE'S THE BRAIN?"
#define MK_TXT51t1	"PEACHES: THIS ACTION TO DEFY THE\nEVIL ONE IS NOT WITHOUT RISK.\nCONFRONTATION IS OFTEN BEST\nAVOIDED..."

char *mockstrings1[] =
{
	MK_TXT01t1, MK_TXT02t1, MK_TXT03t1, MK_TXT04t1,
	MK_TXT05t1, MK_TXT06t1, MK_TXT07t1, MK_TXT08t1,
	MK_TXT09t1, MK_TXT10t1, MK_TXT11t1, MK_TXT12t1,
	MK_TXT13t1, MK_TXT14t1, MK_TXT15t1, MK_TXT16t1,
	MK_TXT17t1, MK_TXT18t1, MK_TXT19t1, MK_TXT20t1,
	MK_TXT21t1, MK_TXT22t1, MK_TXT23t1, MK_TXT24t1,
	MK_TXT25t1, MK_TXT26t1, MK_TXT27t1, MK_TXT28t1,
	MK_TXT29t1, MK_TXT30t1, MK_TXT31t1, MK_TXT32t1,
	MK_TXT33t1, MK_TXT34t1, MK_TXT35t1, MK_TXT36t1,
	MK_TXT37t1, MK_TXT38t1, MK_TXT39t1, MK_TXT40t1,
	MK_TXT41t1, MK_TXT42t1, MK_TXT43t1, MK_TXT44t1,
	MK_TXT45t1, MK_TXT46t1, MK_TXT47t1, MK_TXT48t1,
	MK_TXT49t1, MK_TXT50t1, MK_TXT51t1,
};

fixed_t forwardmove[2] = {0xE000, 0x16000};
fixed_t sidemove[2] = {0xE000, 0x16000};

#define SLOWTURNTICS    10
fixed_t angleturn[] =
	{50,50,83,83,100,116,133,150,150,166,
	133,133,150,166,166,200,200,216,216,233}; // fastangleturn

/*============================================================================= */

mobj_t *slidething;
extern fixed_t slidex, slidey;
extern line_t *specialline;

extern int last_Ltrig, last_Rtrig;

void P_SlideMove (mobj_t *mo);

/*
===================
=
= P_PlayerXYMovement
=
===================
*/

#define	STOPSPEED		0x1000
#define	FRICTION		0xd200  //Jag 0xd240

void P_PlayerXYMovement (mobj_t *mo)
{
	/* try to slide along a blocked move */
	if (!P_TryMove(mo, mo->x + mo->momx, mo->y + mo->momy))
		P_SlideMove(mo);

	/* slow down */
	if (mo->z > mo->floorz)
		return; /* no friction when airborne */

	if (mo->flags & MF_CORPSE)
		if (mo->floorz != mo->subsector->sector->floorheight)
			return; /* don't stop halfway off a step */

	if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
		&& mo->momy > -STOPSPEED && mo->momy < STOPSPEED) {
		mo->momx = 0;
		mo->momy = 0;
	} else {
		mo->momx = (mo->momx>>8)*(FRICTION>>8);
		mo->momy = (mo->momy>>8)*(FRICTION>>8);
	}
}


/*
===============
=
= P_PlayerZMovement
=
===============
*/

void P_PlayerZMovement (mobj_t *mo) // 80021f38
{
	/* */
	/* check for smooth step up */
	/* */
	if (mo->z < mo->floorz)
	{
		mo->player->viewheight -= (mo->floorz - mo->z);
		mo->player->deltaviewheight = (VIEWHEIGHT - mo->player->viewheight) >> 2;
	}

	/* */
	/* adjust height */
	/* */
	mo->z += mo->momz;

	/* */
	/* clip movement */
	/* */
	if (mo->z <= mo->floorz)
	{	/* hit the floor */
		if (mo->momz < 0)
		{
			if (mo->momz < -(GRAVITY*2))	/* squat down */
			{
				mo->player->deltaviewheight = mo->momz>>3;
				S_StartSound (mo, sfx_oof);
			}
			mo->momz = 0;
		}
		mo->z = mo->floorz;
	}
	else
	{
		if (mo->momz == 0)
			mo->momz = -(GRAVITY/2);
		else
			mo->momz -= (GRAVITY/4);
	}

	if (mo->z + mo->height > mo->ceilingz)
	{	/* hit the ceiling */
		if (mo->momz > 0)
			mo->momz = 0;
		mo->z = mo->ceilingz - mo->height;
	}
}


/*
================
=
= P_PlayerMobjThink
=
================
*/

void P_PlayerMobjThink (mobj_t *mobj) // 80022060
{
	state_t	*st;
	int		state;

	/* */
	/* momentum movement */
	/* */
	if (mobj->momx || mobj->momy)
		P_PlayerXYMovement (mobj);

	mobj->player->onground = (mobj->z <= mobj->floorz);

	if ( (mobj->z != mobj->floorz) || mobj->momz)
		P_PlayerZMovement (mobj);

	/* */
	/* cycle through states, calling action functions at transitions */
	/* */
	if (mobj->tics == -1)
		return;				/* never cycle */

	mobj->tics--;

	if (mobj->tics > 0)
		return;				/* not time to cycle yet */

	state = mobj->state->nextstate;
	st = &states[state];

	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;
}

/*============================================================================= */


/*
====================
=
= P_BuildMove
=
====================
*/

#define MAXSENSITIVITY    10

void P_BuildMove (player_t *player) // 80022154
{
	int         speed, sensitivity;
	int			buttons, oldbuttons;
	mobj_t		*mo;
	buttons_t	*cbutton;

	cbutton = BT_DATA[0];
	buttons = ticbuttons[0];
	oldbuttons = oldticbuttons[0];

	player->forwardmove = player->sidemove = player->angleturn = 0;
	
	if(Autorun == true && demoplayback == false) { // [Immorpher] New autorun option
		speed = (buttons & cbutton->BT_SPEED) < 1;	
	}
	else {
		speed = (buttons & cbutton->BT_SPEED) > 0;
	}
	sensitivity = 0;

	/*  */
	/* forward and backward movement  */
	/*  */
	if (cbutton->BT_FORWARD & buttons)
	{
		player->forwardmove = forwardmove[speed];
	}
	else if (cbutton->BT_BACK & buttons)
	{
		player->forwardmove = -forwardmove[speed];
	}
	else
	{
		/* Analyze analog stick movement (up / down) */
		sensitivity = (int)((buttons) << 24) >> 24;

		if(sensitivity >= MAXSENSITIVITY || sensitivity <= -MAXSENSITIVITY)
		{
			player->forwardmove += (forwardmove[1] * sensitivity) / 80;
		}
	}

	/*  */
	/* use two stage accelerative turning on the joypad  */
	/*  */
	if (((buttons & cbutton->BT_LEFT) && (oldbuttons & cbutton->BT_LEFT)))
		player->turnheld++;
	else if (((buttons & cbutton->BT_RIGHT) && (oldbuttons & cbutton->BT_RIGHT)))
		player->turnheld++;
	else
		player->turnheld = 0;

	if (player->turnheld >= SLOWTURNTICS)
		player->turnheld = SLOWTURNTICS-1;

	/*  */
	/* strafe movement  */
	/*  */
	if (buttons & cbutton->BT_STRAFELEFT)
	{
		if (demoplayback)
			player->sidemove -= sidemove[speed];
		else
			player->sidemove -= (sidemove[speed] * last_Ltrig) / 255;
	}
	if (buttons & cbutton->BT_STRAFERIGHT)
	{
		if (demoplayback)
			player->sidemove += sidemove[speed];
		else
			player->sidemove += (sidemove[speed] * last_Rtrig) / 255;
	}

	if (buttons & cbutton->BT_STRAFE)
	{
		if (buttons & cbutton->BT_LEFT)
		{
			player->sidemove = -sidemove[speed];
		}
		else if (buttons & cbutton->BT_RIGHT)
		{
			player->sidemove = sidemove[speed];
		}
		else
		{
			/* Analyze analog stick movement (left / right) */
			sensitivity = (int)(((buttons & 0xff00) >> 8) << 24) >> 24;

			if(sensitivity >= MAXSENSITIVITY || sensitivity <= -MAXSENSITIVITY)
			{
				player->sidemove += (sidemove[1] * sensitivity) / 80;
			}
		}
	}
	else
	{
		if (sensitivity == 0)
			speed = 0;

		if (cbutton->BT_LEFT & buttons)
		{
			player->angleturn =  angleturn[player->turnheld + (speed * SLOWTURNTICS)] << 17;
		}
		else if (cbutton->BT_RIGHT & buttons)
		{
			player->angleturn = -angleturn[player->turnheld + (speed * SLOWTURNTICS)] << 17;
		}
		else
		{
			/* Analyze analog stick movement (left / right) */
			sensitivity = (int)(((buttons & 0xff00) >> 8) << 24) >> 24;
			sensitivity = -sensitivity;

			if(sensitivity >= MAXSENSITIVITY || sensitivity <= -MAXSENSITIVITY)
			{
				sensitivity = ((M_SENSITIVITY << 3) + 233) * sensitivity;
				player->angleturn += (sensitivity / 80) << 17;
			}
		}
	}

	/* */
	/* if slowed down to a stop, change to a standing frame */
	/* */
	mo = player->mo;

	if (!mo->momx && !mo->momy && player->forwardmove == 0 && player->sidemove == 0 )
	{	/* if in a walking frame, stop moving */
		if (mo->state == &states[S_002] //S_PLAY_RUN1
		|| mo->state == &states[S_003]  //S_PLAY_RUN2
		|| mo->state == &states[S_004]  //S_PLAY_RUN3
		|| mo->state == &states[S_005]) //S_PLAY_RUN4
			P_SetMobjState (mo, S_001); //S_PLAY
	}
}

/*
===============================================================================

						movement

===============================================================================
*/

/*
==================
=
= P_Thrust
=
= moves the given origin along a given angle
=
==================
*/

void P_Thrust (player_t *player, angle_t angle, fixed_t move) // 800225BC
{
	angle >>= ANGLETOFINESHIFT;
	player->mo->momx += FixedMul(vblsinframe[0] * move, finecosine[angle]);
	player->mo->momy += FixedMul(vblsinframe[0] * move, finesine[angle]);
}



/*
==================
=
= P_CalcHeight
=
= Calculate the walking / running height adjustment
=
==================
*/

void P_CalcHeight (player_t *player) // 80022670
{
	int			angle;
	fixed_t		bob;
	fixed_t		val;

	/* */
	/* regular movement bobbing (needs to be calculated for gun swing even */
	/* if not on ground) */
	/* OPTIMIZE: tablify angle  */
	/* */
	val = player->mo->momx;
	player->bob = FixedMul(val, val);
	val = player->mo->momy;
	player->bob += FixedMul(val, val);

	player->bob >>= 2;
	if (player->bob > MotionBob)
	{
		player->bob = MotionBob;
	}

	if (!player->onground)
	{
		player->viewz = player->mo->z + VIEWHEIGHT;
		if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
			player->viewz = player->mo->ceilingz-4*FRACUNIT;
		return;
	}

	angle = (FINEANGLES/40*ticon)&(FINEANGLES-1);
	bob = FixedMul((player->bob / 2), finesine[angle]);

	/* */
	/* move viewheight */
	/* */
	if (player->playerstate == PST_LIVE)
	{
		player->viewheight += player->deltaviewheight;
		if (player->viewheight > VIEWHEIGHT)
		{
			player->viewheight = VIEWHEIGHT;
			player->deltaviewheight = 0;
		}
		if (player->viewheight < VIEWHEIGHT/2)
		{
			player->viewheight = VIEWHEIGHT/2;
			if (player->deltaviewheight <= 0)
				player->deltaviewheight = 1;
		}

		if (player->deltaviewheight)
		{
			player->deltaviewheight += FRACUNIT/2;
			if (!player->deltaviewheight)
				player->deltaviewheight = 1;
		}
	}
	player->viewz = player->mo->z + player->viewheight + bob;
	if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
		player->viewz = player->mo->ceilingz-4*FRACUNIT;
}

/*
=================
=
= P_MovePlayer
=
=================
*/

void P_MovePlayer (player_t *player) // 8002282C
{
	player->mo->angle += vblsinframe[0] * player->angleturn;

	if(player->onground)
	{
		if (player->forwardmove)
			P_Thrust (player, player->mo->angle, player->forwardmove);
		if (player->sidemove)
			P_Thrust (player, player->mo->angle-ANG90, player->sidemove);
	}

	if ((player->forwardmove || player->sidemove) && player->mo->state == &states[S_001])//S_PLAY
		P_SetMobjState (player->mo, S_002);//S_PLAY_RUN1
}


/*
=================
=
= P_DeathThink
=
=================
*/

void P_DeathThink (player_t *player)
{
	angle_t angle, delta;
	int mockrandom; // [Immorpher] store mock text randomizer

	P_MovePsprites (player);

	/* fall to the ground */
	if (player->viewheight > 8*FRACUNIT)
		player->viewheight -= FRACUNIT;

	player->onground = (player->mo->z <= player->mo->floorz);

	P_CalcHeight (player);

	if (player->attacker && player->attacker != player->mo) {
		angle = R_PointToAngle2 (player->mo->x, player->mo->y, player->attacker->x, player->attacker->y);
		delta = angle - player->mo->angle;
		if (delta < ANG5 || delta > (unsigned)-ANG5) {
			/* looking at killer, so fade damage flash down */
			player->mo->angle = angle;
			if (player->damagecount)
				player->damagecount--;
		}
		else if (delta < ANG180)
			player->mo->angle += ANG5;
		else
			player->mo->angle -= ANG5;
	}
	else if (player->damagecount)
		player->damagecount--;

	/* mocking text */
	if ((ticon - deathmocktics) > MAXMOCKTIME) {
		do { // [Immorpher] Prevent mock string from repeating twice
			mockrandom = (I_Random()+ticon) % 51; // Updated randomizer for more fun
		} while(player->message1 == mockstrings1[mockrandom]);

		player->messagetic = 2*MSGTICS; // [Immorpher] Doubled message time to read them!
		player->message = mockstrings1[mockrandom];
		player->messagecolor = 0xff200000;
		deathmocktics = ticon;
	}

	if (((ticbuttons[0] & (PAD_A|PAD_B|ALL_TRIG|ALL_CBUTTONS)) != 0) &&
		(player->viewheight <= 8*FRACUNIT)) {
		player->playerstate = PST_REBORN;
	}

	if (player->bonuscount)
		player->bonuscount -= 1;

	// [d64] - recoil pitch from weapons
	if (player->recoilpitch)
		player->recoilpitch = (((player->recoilpitch << 2) - player->recoilpitch) >> 2);

	if(player->bfgcount) {
		player->bfgcount -= 6;

		if(player->bfgcount < 0)
			player->bfgcount = 0;
	}
}

/*
===============================================================================
=
= P_PlayerInSpecialSector
=
= Called every tic frame that the player origin is in a special sector
=
===============================================================================
*/

void P_PlayerInSpecialSector (player_t *player, sector_t *sec) // 80022B1C
{
	fixed_t speed;

	if (player->mo->z != sec->floorheight)
		return;		/* not all the way down yet */

	if(sec->flags & MS_SECRET)
	{
		player->secretcount++;
		player->message = "You found a secret area!";
		player->messagetic = MSGTICS;
		player->messagecolor = 0x00ffff00;
		sec->flags &= ~MS_SECRET;
	}

	if(sec->flags & MS_DAMAGEX5)    /* NUKAGE DAMAGE */
	{
		if(!player->powers[pw_ironfeet])
		{
			if ((gamevbls < (int)gametic) && !(gametic & 31))
				  P_DamageMobj(player->mo, NULL, NULL, 5);
		}
	}

	if(sec->flags & MS_DAMAGEX10)    /* HELLSLIME DAMAGE */
	{
		if(!player->powers[pw_ironfeet])
		{
			if ((gamevbls < (int)gametic) && !(gametic & 31))
				  P_DamageMobj(player->mo, NULL, NULL, 10);
		}
	}

	if(sec->flags & MS_DAMAGEX20)    /* SUPER HELLSLIME DAMAGE */
	{
		if(!player->powers[pw_ironfeet] || (P_Random() < 5))
		{
			if ((gamevbls < (int)gametic) && !(gametic & 31))
				  P_DamageMobj(player->mo, NULL, NULL, 20);
		}
	}

	if(sec->flags & MS_SCROLLFLOOR)
	{
		if(sec->flags & MS_SCROLLFAST)
			speed = 0x3000;
		else
			speed = 0x1000;

		if(sec->flags & MS_SCROLLLEFT)
		{
			P_Thrust(player, ANG180, speed);
		}
		else if(sec->flags & MS_SCROLLRIGHT)
		{
			P_Thrust(player, 0, speed);
		}

		if(sec->flags & MS_SCROLLUP)
		{
			P_Thrust(player, ANG90, speed);
		}
		else if(sec->flags & MS_SCROLLDOWN)
		{
			P_Thrust(player, ANG270, speed);
		}
	}
}

/*
=================
=
= P_PlayerThink
=
=================
*/

void P_PlayerThink (player_t *player) // 80022D60
{
	int		     buttons, oldbuttons;
	buttons_t    *cbutton;
	weapontype_t weapon;
	sector_t     *sec;

	buttons = ticbuttons[0];
	oldbuttons = oldticbuttons[0];
	cbutton = BT_DATA[0];

	/* */
	/* check for weapon change */
	/* */
	if (player->playerstate == PST_LIVE)
	{
		weapon = player->pendingweapon;
		if (weapon == wp_nochange)
			weapon = player->readyweapon;

		if ((buttons & cbutton->BT_WEAPONBACKWARD) && !(oldbuttons & cbutton->BT_WEAPONBACKWARD)) // [Immorpher] Change weapon decriment for easier chainsaw access
		{
			if((int)(weapon) >= wp_chainsaw)
			{
				while(--weapon >= wp_chainsaw && !player->weaponowned[weapon]);
			}
			
			if((int)weapon >= wp_chainsaw)
				player->pendingweapon = weapon;
		}
		else if ((buttons & cbutton->BT_WEAPONFORWARD) && !(oldbuttons & cbutton->BT_WEAPONFORWARD))
		{
		    if((int)(weapon) < NUMWEAPONS)
			{
				while(++weapon < NUMWEAPONS && !player->weaponowned[weapon]);
			}

			if((int)weapon < NUMWEAPONS)
				player->pendingweapon = weapon;
		}
	}

	if (!gamepaused)
	{
		P_PlayerMobjThink(player->mo);
		P_BuildMove(player);

		sec = player->mo->subsector->sector;
		if (sec->flags & (MS_SECRET | MS_DAMAGEX5 | MS_DAMAGEX10 | MS_DAMAGEX20 | MS_SCROLLFLOOR))
			P_PlayerInSpecialSector(player, sec);

		if (player->playerstate == PST_DEAD)
		{
			P_DeathThink(player);
			return;
		}

		/* */
		/* chain saw run forward */
		/* */
		if (player->mo->flags & MF_JUSTATTACKED)
		{
			player->angleturn = 0;
			// ...
			player->forwardmove = 0xc800;
			player->sidemove = 0;
			player->mo->flags &= ~MF_JUSTATTACKED;
		}

		/* */
		/* move around */
		/* reactiontime is used to prevent movement for a bit after a teleport */
		/* */

		if (player->mo->reactiontime)
			player->mo->reactiontime--;
		else
			P_MovePlayer(player);

		P_CalcHeight(player);

		/* */
		/* check for use */
		/* */

		if ((buttons & cbutton->BT_USE))
		{
			if (player->usedown == false)
			{
				P_UseLines(player);
				player->usedown = true;
			}
		}
		else
		{
			player->usedown = false;
		}

		if (buttons & cbutton->BT_ATTACK)
		{
			P_SetMobjState(player->mo, S_006);//S_PLAY_ATK1
			player->attackdown++;
		}
		else
		{
			player->attackdown = 0;
		}

		/* */
		/* cycle psprites */
		/* */

		P_MovePsprites(player);

		/* */
		/* counters */
		/* */

		if (gamevbls < gametic)
		{
			if (player->powers[pw_strength] > 1)
				player->powers[pw_strength]--;	/* strength counts down to diminish fade */

			if (player->powers[pw_invulnerability])
				player->powers[pw_invulnerability]--;

			if (player->powers[pw_invisibility])
			{
				player->powers[pw_invisibility]--;
				if (!player->powers[pw_invisibility])
				{
					player->mo->flags &= ~MF_SHADOW;
				}
				else if ((player->powers[pw_invisibility] < 61) && !(player->powers[pw_invisibility] & 7))
				{
					player->mo->flags ^= MF_SHADOW;
				}
			}

			if (player->powers[pw_infrared])
				player->powers[pw_infrared]--;

			if (player->powers[pw_ironfeet])
				player->powers[pw_ironfeet]--;

			if (player->damagecount)
				player->damagecount--;

			if (player->bonuscount)
				player->bonuscount--;

			// [d64] - recoil pitch from weapons
			if (player->recoilpitch)
				player->recoilpitch = (((player->recoilpitch << 2) - player->recoilpitch) >> 2);

			if(player->bfgcount)
			{
				player->bfgcount -= 6;

				if(player->bfgcount < 0)
					player->bfgcount = 0;
			}
		}
	}
}
