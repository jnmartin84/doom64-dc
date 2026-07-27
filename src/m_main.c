/* m_main.c -- menu routines */

#include "doomdef.h"
#include "r_local.h"
#include "st_main.h"

int Wireframe = 0;

extern int extra_episodes;
extern int kneedeep_only;
//intermission
int DrawerStatus;

//static int button_code_to_symbol_index(uint32_t code);
static int dc_button_to_symbol(uint32_t code);
#define CT_TXT00 "default: %d"
#define CT_TXT01 "right"
#define CT_TXT02 "left"
#define CT_TXT03 "forward"
#define CT_TXT04 "backward"
#define CT_TXT05 "attack"
#define CT_TXT06 "use"
#define CT_TXT07 "map"
#define CT_TXT08 "speed"
#define CT_TXT09 "strafe on"
#define CT_TXT10 "strafe left"
#define CT_TXT11 "strafe right"
#define CT_TXT12 "weapon backward"
#define CT_TXT13 "weapon forward"

char *ControlText[] =
	{ CT_TXT00, CT_TXT01, CT_TXT02, CT_TXT03, CT_TXT04, CT_TXT05, CT_TXT06,
	  CT_TXT07, CT_TXT08, CT_TXT09, CT_TXT10, CT_TXT11, CT_TXT12, CT_TXT13 };

#define M_TXT00 "Gamepad"
#define M_TXT01 "Volume"
#define M_TXT02 "Video"
#define M_TXT03 "Password"
#define M_TXT04 "Main Menu"
#define M_TXT05 "Restart Level"
#define M_TXT06 "\x90 Return"
#define M_TXT07 "Music Volume"
#define M_TXT08 "Sound Volume"
#define M_TXT09 "Brightness"
#define M_TXT10 "Resume"
#define M_TXT11 "Options"
#define M_TXT12 "Autorun:"
#define M_TXT13 "--------" // was Defaults
#define M_TXT14 "New Game"
#define M_TXT15 "Be Gentle!"
#define M_TXT16 "Bring It On!"
#define M_TXT17 "I Own Doom!"
#define M_TXT18 "Watch Me Die!"
#define M_TXT19 "Be Merciless!"
#define M_TXT20 "Yes"
#define M_TXT21 "No"
#define M_TXT22 "Features"
#define M_TXT23 "WARP TO LEVEL"
#define M_TXT24 "INVULNERABLE"
#define M_TXT25 "HEALTH BOOST"
#define M_TXT26 "SECURITY KEYS"
#define M_TXT27 "WEAPONS"
#define M_TXT28 "Exit"
#define M_TXT29 "DEBUG"
#define M_TXT30 "TEXTURE TEST"
#define M_TXT31 "WALL BLOCKING"
#define M_TXT32 "------ -------" // was Center Display
#define M_TXT33 "Messages:"
#define M_TXT34 "Opacity"
#define M_TXT35 "LOCK MONSTERS"
#define M_TXT36 "SCREENSHOT"
#define M_TXT37 "MAP EVERYTHING"
#define M_TXT38 "MACRO PEEK"
#define M_TXT39 "MUSIC TEST"
#define M_TXT40 "WARP TO FUN"
#define M_TXT41 "Movement"
#define M_TXT42 "Original" // Original default for Doom 64
#define M_TXT43 "Sensitivity"
#define M_TXT44 "Manage VMU"
#define M_TXT45 "Do not use VMU"
#define M_TXT46 "Try again"
#define M_TXT47 "Create game note"

// New additions to Doom 64 RE and Merciless Edition
#define M_TXT48 "COLORS" // [GEC] NEW CHEAT CODE
#define M_TXT49 "FULL BRIGHT" // [GEC] NEW CHEAT CODE
#define M_TXT50 "Filtering:" // [GEC] New video filter option
#define M_TXT51 "Display" // [Immorpher] New menu item
#define M_TXT52 "Motion Bob" // [Immorpher] New menu item
#define M_TXT53 "Dither Filter:" // [Immorpher] Dither Filter
#define M_TXT54 "Anti-Aliasing:" // [Immorpher] New anti-aliasing option
#define M_TXT55 "Interlacing:" // [Immorpher] New interlacing option
#define M_TXT56 "Color Dither:" // [Immorpher] New color dither options
#define M_TXT57 "Flash Brightness" // [Immorpher] New flash brightness option
#define M_TXT58 "Merciless" // [Immorpher] Merciless default settings
#define M_TXT59 "Immorpher" // [Immorpher] Immorpher default settings
#define M_TXT60 \
	"Accessible" // [Immorpher] Increased accessibility default settings
#define M_TXT61 "Story Text:" // [Immorpher] Skip cut scenes
#define M_TXT62 "Map Stats:" // [Immorpher] Display automap statistics
#define M_TXT63 "Status HUD" // [Immorpher] New menu option for HUD elements!
#define M_TXT64 "Margin" // [Immorpher] Adjust the margin for the HUD
#define M_TXT65 "WARP TO MOTHER" // [Immorpher] New features menu warps
#define M_TXT66 "WARP TO SECRET" // [Immorpher] New features menu warps
#define M_TXT67 "Colored:" // [Immorpher] Colored hud
#define M_TXT68 "GAMMA CORRECT" // [Immorpher] NEW CHEAT CODE

// Credits
#define M_TXT69 "DOOM 64 DC CREDITS" // [Immorpher] Credits
#define M_TXT70 "DEVELOPMENT BY: JNMARTIN84" // [Immorpher] Credits
#define M_TXT71 "BASED ON: MERCILESS EDITION, XE" // [Immorpher] Credits
#define M_TXT72 "BY IMMORPHER" // [Immorpher] Credits
#define M_TXT73 "BASED ON: DOOM 64 RE BY ERICK194" // [Immorpher] Credits
#define M_TXT74 "SPECIAL THANKS TO ALL CONTRIBUTORS" // [Immorpher] Credits
#define M_TXT75 "IMMORPHER" // [Immorpher] Credits
#define M_TXT76 "FALCO GIRGIS, PAUL CERCUEIL," // [Immorpher] Credits
#define M_TXT77 "QUZAR, SWAT, BBHOODSTA, KAZADE," // [Immorpher] Credits
#define M_TXT78 "ERIK5249, LOBOTOMY, MITTENS," // [Immorpher] Credits
#define M_TXT79 "STRIKERTHEHEDGEFOX," // [Immorpher] Credits
#define M_TXT80 "#KALLISTIOS/SIMULANT DISCORD" // [Immorpher] Credits
#define M_TXT81 "Z0K, ANDREW HULSHULT," // [Immorpher] Credits
#define M_TXT82 "EVERY TESTER" // [Immorpher] Credits
#define M_TXT83 "" // [Immorpher] Credits
#define M_TXT84 "ID SOFTWARE AND MIDWAY" // [Immorpher] Credits

#define M_TXT85 "Absolution"
#define M_TXT86 "Lost Levels"
#define M_TXT87 "Knee-Deep\nIn The Dead"

#define M_TXT88 "Quality"
#define M_TXT89 "Low"
#define M_TXT90 "Medium"
#define M_TXT91 "Ultra"

#define M_TXT92 "FPS"
#define M_TXT93 "30"
#define M_TXT94 "Uncapped"

#define M_TXT95 "Rumble:"

#define M_TXT96 "Interpolate"

#define M_TXT97 "VMU Display:"

#define M_TXT98 "On"
#define M_TXT99 "Unused"
#define M_TXT100 "Unused"

#define M_TXT101 "WIREFRAME"

static char *MenuText[] =
	{
		M_TXT00, M_TXT01, M_TXT02, M_TXT03, M_TXT04, M_TXT05, M_TXT06,
		M_TXT07, M_TXT08, M_TXT09, M_TXT10, M_TXT11, M_TXT12, M_TXT13,
		M_TXT14, M_TXT15, M_TXT16, M_TXT17, M_TXT18, M_TXT19, M_TXT20,
		M_TXT21, M_TXT22, M_TXT23, M_TXT24, M_TXT25, M_TXT26, M_TXT27,
		M_TXT28, M_TXT29, M_TXT30, M_TXT31, M_TXT32, M_TXT33, M_TXT34,
		M_TXT35, M_TXT36, M_TXT37, M_TXT38, M_TXT39, M_TXT40, M_TXT41,
		M_TXT42, M_TXT43, M_TXT44, M_TXT45, M_TXT46, M_TXT47, M_TXT48,
		M_TXT49, M_TXT50, M_TXT51, M_TXT52, M_TXT53, M_TXT54, M_TXT55,
		M_TXT56, M_TXT57, M_TXT58, M_TXT59, M_TXT60, M_TXT61, M_TXT62,
		M_TXT63, M_TXT64, M_TXT65, M_TXT66, M_TXT67, M_TXT68, M_TXT69,
		M_TXT70, M_TXT71, M_TXT72, M_TXT73, M_TXT74, M_TXT75, M_TXT76,
		M_TXT77, M_TXT78, M_TXT79, M_TXT80, M_TXT81, M_TXT82, M_TXT83,
		M_TXT84,
		M_TXT85, M_TXT86, M_TXT87,
		M_TXT88, M_TXT89, M_TXT90, M_TXT91,
		M_TXT92, M_TXT93, M_TXT94, M_TXT95, M_TXT96, M_TXT97, M_TXT98,
		M_TXT99, M_TXT100, M_TXT101
	};

#define NUM_MENU_TITLE 3
menuitem_t Menu_Title[NUM_MENU_TITLE] =
	{
		{ 14, 115, 170 }, // New Game
		{ 3, 115, 190 }, // Password
		{ 11, 115, 210 }, // Options
	};

#define NUM_MENU_SKILL 6
menuitem_t Menu_Skill[NUM_MENU_SKILL] =
	{
		{ 15, 102, 70 }, // Be Gentle!
		{ 16, 102, 90 }, // Bring it on!
		{ 17, 102, 110 }, // I own Doom!
		{ 18, 102, 130 }, // Watch me die!
		{ 19, 102, 150 }, // Be merciless!
		{ 6, 102, 180 }, // Return
	};

#define NUM_MENU_EPISODES 3
menuitem_t Menu_Episode[NUM_MENU_EPISODES] =
{
	{ 85, 102, 80 },    // Doom 64
	{ 87, 102, 100},    // Knee-Deep In The Dead
	{ 6, 102, 180 }, // Return
};

#define NUM_MENU_2EPISODES 4
menuitem_t Menu_2Episode[NUM_MENU_2EPISODES] =
{
	{ 85, 102, 80 },    // Doom 64
	{ 86, 102, 100},    // The Lost Levels
	{ 87, 102, 120},    // Knee-Deep In The Dead
	{ 6, 102, 200 }, // Return
};


#define NUM_MENU_OPTIONS 7
menuitem_t Menu_Options[NUM_MENU_OPTIONS] =
	{
		{ 0, 112, 60 }, // Gamepad
		{ 41, 112, 80 }, // Movement
		{ 1, 112, 100 }, // Volume
		{ 2, 112, 120 }, // Video
		{ 51, 112, 140 }, // Display
		{ 63, 112, 160 }, // Status HUD
		{ 6, 112, 180 /*200*/ }, // Return
	};

#define NUM_MENU_VOLUME 3
menuitem_t Menu_Volume[NUM_MENU_VOLUME] =
	{
		{ 7, 82, 60 }, // Music Volume
		{ 8, 82, 100 }, // Sound Volume
		{ 6, 82, 140 }, // Return
	};

#define NUM_MENU_MOVEMENT 5
menuitem_t Menu_Movement[NUM_MENU_MOVEMENT] =
	{
		{ 52, 82, 60 }, // Motion Bob
		{ 43, 82, 100 }, // Sensitivity
		{ 12, 82, 140 }, // Autorun
		{ 95, 82, 160 }, // Rumble
		{ 6, 82, 180 }, // Return
	};

#define NUM_MENU_VIDEO 6
menuitem_t Menu_Video[NUM_MENU_VIDEO] = {
	{ 9, 82, 60 }, // Brightness
	{ 50, 82, 100 }, // Video Filter
	{ 88, 82, 120 }, // Quality menu
	{ 92, 82, 140 }, // fps menu
	{ 96, 82, 160 }, // interpolate
	{ 6, 82, 180 }, // Return
};

#define NUM_MENU_DISPLAY 4
menuitem_t Menu_Display[NUM_MENU_DISPLAY] =
	{
		{ 61, 62, 120 - 60 }, // Story Text
		{ 62, 62, 140 - 60 }, // Map Stats
		{ 97, 62, 160 - 60 }, // VMU Display
		{ 6, 62, 180 - 60 }, // Return
	};

#define NUM_MENU_STATUSHUD 5
menuitem_t Menu_StatusHUD[NUM_MENU_STATUSHUD] =
	{
		{ 64, 82, 60 }, // Margin
		{ 34, 82, 100 }, // Opacity
		{ 67, 82, 140 }, // Colored HUD
		{ 33, 82, 160 }, // Messages
		{ 6, 82, 180 }, // Return
	};

#define NUM_MENU_GAME 5
menuitem_t Menu_Game[NUM_MENU_GAME] =
	{
		{ 3, 122, 60 }, // Password
		{ 11, 122, 80 }, // Options
		{ 4, 122, 100 }, // Main Menu
		{ 5, 122, 120 }, // Restart Level
		{ 22, 122, 140 }, // Features
	};

#define NUM_MENU_QUIT 2
menuitem_t Menu_Quit[NUM_MENU_QUIT] =
	{
		{ 20, 142, 100 }, // Yes
		{ 21, 142, 120 }, // No
	};

#define NUM_MENU_DELETENOTE 2
menuitem_t Menu_DeleteNote[NUM_MENU_DELETENOTE] =
	{
		{ 20, 142, 100 }, // Yes
		{ 21, 142, 120 }, // No
	};

#define NUM_MENU_CONTROLLERPAKBAD 2
menuitem_t Menu_ControllerPakBad[NUM_MENU_CONTROLLERPAKBAD] =
	{
		{ 46, 120, 100 }, // Try again
		{ 45, 120, 120 }, // Do not use Pak
	};

#define NUM_MENU_CONTROLLERPAKFULL 3
menuitem_t Menu_ControllerPakFull[NUM_MENU_CONTROLLERPAKFULL] =
	{
		{ 44, 110, 90 }, // Manage Pak
		{ 47, 110, 110 }, // Create game note
		{ 45, 110, 130 }, // Do not use Pak
	};


#define NUM_MENU_CREATENOTE 3
menuitem_t Menu_CreateNote[NUM_MENU_CREATENOTE] =
	{
		{ 20, 110, 90 }, // Yes
		{ 45, 110, 110 }, // Do not use Pak
		{ 44, 110, 130 }, // Manage Pak
	};

#define NUM_MENU_FEATURES 11
menuitem_t Menu_Features[NUM_MENU_FEATURES] =
	{
		{ 23, 40, 50 }, // WARP TO LEVEL
		{ 24, 40, 60 }, // INVULNERABLE
		{ 25, 40, 70 }, // HEALTH BOOST
		{ 27, 40, 80 }, // WEAPONS
		{ 37, 40, 90 }, // MAP EVERYTHING
		{ 26, 40, 100 }, // SECURITY KEYS
		{ 31, 40, 110 }, // WALL BLOCKING
		{ 35, 40, 120 }, // LOCK MONSTERS
		{ 39, 40, 130 }, // MUSIC TEST
		{ 69, 40, 140 }, // Doom 64 DC credits
		{ 101, 40, 150 }, // WIREFRAME
	};

#define NUM_DOOM64DC_CREDITS 15
menuitem_t Doom64DC_Credits[NUM_DOOM64DC_CREDITS] =
	{
		{ 70, 20, 45 }, // Credits

		{ 71, 20, 60 }, // Credits
		{ 72, 40, 70 }, // Credits
		{ 73, 20, 80 }, // Credits

		{ 74, 20, 95 }, // Credits
		{ 75, 20, 105 }, // Credits
		{ 76, 20, 115 }, // Credits
		{ 77, 20, 125 }, // Credits
		{ 78, 20, 135 }, // Credits
		{ 79, 20, 145 }, // Credits
		{ 80, 20, 155 }, // Credits
		{ 81, 20, 165 }, // Credits
		{ 82, 20, 175 }, // Credits
		{ 83, 20, 185 }, // Credits
		{ 84, 20, 195 }, // Credits
	};

menudata_t MenuData[8];
int MenuAnimationTic;
int cursorpos;
float f_m_vframe1;
menuitem_t *MenuItem;
int itemlines;
menufunc_t MenuCall;

int linepos;
int text_alpha_change_value;
int MusicID;
int m_actualmap;
int last_ticon;

skill_t startskill;
int startmap;
int UseVMU;

//-----------------------------------------

static char textbuff[256];

#define MAX_BRIGHTNESS 127

doom64_settings_t  __attribute__((aligned(32))) menu_settings;

int force_vmu_refresh = 0;

#define M_FILTER_NONE 0
#define M_FILTER_BILINEAR 2

void M_ResetSettings(doom64_settings_t *s) {
	s->HUDopacity = 255;
	s->SfxVolume = 45;
	s->MusVolume = 45;
	s->brightness = MAX_BRIGHTNESS;
	s->enable_messages = 1;
	s->M_SENSITIVITY = 27;
	s->MotionBob = 16 << FRACBITS;
	s->Rumble = 0;
	s->VideoFilter = M_FILTER_BILINEAR;
	s->Autorun = 1;
	s->StoryText = 1;
	s->MapStats = 1;
	s->HUDmargin = 20;
	s->ColoredHUD = 0;
	s->Quality = 2;
	s->FpsUncap = 1;
	s->PlayDeadzone = 0;
	s->Interpolate = 0;
	s->VmuDisplay = 0;

	if (I_CheckControllerPak() == 0) {
		I_ReadPakSettings(s);
	}

	s->version = SETTINGS_SAVE_VERSION;
	s->runintroduction = 0;
}

int MenuIdx = 0;
int text_alpha = 255;
int ConfgNumb = 0;
const boolean FeaturesUnlocked = true;
int force_filter_flush = 0;
//int FlashBrightness = 16;

int __attribute__((aligned(16))) ActualConfiguration[13] =
	{ PAD_RIGHT,   PAD_LEFT, PAD_UP,     PAD_DOWN,   PAD_Z_TRIG,
	  PAD_RIGHT_C, PAD_UP_C, PAD_LEFT_C, PAD_DOWN_C, PAD_L_TRIG,
	  PAD_R_TRIG,  PAD_A,    PAD_B };

int __attribute__((aligned(16))) CustomConfiguration[13] =
	{ PAD_RIGHT,   PAD_LEFT, PAD_UP,     PAD_DOWN,   PAD_Z_TRIG,
	  PAD_RIGHT_C, PAD_UP_C, PAD_LEFT_C, PAD_DOWN_C, PAD_L_TRIG,
	  PAD_R_TRIG,  PAD_A,    PAD_B };

int __attribute__((aligned(16))) DefaultConfiguration[1][13] =
	{
		// Default 1
		{ PAD_RIGHT,   PAD_LEFT, PAD_UP,     PAD_DOWN,   PAD_Z_TRIG,
		  PAD_RIGHT_C, PAD_UP_C, PAD_LEFT_C, PAD_DOWN_C, PAD_L_TRIG,
		  PAD_R_TRIG,  PAD_A,    PAD_B }
	};

//-----------------------------------------

extern void P_FlushAllCached(void);
int from_menu = 1;
int in_menu = 1;

int M_RunTitle(void)
{
	int exit;
	P_FlushAllCached();
	DrawerStatus = 0;
	startskill = sk_easy;
	startmap = 1;

	MenuIdx = 0;
	MenuItem = Menu_Title;
	MenuCall = M_MenuTitleDrawer;
	text_alpha = 0;
	itemlines = NUM_MENU_TITLE;
	cursorpos = 0;
	last_ticon = 0;
	from_menu = 1;
	S_StartMusic(116);
	from_menu = 0;
	exit = MiniLoop(M_FadeInStart, M_MenuClearCall, M_MenuTicker, M_MenuGameDrawer);

	I_WIPE_FadeOutScreen();
	S_StopMusic();

	if (exit == ga_timeout)
		return ga_timeout;

	G_InitNew(startskill, startmap, (gametype_t)ga_nothing);
	G_RunGame();

	return 0;
}

int M_ControllerPak(void)
{
	int exit;
	int ret;
	boolean PakBad;

	PakBad = false;

	while (1) {
		ret = I_CheckControllerPak();

		if ((ret != PFS_ERR_NOPACK)) //&& (ret != PFS_ERR_ID_FATAL))
			PakBad = true;

		if (ret == 0) {
			ret = I_ReadPakFile();

			// Free Pak_Data
			if (Pak_Data) {
				Z_Free(Pak_Data);
				Pak_Data = NULL;
			}

			if (ret == 0) {
				exit = ga_nothing;
				break;
			}

			// Create VMU file
			MenuItem = Menu_CreateNote;
			itemlines = NUM_MENU_CREATENOTE;
			MenuCall = M_MenuTitleDrawer;
			cursorpos = 0;

			MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
			M_FadeOutStart(8);

			if (cursorpos != 0) {
				exit = ga_exit;
				break;
			}

			// Check Memory and Files Used on VMU
			if ((Pak_Memory > 0) && (FilesUsed != 200)) {
				if (I_CreatePakFile() != 0)
					goto ControllerPakBad;

				exit = ga_nothing;
				break;
			}

			// Show VMU Full
			MenuItem = Menu_ControllerPakFull;
			itemlines = NUM_MENU_CONTROLLERPAKFULL;
			MenuCall = M_MenuTitleDrawer;
			cursorpos = 0;

			MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
			M_FadeOutStart(8);

			if (cursorpos != 1) {
				exit = ga_exit;
				break;
			}
		} else {
			if (PakBad == false) {
				exit = ga_exit;
				break;
			}

			// Show Controller Pak Bad
		ControllerPakBad:
			MenuItem = Menu_ControllerPakBad;
			itemlines = NUM_MENU_CONTROLLERPAKBAD;
			MenuCall = M_MenuTitleDrawer;
			cursorpos = 0;

			MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
			M_FadeOutStart(8);

			if (cursorpos != 0) {
				exit = ga_exit;
				break;
			}
		}
	}

	return exit;
}

#define MAXSENSITIVITY 20

int M_ButtonResponder(int buttons)
{
	int sensitivity;
	int NewButtons;

	/* Copy Default Buttons */
	NewButtons = (buttons);

	/* Analyze Analog Stick (up / down) */
	sensitivity = (int)((buttons) << 24) >> 24;

	if (sensitivity <= -MAXSENSITIVITY)
		NewButtons |= PAD_DOWN;
	else if (sensitivity >= MAXSENSITIVITY)
		NewButtons |= PAD_UP;

	/* Analyze Analog Stick (left / right) */
	sensitivity = (int)(((buttons & 0xff00) >> 8) << 24) >> 24;

	if (sensitivity <= -MAXSENSITIVITY)
		NewButtons |= PAD_LEFT;
	else if (sensitivity >= MAXSENSITIVITY)
		NewButtons |= PAD_RIGHT;

	return NewButtons & 0xffff0000;
}

void M_AlphaInStart(void)
{
	text_alpha = 0;
	text_alpha_change_value = 20;
}

void M_AlphaOutStart(void)
{
	text_alpha = 255;
	text_alpha_change_value = -20;
}

int M_AlphaInOutTicker(void)
{
	static int last_f_gametic = 0;

	if (((int)f_gamevbls < (int)f_gametic) && (((int)f_gametic & 3) == 0)) {
		if (last_f_gametic != (int)f_gametic) {
			last_f_gametic = (int)f_gametic;
			MenuAnimationTic = (MenuAnimationTic + 1) & 7;
		}
	}

	text_alpha += text_alpha_change_value;
	if (text_alpha_change_value < 0) {
		if (text_alpha < 0) {
			text_alpha = 0;
			return 8;
		}
	} else {
		if ((text_alpha_change_value > 0) && (text_alpha >= 256)) {
			text_alpha = 255;
			return 8;
		}
	}

	return 0;
}

void M_FadeInStart(void)
{
	MiniLoop(M_AlphaInStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_FadeOutStart(int exitmode)
{
	if (exitmode == 8)
		MiniLoop(M_AlphaOutStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_SaveMenuData(void)
{
	menudata_t *mdat;

	// Save Actual Menu Page
	mdat = &MenuData[MenuIdx];
	MenuIdx += 1;

	mdat->menu_item = MenuItem;
	mdat->item_lines = itemlines;
	mdat->menu_call = MenuCall;
	mdat->cursor_pos = cursorpos;

	// Start Menu Fade Out
	MiniLoop(M_AlphaOutStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_RestoreMenuData(boolean alpha_in)
{
	menudata_t *mdat;

	// Restore Previous Save Menu Page
	MenuIdx -= 1;
	mdat = &MenuData[MenuIdx];

	MenuItem = mdat->menu_item;
	itemlines = mdat->item_lines;
	MenuCall = mdat->menu_call;
	cursorpos = mdat->cursor_pos;
	// Start Menu Fade In
	if (alpha_in)
		MiniLoop(M_AlphaInStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_MenuGameDrawer(void)
{
	switch (DrawerStatus) {
	case 1:
		P_Drawer();
		break;

	case 2:
		F_DrawerIntermission();
		break;

	case 3:	
		F_Drawer();
		break;

	default:
		I_ClearFrame();

		M_DrawBackground(TITLE, 80);

		if (MenuItem != Menu_Title)
			M_DrawOverlay();

		MenuCall();
		I_DrawFrame();
		break;
	}
}

extern mobj_t mobjhead;
extern mapthing_t *spawnlist;
extern int spawncount;
extern int gobalcheats;

int M_MenuTicker(void)
{
	static int last_f_gametic = 0;
	unsigned int buttons, oldbuttons;
	int exit;
	int truebuttons;
	int ret;
	int i;
	mobj_t *m;

	/* animate skull */
	if (((int)f_gamevbls < (int)f_gametic) && (((int)f_gametic & 3U) == 0)) {
		if (last_f_gametic != (int)f_gametic) {
			last_f_gametic = (int)f_gametic;
			MenuAnimationTic = (MenuAnimationTic + 1) & 7;
		}
	}

	buttons = M_ButtonResponder(ticbuttons[0]);
	oldbuttons = oldticbuttons[0] & 0xffff0000;

	/* exit menu if button press */
	if (buttons != 0)
		last_ticon = ticon;

	/* exit menu if time out */
	if ((MenuItem == Menu_Title) && ((ticon - last_ticon) >= 900)) { // 30 * TICRATE
		exit = ga_timeout;
	} else {
		/* check for movement */
		if (!(buttons & (PAD_Z_TRIG | ALL_JPAD))) {
			f_m_vframe1 = 0.0f;
		} else {
			f_m_vframe1 = f_m_vframe1 - f_vblsinframe[0];
			if (f_m_vframe1 <= 0.0f) {
				f_m_vframe1 = (float)(TICRATE / 2);

				if (buttons & PAD_DOWN) {
					cursorpos += 1;

					if (cursorpos >= itemlines)
						cursorpos = 0;

					S_StartSound(NULL, sfx_switch1);
				} else if (buttons & PAD_UP) {
					cursorpos -= 1;

					if (cursorpos < 0)
						cursorpos = itemlines - 1;

					S_StartSound(NULL, sfx_switch1);
				}
			}
		}

		if ((buttons & PAD_START) && !(oldticbuttons[0] & PAD_START)) {
			if ((MenuItem == Menu_Title) ||
			    (MenuItem == Menu_ControllerPakBad) ||
			    (MenuItem == Menu_CreateNote) ||
			    (MenuItem == Menu_ControllerPakFull)) {
				return ga_nothing;
			} else {

				if (MenuIdx != 0)
					S_StartSound(NULL, sfx_pistol);

				return ga_exit;
			}
		} else {
			truebuttons = (0 < (buttons ^ oldbuttons));

			if (truebuttons)
				truebuttons = (0 < (buttons & ALL_BUTTONS));

			switch (MenuItem[cursorpos].casepos) {
			case 0: // Gamepad
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuCall = M_ControlPadDrawer;
					cursorpos = 0;
					linepos = 0;

					MiniLoop(M_FadeInStart, M_FadeOutStart, M_ControlPadTicker, M_MenuGameDrawer);
					M_RestoreMenuData(true);
					return ga_nothing;
				}
				break;

			case 1: // Volume
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Volume;
					itemlines = NUM_MENU_VOLUME;
					MenuCall = M_VolumeDrawer;
					cursorpos = 0;

					MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData(true);
					return ga_nothing;
				}
				break;

			case 2: // Video
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Video;
					itemlines = NUM_MENU_VIDEO;
					MenuCall = M_VideoDrawer;
					cursorpos = 0;

					MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData(true);
					return ga_nothing;
				}
				break;

			case 3: // Password
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					ret = I_CheckControllerPak();
					exit = ga_exit;

					if (ret == 0) {
						if (I_ReadPakFile() == 0) {
							UseVMU = 1;

							MenuCall = M_LoadPakDrawer;

							exit = MiniLoop(M_LoadPakStart, M_LoadPakStop, M_LoadPakTicker, M_MenuGameDrawer);
						} else
							exit = ga_exit;
					}

					if (exit == ga_exit) {
						MenuCall = M_PasswordDrawer;

						exit = MiniLoop(M_PasswordStart, M_PasswordStop, M_PasswordTicker, M_MenuGameDrawer);
					}

					if (exit == ga_exit) {
						M_RestoreMenuData(true);
						return ga_nothing;
					}

					if (UseVMU != 0) {
						return exit;
					}

					UseVMU = (M_ControllerPak() == 0);
					return exit;
				}
				break;

			case 4: // Main Menu
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Quit;
					itemlines = NUM_MENU_QUIT;
					MenuCall = M_MenuTitleDrawer;
					cursorpos = 1;

					exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData((exit == ga_exit));

					// have to exit eventually, good enough place to hook this
					I_SavePakSettings(&menu_settings);

					if (exit == ga_exit) {
						return ga_nothing;
					}

					return ga_exitdemo;
				}
				break;

			case 5: // [Immorpher] Updated Restart Level
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Skill;
					itemlines = NUM_MENU_SKILL;
					MenuCall = M_MenuTitleDrawer;
					cursorpos = gameskill; // Set default to current difficulty

					exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);

					// [Immorpher] 5th to exit menu
					if (exit == ga_exit && cursorpos == 5) { 
						M_RestoreMenuData((exit == ga_exit));
						return ga_nothing;
					}

					gameskill = cursorpos;

					startmap = gamemap;
					startskill = gameskill;
					G_InitSkill(gameskill); // [Immorpher] initialize new skill

					return ga_warped;
				}
				break;

			case 6: // Return
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					// have to exit eventually, good enough place to hook this
					I_SavePakSettings(&menu_settings);
					return ga_exit;
				}
				break;

			case 7: // Music Volume
				if (buttons & PAD_RIGHT) {
					menu_settings.MusVolume += 1;
					if (menu_settings.MusVolume <= 100) {
						S_SetMusicVolume(menu_settings.MusVolume);
						if (menu_settings.MusVolume & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					} else {
						menu_settings.MusVolume = 100;
					}
				} else if (buttons & PAD_LEFT) {
					menu_settings.MusVolume -= 1;
					if (menu_settings.MusVolume < 0) {
						menu_settings.MusVolume = 0;
					} else {
						S_SetMusicVolume(menu_settings.MusVolume);
						if (menu_settings.MusVolume & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					}
				}
				break;

			case 8: // Sound Volume
				if (buttons & PAD_RIGHT) {
					menu_settings.SfxVolume += 1;
					if (menu_settings.SfxVolume <= 75) {
						S_SetSoundVolume(menu_settings.SfxVolume);
						if (menu_settings.SfxVolume & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					} else {
						menu_settings.SfxVolume = 75;
					}
				} else if (buttons & PAD_LEFT) {
					menu_settings.SfxVolume -= 1;
					if (menu_settings.SfxVolume < 0) {
						menu_settings.SfxVolume = 0;
					} else {
						S_SetSoundVolume(menu_settings.SfxVolume);
						if (menu_settings.SfxVolume & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					}
				}
				break;

			case 9: // Brightness
				if (buttons & PAD_RIGHT) {
					menu_settings.brightness += 1;
					if (menu_settings.brightness <= MAX_BRIGHTNESS) {
						P_RefreshBrightness();
						if (menu_settings.brightness & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					} else {
						menu_settings.brightness = MAX_BRIGHTNESS;
					}
				} else if (buttons & PAD_LEFT) {
					menu_settings.brightness -= 1;
					if (menu_settings.brightness < 0) {
						menu_settings.brightness = 0;
					} else {
						P_RefreshBrightness();
						if (menu_settings.brightness & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					}
				}
				break;

			case 11: // Options
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Options;
					itemlines = NUM_MENU_OPTIONS;
					MenuCall = M_MenuTitleDrawer;
					cursorpos = 0;

					exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData((exit == ga_exit));

					// have to exit eventually, good enough place to hook this
					I_SavePakSettings(&menu_settings);

					if (exit == ga_exit)
						return ga_nothing;

					return exit;
				}
				break;

			case 12: // Autorun
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					menu_settings.Autorun ^= true;
					return ga_nothing;
				}
				break;

			case 14: // New Game
				if (truebuttons)
				{
					// Check ControllerPak
                    UseVMU = (M_ControllerPak() == 0);

					if (extra_episodes) {
						S_StartSound(NULL, sfx_pistol);
						M_SaveMenuData();

						if (extra_episodes == 1) {
							MenuItem = Menu_Episode;
							itemlines = NUM_MENU_EPISODES;
						} else {
							MenuItem = Menu_2Episode;
							itemlines = NUM_MENU_2EPISODES;
						}
						MenuCall = M_MenuTitleDrawer;
						cursorpos = 0;

						exit = MiniLoop(M_FadeInStart, M_MenuClearCall, M_MenuTicker, M_MenuGameDrawer);
						M_RestoreMenuData((exit == ga_exit));

						if (exit == ga_exit)
							return ga_nothing;

						return exit;
					} else {
						startmap = 1;

						S_StartSound(NULL, sfx_pistol);
						M_SaveMenuData();

						MenuItem = Menu_Skill;
						itemlines = NUM_MENU_SKILL;
						MenuCall = M_MenuTitleDrawer;
						cursorpos = 2;

						exit = MiniLoop(M_FadeInStart, M_MenuClearCall, M_MenuTicker, M_MenuGameDrawer);
						M_RestoreMenuData((exit == ga_exit));

						if (exit == ga_exit)
							return ga_nothing;

						nextmap = 1; // [Immorpher] For running introduction for Doom 64
						menu_settings.runintroduction = true; // [Immorpher] turn introduction on

						return exit;
					}
				}
				break;

			case 15: // Be Gentle!
			case 16: // Bring it on!
			case 17: // I own Doom!
			case 18: // Watch me die!
			case 19: // Be merciless!
				if (truebuttons) {
						startskill = MenuItem[cursorpos].casepos - 15;
						S_StartSound(NULL, sfx_pistol);
						return ga_restart;
				}
				break;

			case 20: // Yes
			case 46: // Try again
			case 47: // Create game note
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					return 5; //ga_exitdemo;
				}
				break;

			case 21: // No
			case 45: // Do not use Pak
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					return ga_exit;
				}
				break;

			case 22: // Features
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					players[0].cheats &= 0xffff1fff;

					MenuItem = Menu_Features;
					itemlines = NUM_MENU_FEATURES;
					MenuCall = M_FeaturesDrawer;
					cursorpos = 0;
					m_actualmap = gamemap;

					exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData((exit == 8));

					if (exit == 8)
						return ga_nothing;

					return exit;
				}
				break;

			case 23: // WARP TO LEVEL
				if (buttons ^ oldbuttons) {
					if (buttons & PAD_LEFT) {
						m_actualmap -= 1;

						if (kneedeep_only) {
							if (m_actualmap > 33 && m_actualmap < 41) {
								m_actualmap = 33;
							}
						}


						if (m_actualmap < 1) {
							m_actualmap = 1;
						} 
						return ga_nothing;
					} else if (buttons & PAD_RIGHT) {
						m_actualmap += 1;

						if (kneedeep_only) {
							if (m_actualmap > 33 && m_actualmap < 41) {
								m_actualmap = 41;
							}
						}

						if (extra_episodes) {
							if (m_actualmap > 49) {
								m_actualmap = 49;
							}
						} else if (m_actualmap > ABS_LASTLEVEL) {
							m_actualmap = ABS_LASTLEVEL;
						}

						return ga_nothing;
					} else if (truebuttons) {
						gamemap = m_actualmap;
						startmap = m_actualmap;
						return ga_warped;
					}
				}
				break;

			case 24: // INVULNERABLE
				if (((gamemap != 32) & truebuttons)) {
					S_StartSound(NULL, sfx_switch2);
					players[0].cheats ^= CF_GODMODE;
					return ga_nothing;
				}
				break;

			case 25: // HEALTH BOOST
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					players[0].cheats |= CF_HEALTH;
					players[0].health = 100;
					players[0].mo->health = 100;
					return ga_nothing;
				}
				break;

			case 26: // SECURITY KEYS
				/* Not available in the release code */
				/*
                    Reconstructed code based on Psx Doom
                    */
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					players[0].cheats |= CF_ALLKEYS;

					for (m = mobjhead.next; m != &mobjhead;
					     m = m->next) {
						switch (m->type) {
						case MT_ITEM_BLUECARDKEY:
							players[0].cards[it_bluecard] = true;
							break;
						case MT_ITEM_REDCARDKEY:
							players[0].cards[it_redcard] = true;
							break;
						case MT_ITEM_YELLOWCARDKEY:
							players[0].cards[it_yellowcard] = true;
							break;
						case MT_ITEM_YELLOWSKULLKEY:
							players[0].cards[it_yellowskull] = true;
							break;
						case MT_ITEM_REDSKULLKEY:
							players[0].cards[it_redskull] = true;
							break;
						case MT_ITEM_BLUESKULLKEY:
							players[0].cards[it_blueskull] = true;
							break;
						default:
							break;
						}
					}

					for (i = 0; i < spawncount; i++) {
						switch (spawnlist[i].type) {
						case 5:
							players[0].cards[it_bluecard] = true;
							break;
						case 13:
							players[0].cards[it_redcard] = true;
							break;
						case 6:
							players[0].cards[it_yellowcard] = true;
							break;
						case 39:
							players[0].cards[it_yellowskull] = true;
							break;
						case 38:
							players[0].cards[it_redskull] = true;
							break;
						case 40:
							players[0].cards[it_blueskull] = true;
							break;
						default:
							break;
						}
					}

					return ga_nothing;
				}
				break;

			case 27: // WEAPONS
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					players[0].cheats |= CF_WEAPONS;

					for (i = 0; i < NUMWEAPONS; i++)
						players[0].weaponowned[i] = true;

					// [jnmartin84] give backpack too
					for (i = 0; i < NUMAMMO; i++) {
						if (!players[0].backpack)
							players[0].maxammo[i] *= 2;

						players[0].ammo[i] = players[0].maxammo[i];
					}

					players[0].backpack = true;

					return ga_nothing;
				}
				break;

			case 28: // Exit
				/* nothing special */
				break;

			case 31: // WALL BLOCKING
				/* Not available in the release code */
				/*
                    In my opinion it must have been the NOCLIP cheat code
                    */
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					players[0].cheats ^= CF_WALLBLOCKING;
					players[0].mo->flags ^= MF_NOCLIP;
					return ga_nothing;
				}
				break;

			case 33: // Messages
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					menu_settings.enable_messages ^= true;
					return ga_nothing;
				}
				break;

			case 34: // [Immorpher] HUD opacity
				if (buttons & PAD_RIGHT) {
					menu_settings.HUDopacity += 4;
					if (menu_settings.HUDopacity <= 255) {
						if (menu_settings.HUDopacity & 4) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					} else {
						menu_settings.HUDopacity = 255;
					}
				} else if (buttons & PAD_LEFT) {
					menu_settings.HUDopacity -= 4;
					if (menu_settings.HUDopacity < 0) {
						menu_settings.HUDopacity = 0;
					} else {
						if (menu_settings.HUDopacity & 4) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					}
				}
				break;

			case 35: // LOCK MONSTERS
				/* Not available in the release code */
				/*
                    Reconstructed code based on Doom 64 Ex
                    */
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					players[0].cheats ^= CF_LOCKMOSTERS;
					return ga_nothing;
				}
				break;

			case 37: // MAP EVERYTHING
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					players[0].cheats ^= CF_ALLMAP;
					return ga_nothing;
				}
				break;

			case 101:
				if (truebuttons) {
					Wireframe = !Wireframe;
					return ga_nothing;
				}

			case 39: // MUSIC TEST
				/* Not available in the release code */
				/*
                    Reconstructed code in my interpretation
                    */
				if (buttons ^ oldbuttons) {
					if (buttons & PAD_LEFT) {
						MusicID -= 1;
						if (MusicID > 0) {
							S_StartSound(NULL, sfx_switch2);
							return ga_nothing;
						}
						MusicID = 1;
					} else if (buttons & PAD_RIGHT) {
						MusicID += 1;
						if (MusicID < 25) {
							S_StartSound(NULL, sfx_switch2);
							return ga_nothing;
						}
						MusicID = 24;
					} else if (truebuttons) {
						S_StopMusic();
						from_menu = 1;
						S_StartMusic(MusicID + 92);
						from_menu = 0;
						return ga_nothing;
					}
				}
				break;

			case 41: // Control Stick
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Movement;
					itemlines = NUM_MENU_MOVEMENT;
					MenuCall = M_MovementDrawer;
					cursorpos = 0;

					MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData(true);

					return ga_nothing;
				}
				break;

			case 43: // Sensitivity
				if (buttons & PAD_RIGHT) {
					menu_settings.M_SENSITIVITY += 1;
					if (menu_settings.M_SENSITIVITY <= 127) {
						if (menu_settings.M_SENSITIVITY & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					} else {
						menu_settings.M_SENSITIVITY = 127;
					}
				} else if (buttons & PAD_LEFT) {
					menu_settings.M_SENSITIVITY -= 1;
					if (menu_settings.M_SENSITIVITY < 0) {
						menu_settings.M_SENSITIVITY = 0;
					} else {
						if (menu_settings.M_SENSITIVITY & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					}
				}
				break;

			case 44: // Manage Pak
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuCall = M_ControllerPakDrawer;
					linepos = 0;
					cursorpos = 0;

					exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_ScreenTicker, M_MenuGameDrawer);
					M_RestoreMenuData((exit == ga_exit));

					if (exit == ga_exit)
						return ga_nothing;

					return exit;
				}
				break;

			case 50: // [GEC and Immorpher] Video filtering mode
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					if (menu_settings.VideoFilter == M_FILTER_BILINEAR) {
						menu_settings.VideoFilter = M_FILTER_NONE;
					} else {
						menu_settings.VideoFilter = M_FILTER_BILINEAR;
					}
					force_filter_flush = 1;
					return ga_nothing;
				}
				break;

			case 51: // Display
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Display;
					itemlines = NUM_MENU_DISPLAY;
					MenuCall = M_DisplayDrawer;
					cursorpos = 0;

					MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData(true);
					return ga_nothing;
				}
				break;

			case 52: // Motion Bob
				if (buttons & PAD_RIGHT) {
					menu_settings.MotionBob += 0x8000; // increments 1/2 fixed point

					if (menu_settings.MotionBob <= 0x100000) { // Maximum is 16 fixed point
						if (menu_settings.MotionBob & 0x8000) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					} else {
						menu_settings.MotionBob = 0x100000; // The Limit
					}
				} else if (buttons & PAD_LEFT) {
					menu_settings.MotionBob -= 0x8000; // decrements 1/2 fixed point
					if (menu_settings.MotionBob < 0x0) {
						menu_settings.MotionBob = 0x0;
					} else {
						if (menu_settings.MotionBob & 0x8000) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					}
				}
				break;

			case 61: // [Immorpher] Story Text
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					menu_settings.StoryText ^= true;
					return ga_nothing;
				}
				break;

			case 62: // [Immorpher] Map stats
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					menu_settings.MapStats ^= true;
					return ga_nothing;
				}
				break;

			case 63: // [Immorpher] Status HUD
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_StatusHUD;
					itemlines = NUM_MENU_STATUSHUD;
					MenuCall = M_StatusHUDDrawer;
					cursorpos = 0;

					MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData(true);
					return ga_nothing;
				}
				break;

			case 64: // HUDmargin
				if (buttons & PAD_RIGHT) {
					menu_settings.HUDmargin += 1; // increments

					if (menu_settings.HUDmargin <= 20) { // Maximum is 20
						if (menu_settings.HUDmargin & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					} else {
						menu_settings.HUDmargin = 20; // The Limit
					}
				} else if (buttons & PAD_LEFT) {
					menu_settings.HUDmargin -= 1; // decrements

					if (menu_settings.HUDmargin < 0) {
						menu_settings.HUDmargin = 0;
					} else {
						if (menu_settings.HUDmargin & 1) {
							S_StartSound(NULL, sfx_secmove);
							return ga_nothing;
						}
					}
				}
				break;

			case 67: // [Immorpher] Colored HUD
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					menu_settings.ColoredHUD ^= true;
					return ga_nothing;
				}
				break;

			case 69: // Credits
				if (truebuttons) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Doom64DC_Credits;
					itemlines = NUM_DOOM64DC_CREDITS;
					MenuCall = M_CreditsDrawer;
					cursorpos = 0;

					exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData((exit == ga_exit));

					if (exit == ga_exit)
						return ga_nothing;

					return exit;
				}
				break;

			case 85:
				if (truebuttons) {
					startmap = 1;

					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Skill;
					itemlines = NUM_MENU_SKILL;
					MenuCall = M_MenuTitleDrawer;
					cursorpos = 2;

					exit = MiniLoop(M_FadeInStart, M_MenuClearCall, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData((exit == ga_exit));

					if (exit == ga_exit)
						return ga_nothing;

					nextmap = 1; // [Immorpher] For running introduction for Doom 64
					menu_settings.runintroduction = true; // [Immorpher] turn introduction on

					return exit;
				}
				break;

			case 86:
				if (truebuttons) {
					startmap = 34;

					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Skill;
					itemlines = NUM_MENU_SKILL;
					MenuCall = M_MenuTitleDrawer;
					cursorpos = 2;

					exit = MiniLoop(M_FadeInStart, M_MenuClearCall, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData((exit == ga_exit));

					if (exit == ga_exit)
						return ga_nothing;

					nextmap = LOSTLEVEL; // [Immorpher] For running introduction for Lost Levels
					menu_settings.runintroduction = true; // [Immorpher] turn introduction on

					return exit;
				}
				break;

			case 87:
				if (truebuttons) {
					startmap = 41;

					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_Skill;
					itemlines = NUM_MENU_SKILL;
					MenuCall = M_MenuTitleDrawer;
					cursorpos = 2;

					exit = MiniLoop(M_FadeInStart, M_MenuClearCall, M_MenuTicker, M_MenuGameDrawer);
					M_RestoreMenuData((exit == ga_exit));

					if (exit == ga_exit)
						return ga_nothing;

					nextmap = 41; // [Immorpher] For running introduction for Lost Levels
					menu_settings.runintroduction = true; // [Immorpher] turn introduction on

					return exit;
				}
				break;

			case 88: // Quality mode
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					global_render_state.quality = (global_render_state.quality + 1) % NUM_QUALITY;
					menu_settings.Quality = (menu_settings.Quality + 1) % NUM_QUALITY;
					global_render_state.context_change = 1;
					return ga_nothing;
				}
				break;

			case 92: // FPS cap
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					global_render_state.fps_uncap = !global_render_state.fps_uncap;
					menu_settings.FpsUncap = !menu_settings.FpsUncap;
					return ga_nothing;
				}
				break;

			case 95:
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					menu_settings.Rumble = (menu_settings.Rumble + 1) % NUM_RUMBLEPAKS;
					return ga_nothing;
				}
				break;

			case 96: // Interpolate
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					menu_settings.Interpolate = !menu_settings.Interpolate;
					return ga_nothing;
				}
				break;

			case 97: // VMU Display mode
				if (truebuttons) {
					S_StartSound(NULL, sfx_switch2);
					menu_settings.VmuDisplay = (menu_settings.VmuDisplay + 1) % 3;
					force_vmu_refresh = 1;
					return ga_nothing;
				}
				break;
			}
			exit = ga_nothing;
		}
	}
	return exit;
}

void M_MenuClearCall(int exit)
{
	(void)exit;
	MenuCall = NULL;
}

void M_MenuTitleDrawer(void)
{
	menuitem_t *item = MenuItem;
	int i;
	if (MenuItem == Menu_Game) {
		ST_DrawString(-1, 20, "Pause", text_alpha | 0xc0000000, ST_ABOVE_OVL);
		ST_DrawString(-1, 200, "press \x8d to resume", text_alpha | 0xffffff00, ST_ABOVE_OVL);
	} else if (MenuItem == Menu_Skill) {
		ST_DrawString(-1, 20, "Choose Your Skill...", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	} else if (MenuItem == Menu_Options) {
		ST_DrawString(-1, 20, "Options", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	} else if (MenuItem == Menu_Quit) {
		ST_DrawString(-1, 20, "Quit Game?", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	} else if (MenuItem == Menu_DeleteNote) {
		ST_DrawString(-1, 20, "Delete Game Note?", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	} else if (MenuItem == Menu_ControllerPakBad) {
		ST_DrawString(-1, 20, "VMU Bad", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	} else if (MenuItem == Menu_ControllerPakFull) {
		ST_DrawString(-1, 20, "VMU Full", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	} else if (MenuItem == Menu_CreateNote) {
		ST_DrawString(-1, 20, "Create Game Note?", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	} else if (MenuItem == Menu_Episode) {
		ST_DrawString(-1, 20, "Choose Campaign", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	} else if (MenuItem == Menu_2Episode) {
		ST_DrawString(-1, 20, "Choose Campaign", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	}

	for (i = 0; i < itemlines; i++) {
		ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000, ST_ABOVE_OVL);
		item++;
	}

	ST_DrawSymbol(MenuItem[0].x - 37, MenuItem[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00, ST_ABOVE_OVL);
}

#define MT_WARPTOFUN 40
#define MT_WARPTOMOTHER 65
#define MT_WARPTOSECRET 66
#define MT_WARPTOLEVEL 23

void M_FeaturesDrawer(void)
{
	char *text;
	menuitem_t *item = MenuItem;
	int i;

	memset(textbuff, 0, 256);

	ST_DrawString(-1, 20, "Features", text_alpha | 0xc0000000, ST_ABOVE_OVL);

	for (i = 0; i < itemlines; i++) {
		if ((item->casepos == 23) && FUNLEVEL(m_actualmap))
			ST_Message(item->x, item->y, MenuText[MT_WARPTOFUN], text_alpha | 0xffffff00, ST_ABOVE_OVL);
		else if ((item->casepos == 23) && (m_actualmap == 28))
			ST_Message(item->x, item->y, MenuText[MT_WARPTOMOTHER], text_alpha | 0xffffff00, ST_ABOVE_OVL);
		else if ((item->casepos == 23) && (m_actualmap > 28) && (m_actualmap < 34))
			ST_Message(item->x, item->y, MenuText[MT_WARPTOSECRET], text_alpha | 0xffffff00, ST_ABOVE_OVL);
		else if (item->casepos == 23)
			ST_Message(item->x, item->y, MenuText[MT_WARPTOLEVEL], text_alpha | 0xffffff00, ST_ABOVE_OVL);
		else
			ST_Message(item->x, item->y, MenuText[item->casepos], text_alpha | 0xffffff00, ST_ABOVE_OVL);

		text = textbuff;
		switch (item->casepos) {
		case 23: /* WARP TO LEVEL */
			sprintf(textbuff, "%s", MapInfo[m_actualmap].name);
			break;
		case 24: /* INVULNERABLE */
			text = (!(players[0].cheats & CF_GODMODE)) ? "OFF" : "ON";
			break;
		case 25: /* HEALTH BOOST */
			text = (!(players[0].cheats & CF_HEALTH)) ? "-" : "100%";
			break;
		case 26: /* SECURITY KEYS */
			text = (!(players[0].cheats & CF_ALLKEYS)) ? "-" : "100%";
			break;
		case 27: /* WEAPONS */
			text = (!(players[0].cheats & CF_WEAPONS)) ? "-" : "100%";
			break;
		case 28: /* Exit */
			break;
		case 31: /* WALL BLOCKING */
			text = (!(players[0].cheats & CF_WALLBLOCKING)) ? "ON" : "OFF";
			break;
		case 35: /* LOCK MONSTERS */
			text = (!(players[0].cheats & CF_LOCKMOSTERS)) ? "OFF" : "ON";
			break;
		case 37: /* MAP EVERYTHING */
			text = (!(players[0].cheats & CF_ALLMAP)) ? "OFF" : "ON";
			break;
		case 39: /* MUSIC TEST */
			sprintf(textbuff, "%d", MusicID);
			break;
		case 101:
			text = Wireframe ? "ON" : "OFF";
			break;
		default:
			text = ""; // [Immorpher] set to null for credits menu
			break;
		}

		ST_Message(item->x + 130, item->y, text, text_alpha | 0xffffff00, ST_ABOVE_OVL);
		item++;
	}

	ST_DrawSymbol(MenuItem->x - 10, MenuItem[cursorpos].y - 1, 78, text_alpha | 0xffffff00, ST_ABOVE_OVL);
}

void M_CreditsDrawer(void)
{
	char *text;
	menuitem_t *item;
	int i;

	memset(textbuff, 0, 256);

	ST_DrawString(-1, 20, "DOOM 64 DC Credits", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	item = MenuItem;

	for (i = 0; i < itemlines; i++) {
		ST_Message(item->x, item->y, MenuText[item->casepos], text_alpha | 0xffffff00, ST_ABOVE_OVL);
		text = textbuff;
		ST_Message(item->x + 130, item->y, text, text_alpha | 0xffffff00, ST_ABOVE_OVL);
		item++;
	}
}

void M_VolumeDrawer(void)
{
	menuitem_t *item;
	int i;

	ST_DrawString(-1, 20, "Volume", text_alpha | 0xc0000000, ST_ABOVE_OVL);
	item = Menu_Volume;

	for (i = 0; i < itemlines; i++) {
		ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000, ST_ABOVE_OVL);
		item++;
	}

	ST_DrawSymbol(MenuItem->x - 37, MenuItem[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00, ST_ABOVE_OVL);

	ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00, ST_ABOVE_OVL);
	ST_DrawSymbol(menu_settings.MusVolume + 83, 80, 69, text_alpha | 0xffffff00, ST_ABOVE_OVL);

	ST_DrawSymbol(82, 120, 68, text_alpha | 0xffffff00, ST_ABOVE_OVL);
	ST_DrawSymbol(((menu_settings.SfxVolume*100)/75) + 83, 120, 69, text_alpha | 0xffffff00, ST_ABOVE_OVL);
}

void M_MovementDrawer(void)
{
	char *text = NULL;
	menuitem_t *item = Menu_Movement;
	int i, casepos;

	ST_DrawString(-1, 20, "Movement", text_alpha | 0xc0000000, ST_ABOVE_OVL);

	for (i = 0; i < itemlines; i++) {
		casepos = item->casepos;

		if (casepos == 12) {
			if (menu_settings.Autorun)
				text = "On";
			else
				text = "Off";
		} else if (casepos == 95) {
			if (menu_settings.Rumble == 0)
				text = "Off";
			else if (menu_settings.Rumble == 1)
				text = M_TXT98;
			else if (menu_settings.Rumble == 2)
				text = M_TXT99;	
			else if (menu_settings.Rumble == 3)
				text = M_TXT100;	
			else
				text = "?";
		} else {
			text = NULL;
		}

		if (text)
			ST_DrawString(item->x + 100, item->y, text, text_alpha | 0xc0000000, ST_ABOVE_OVL);

		ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000, ST_ABOVE_OVL);
		item++;
	}

	ST_DrawSymbol(MenuItem->x - 37, MenuItem[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00, ST_ABOVE_OVL);

	// Sensitivity
	ST_DrawSymbol(82, 120, 68, text_alpha | 0xffffff00, ST_ABOVE_OVL);
	ST_DrawSymbol(((101 * menu_settings.M_SENSITIVITY) >> 7) + 83, 120, 69, text_alpha | 0xffffff00, ST_ABOVE_OVL);

	// Motion bob
	ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00, ST_ABOVE_OVL);
	ST_DrawSymbol((menu_settings.MotionBob / 0x28F6) + 83, 80, 69, text_alpha | 0xffffff00, ST_ABOVE_OVL);
}

void M_VideoDrawer(void)
{
	char *text = NULL;
	menuitem_t *item = Menu_Video;
	int i, casepos;

	ST_DrawString(-1, 20, "Video", text_alpha | 0xc0000000, ST_ABOVE_OVL);

	for (i = 0; i < NUM_MENU_VIDEO; i++) {
		casepos = item->casepos;

		if (casepos == 96) { // interpolate
			text = menu_settings.Interpolate ? "On" : "Off";
		} else if (casepos == 92) { // fps cap menu
			text = (global_render_state.fps_uncap == 0) ? M_TXT93 : M_TXT94;
		} else if (casepos == 88) { // quality menu
			if (global_render_state.quality == q_low)
				text = M_TXT89;
			else if (global_render_state.quality == q_medium)
				text = M_TXT90;
			else if (global_render_state.quality == q_ultra)
				text = M_TXT91;
		} else if (casepos == 50) { // [GEC and Immorpher] New video filter
			text = (menu_settings.VideoFilter == M_FILTER_BILINEAR) ? "On" : "Off";
		} else {
			text = NULL;
		}

		if (text)
			ST_DrawString(item->x + 140, item->y, text, text_alpha | 0xc0000000, ST_ABOVE_OVL);

		ST_DrawString(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000, ST_ABOVE_OVL);

		item++;
	}

	ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00, ST_ABOVE_OVL);
	ST_DrawSymbol(((101 * menu_settings.brightness) >> 7) + 83, 80, 69, text_alpha | 0xffffff00, ST_ABOVE_OVL);

	ST_DrawSymbol(Menu_Video[0].x - 37, Menu_Video[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00, ST_ABOVE_OVL);
}

void M_DisplayDrawer(void)
{
	char *text = NULL;
	menuitem_t *item = Menu_Display;
	int i, casepos;

	ST_DrawString(-1, 20, "Display", text_alpha | 0xc0000000, ST_ABOVE_OVL);

	for (i = 0; i < itemlines; i++) {
		casepos = item->casepos;

		if (casepos == 61) { // Story Text:
			text = menu_settings.StoryText ? "On" : "Off";
		} else if (casepos == 62) { // Map stats:
			text = menu_settings.MapStats ? "On" : "Off";
		} else if (casepos == 97) { // vmu
			if (menu_settings.VmuDisplay == 0)
				text = "Off";
			else if (menu_settings.VmuDisplay == 1)
				text = "Face";
			else if (menu_settings.VmuDisplay == 2)
				text = "Stats Face";
			else
				text = NULL;
		} else {
			text = NULL;
		}

		if (text)
			ST_DrawString(item->x + 130, item->y, text, text_alpha | 0xc0000000, ST_ABOVE_OVL);

		ST_DrawString(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000, ST_ABOVE_OVL);

		item++;
	}

	ST_DrawSymbol(Menu_Display[0].x - 37, Menu_Display[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00, ST_ABOVE_OVL);
}

void M_StatusHUDDrawer(void)
{
	char *text = NULL;
	menuitem_t *item = Menu_StatusHUD;
	int i, casepos;

	ST_DrawString(-1, 20, "Status HUD", text_alpha | 0xc0000000, ST_ABOVE_OVL);

	for (i = 0; i < itemlines; i++) {
		casepos = item->casepos;

		if (casepos == 33) // Messages:
			text = menu_settings.enable_messages ? "On" : "Off";
		else if (casepos == 67) // Colored HUD:
			text = menu_settings.ColoredHUD ? "On" : "Off";
		else
			text = NULL;

		if (text)
			ST_DrawString(item->x + 130, item->y, text, text_alpha | 0xc0000000, ST_ABOVE_OVL);

		ST_DrawString(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000, ST_ABOVE_OVL);

		item++;
	}

	// HUD Margin
	ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00, ST_ABOVE_OVL);
	ST_DrawSymbol(100 * menu_settings.HUDmargin / 20 + 83, 80, 69, text_alpha | 0xffffff00, ST_ABOVE_OVL);

	// HUD Opacity
	ST_DrawSymbol(82, 120, 68, text_alpha | 0xffffff00,1);
	ST_DrawSymbol(((100 * menu_settings.HUDopacity) / 255) + 83, 120, 69, text_alpha | 0xffffff00, ST_ABOVE_OVL);

	ST_DrawSymbol(Menu_StatusHUD[0].x - 37, Menu_StatusHUD[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00, ST_ABOVE_OVL);
}

uint16_t bgpal[256];
uint16_t biggest_bg[512 * 256];
uint64_t lastname[2] = { 0xffffffff, 0xffffffff };
int bg_last_width[2];
int bg_last_height[2];
pvr_ptr_t pvrbg[2] = { 0, 0 };

static pvr_sprite_cxt_t bg_scxt;
pvr_sprite_hdr_t bg_shdr[2];
pvr_sprite_txr_t bg_stxr[2];

static char __attribute__((aligned(32))) bgnamebuf[8];

static d64_bg_t d64_bg[NUM_BG] = {
(d64_bg_t){27,74,0},
(d64_bg_t){68,21,0},
(d64_bg_t){32,41,1},
(d64_bg_t){22,82,0},
(d64_bg_t){29,28,1},
(d64_bg_t){63,25,0},
(d64_bg_t){0,0,0},
(d64_bg_t){56,57,2}
};

static char *d64_bg_name[NUM_BG] = {
	"USLEGAL",
	"IDCRED1",
	"IDCRED2",
	"WMSCRED1",
	"WMSCRED2",
	"EVIL",
	"FINAL",
	"TITLE"
};

void M_DrawBackground(d64_bg_enum_t bg, int alpha)
{
	int width, height;
	int offset;
	uint8_t *data;
	uint8_t *gfxsrc;
	short *palsrc;
	float u1, v1, u2, v2;
	float x1, y1, x2, y2;
	float z = 0.00015f;
	uint8_t a1 = alpha & 0xff;
	int x = d64_bg[bg].x;
	int y = d64_bg[bg].y;
	int num = d64_bg[bg].num;
	char *name = d64_bg_name[bg];

	switch (num) {
		case 0:
			z = 0.00015f;
		break;

		case 1:
			z = 0.00016f;
		break;

		case 2:
			z = 8.999999f;
			num = 0;
		break;
	}

	memset(bgnamebuf, 0, 8);
	memcpy(bgnamebuf, name, strlen(name) < 8 ? strlen(name) : 8);

	if (!pvrbg[num]) {
		pvrbg[num] = pvr_mem_malloc(512 * 512);

		if (!pvrbg[num])
			I_Error("PVR OOM for background %s [%d]", name, num);

		pvr_sprite_cxt_txr(&bg_scxt, PVR_LIST_TR_POLY, D64_TARGB, 512, 256, pvrbg[num], PVR_FILTER_NONE);
		pvr_sprite_compile(&bg_shdr[num], &bg_scxt);
	}

	//uint32_t wasnt enough to differentiate between the credit screens
	if (*(uint64_t *)(bgnamebuf) != lastname[num]) {
		lastname[num] = *(uint64_t *)bgnamebuf;
		data = (uint8_t *)W_CacheLumpName(name, PU_STATIC, dec_jag);

		width = SwapShort(((gfxN64_t *)data)->width);
		height = SwapShort(((gfxN64_t *)data)->height);
		if (height > 256)
			height = 256;
		bg_last_width[num] = width;
		bg_last_height[num] = height;
		offset = (width * height);
		offset = (offset + 7) & ~7;
		gfxsrc = data + sizeof(gfxN64_t);
		palsrc = (short *)((void *)data + offset + sizeof(gfxN64_t));

		for (int j = 0; j < 256; j++) {
			short val = SwapShort(*palsrc++);
			// Unpack and expand to 8bpp, then flip from BGR to RGB.
			uint8_t b = (val & 0x003E) << 2;
			uint8_t g = (val & 0x07C0) >> 3;
			uint8_t r = (val & 0xF800) >> 8;
			uint8_t a = 0xff; // Alpha is always 255..
			if (j == 0 && r == 0 && g == 0 && b == 0)
				bgpal[j] = get_color_argb1555(0, 0, 0, 0);
			else
				bgpal[j] = get_color_argb1555(r, g, b, a);
		}

		for (unsigned h = 0; h < height; h++)
			for (unsigned w = 0; w < width; w++)
				biggest_bg[w + (h * 512)] = bgpal[gfxsrc[w + (h * width)]];

		Z_Free(data);

		pvr_txr_load_ex(biggest_bg, pvrbg[num], 512, 256, PVR_TXRLOAD_16BPP);
	}

	u1 = 0.0f;
	v1 = 0.0f;
	u2 = (float)bg_last_width[num] * recip512;;
	v2 = (float)bg_last_height[num] * recip256;;

	bg_stxr[num].flags = PVR_CMD_VERTEX_EOL;
	bg_stxr[num].az = z;
	bg_stxr[num].bz = z;
	bg_stxr[num].cz = z;
	bg_stxr[num].dummy = 0;
	bg_shdr[num].argb = (a1 << 24) | 0x00ffffff;

	x1 = (float)(x * RES_RATIO);
	y1 = (float)(y * RES_RATIO);
	x2 = (float)((x + bg_last_width[num]) * RES_RATIO);
	y2 = (float)((y + bg_last_height[num]) * RES_RATIO);

	bg_stxr[num].ax = x1;
	bg_stxr[num].ay = y2;
	bg_stxr[num].bx = x1;
	bg_stxr[num].by = y1;
	bg_stxr[num].cx = x2;
	bg_stxr[num].cy = y1;
	bg_stxr[num].dx = x2;
	bg_stxr[num].dy = y2;
	bg_stxr[num].auv = PVR_PACK_16BIT_UV(u1, v2);
	bg_stxr[num].buv = PVR_PACK_16BIT_UV(u1, v1);
	bg_stxr[num].cuv = PVR_PACK_16BIT_UV(u2, v1);
	pvr_list_prim(PVR_LIST_TR_POLY, &bg_shdr[num], sizeof(pvr_sprite_hdr_t));
	pvr_list_prim(PVR_LIST_TR_POLY, &bg_stxr[num], sizeof(pvr_sprite_txr_t));

	globallump = -1;
	global_render_state.context_change = 1;
}

static pvr_vertex_t overlay_verts[4] = {
	{PVR_CMD_VERTEX, 0, 480, 9.9, 0, 0, 0x60000000, 0},
	{PVR_CMD_VERTEX, 0, 0, 9.9, 0, 0, 0x60000000, 0},
	{PVR_CMD_VERTEX, 640, 480, 9.9, 0, 0, 0x60000000, 0},
	{PVR_CMD_VERTEX_EOL, 640, 0, 9.9, 0, 0, 0x60000000, 0}
};
extern pvr_poly_hdr_t overlay_hdr;

// this was never called with params other than 0,0,320,240,96
// now it has no params
void M_DrawOverlay(void)
{
	pvr_list_prim(PVR_LIST_TR_POLY, &overlay_hdr, sizeof(pvr_poly_hdr_t));
	pvr_list_prim(PVR_LIST_TR_POLY, overlay_verts, sizeof(overlay_verts));
	globallump = -1;
	global_render_state.context_change = 1;
}

int M_ScreenTicker(void)
{
	static int last_f_gametic = 0;
	int exit;
	unsigned int buttons;
	unsigned int oldbuttons;

	if ((FilesUsed == -1) && (I_CheckControllerPak() == 0)) {
		cursorpos = 0;
		linepos = 0;
	}

	if (((int)f_gamevbls < (int)f_gametic) && (((int)f_gametic & 3) == 0)) {
		if (last_f_gametic != (int)f_gametic) {
			last_f_gametic = (int)f_gametic;
			MenuAnimationTic = (MenuAnimationTic + 1) & 7;
		}
	}

	buttons = M_ButtonResponder(ticbuttons[0]);
	oldbuttons = oldticbuttons[0] & 0xffff0000;

	if (!(buttons & ALL_JPAD)) {
		f_m_vframe1 = 0.0f;
	} else {
		f_m_vframe1 -= f_vblsinframe[0];

		if (f_m_vframe1 <= 0.0f) {
			f_m_vframe1 = (float)TICRATE / 2;

			if (buttons & PAD_DOWN) {
				cursorpos += 1;

				if (cursorpos < 16)
					S_StartSound(NULL, sfx_switch1);
				else
					cursorpos = 15;

				if ((linepos + 5) < cursorpos)
					linepos += 1;
			} else if (buttons & PAD_UP) {
				cursorpos -= 1;

				if (cursorpos < 0)
					cursorpos = 0;
				else
					S_StartSound(NULL, sfx_switch1);

				if (cursorpos < linepos)
					linepos -= 1;
			}
		}
	}

	if (!(buttons ^ oldbuttons) || !(buttons & PAD_START)) {
		if (buttons ^ oldbuttons) {
			if (buttons == (PAD_DREAMCAST_Y | PAD_DREAMCAST_A)) {
				dirent_t *fState = &FileState[cursorpos];

				if (fState->size != 0) {
					S_StartSound(NULL, sfx_pistol);
					M_SaveMenuData();

					MenuItem = Menu_DeleteNote;
					itemlines = NUM_MENU_DELETENOTE;
					MenuCall = M_MenuTitleDrawer;
					cursorpos = 1;
					MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);

					M_FadeOutStart(8);
					if (cursorpos == 0) {
						if (I_DeletePakFile(fState) == 0)
							fState->size = 0;
						else
							FilesUsed = -1;
					}
					M_RestoreMenuData(true);
				}
			}
		}
		exit = 0;
	} else {
		S_StartSound(NULL, sfx_pistol);
		exit = 8;
	}
	return exit;
}

static char buffer[33];
void M_ControllerPakDrawer(void)
{
	char *tmpbuf;
	int i, j;
	uint8_t idx;
	ST_DrawString(-1, 20, "VMU", text_alpha | 0xc0000000, ST_ABOVE_OVL);

	if (FilesUsed == -1) {
		if ((MenuAnimationTic & 2) != 0)
			ST_DrawString(-1, 114, "VMU removed!", text_alpha | 0xc0000000, ST_ABOVE_OVL);

		ST_DrawString(-1, 210, "press \x8d to exit", text_alpha | 0xffffff00, ST_ABOVE_OVL);
	} else {
		dirent_t *fState = &FileState[linepos];
		for (i = linepos; i < (linepos + 6); i++) {
			if (fState->size == 0) {
				strcpy(buffer, "empty");
			} else {
				tmpbuf = buffer;

				for (j = 0; j < 16; j++) {
					idx = (uint8_t)fState->name[j];
					if (idx == 0)
						break;

					tmpbuf[0] = idx;
					tmpbuf++;
				}

				*tmpbuf = '\0';
			}

			ST_DrawString(60, ((i - linepos) * 15) + 60, buffer, text_alpha | 0xc0000000, ST_ABOVE_OVL);

			fState++;
		}

		if (linepos != 0)
			ST_DrawString(60, 45, "\x8F more...", text_alpha | 0xffffff00, ST_ABOVE_OVL);

		if ((linepos + 6) < 16)
			ST_DrawString(60, 150, "\x8E more...", text_alpha | 0xffffff00, ST_ABOVE_OVL);

		sprintf(buffer, "pages used: %d   free: %ld", FileState[cursorpos].size >> 9, Pak_Memory);

		ST_DrawString(-1, 170, buffer, text_alpha | 0xc0000000, ST_ABOVE_OVL);
		ST_DrawSymbol(23, ((cursorpos - linepos) * 15) + 51, MenuAnimationTic + 70, text_alpha | 0xffffff00, ST_ABOVE_OVL);

		ST_DrawString(-1, 200, "press \x8d to exit", text_alpha | 0xffffff00, ST_ABOVE_OVL);
		ST_DrawString(-1, 215, "press \x8b\x8c to delete", text_alpha | 0xffffff00, ST_ABOVE_OVL);
	}
}

extern int32_t Pak_Size;
void M_SavePakStart(void)
{
	int i;
	int ret;
	int size;

	cursorpos = 0;
	linepos = 0;
	last_ticon = 0;

	ret = I_CheckControllerPak();
	if (ret == 0) {
		if (I_ReadPakFile() == 0) {
			size = Pak_Size / 32;

			i = 0;
			if (size != 0) {
				do
				{
					if (Pak_Data[i * 32] == 0)
						break;

					i++;
				} while (i != size);
			}

			if (i < size) {
				cursorpos = i;

				if (!(size < (i + 6)))
					linepos = i;
				else
					linepos = (size - 6);
			}
		}
	} else {
		FilesUsed = -1;
	}
}

void M_SavePakStop(int exit)
{
	(void)exit;
	S_StartSound(NULL, sfx_pistol);
	if (Pak_Data) {
		Z_Free(Pak_Data);
		Pak_Data = NULL;
	}
}

int M_SavePakTicker(void)
{
	static int last_f_gametic = 0;
	unsigned int buttons;
	unsigned int oldbuttons;
	int size;

	if (((int)f_gamevbls < (int)f_gametic) && (((int)f_gametic & 3) == 0)) {
		if (last_f_gametic != (int)f_gametic) {
			last_f_gametic = (int)f_gametic;
			MenuAnimationTic = (MenuAnimationTic + 1) & 7;
		}
	}

	buttons = M_ButtonResponder(ticbuttons[0]);
	oldbuttons = oldticbuttons[0] & 0xffff0000;

	if ((buttons != oldbuttons) && (buttons & PAD_START))
		return ga_exit;

	if (FilesUsed == -1) {
		if (I_CheckControllerPak())
			return ga_nothing;

		if (I_ReadPakFile()) {
			FilesUsed = -1;
			return ga_nothing;
		}

		cursorpos = 0;
		linepos = 0;
	}

	if (!(buttons & ALL_JPAD)) {
		f_m_vframe1 = 0;
	} else {
		f_m_vframe1 -= f_vblsinframe[0];

		if (f_m_vframe1 <= 0) {
			f_m_vframe1 = (float)(TICRATE / 2);

			if (buttons & PAD_DOWN) {
				cursorpos += 1;

				size = (Pak_Size / 32) - 1;

				if (size < cursorpos)
					cursorpos = size;
				else
					S_StartSound(NULL, sfx_switch1);

				if ((linepos + 5) < cursorpos)
					linepos += 1;
			} else if (buttons & PAD_UP) {
				cursorpos -= 1;

				if (cursorpos < 0)
					cursorpos = 0;
				else
					S_StartSound(NULL, sfx_switch1);

				if (cursorpos < linepos)
					linepos -= 1;
			}
		}
	}

	if (last_ticon == 0) {
		// press Dreamcast A button to save
		if ((buttons != oldbuttons) && (buttons == PAD_Z_TRIG)) {
			// clear the entry before putting any data in it
			memset((char *)&Pak_Data[cursorpos * 32], 0, 32);

			// save the next level number and password data in text format
			if (gameskill == sk_baby)
				sprintf((char *)&Pak_Data[cursorpos * 32], "level %2.2d - bg", nextmap);
			else if (gameskill == sk_easy)
				sprintf((char *)&Pak_Data[cursorpos * 32], "level %2.2d - bio", nextmap);
			else if (gameskill == sk_medium)
				sprintf((char *)&Pak_Data[cursorpos * 32], "level %2.2d - iod", nextmap);
			else if (gameskill == sk_hard)
				sprintf((char *)&Pak_Data[cursorpos * 32], "level %2.2d - wmd", nextmap);
			else if (gameskill == sk_nightmare)
				sprintf((char *)&Pak_Data[cursorpos * 32], "level %2.2d - bm", nextmap);
			else
				sprintf((char *)&Pak_Data[cursorpos * 32], "level %2.2d", nextmap);

			memcpy((char *)&Pak_Data[(cursorpos * 32) + 16], (char *)&Passwordbuff, 16);

			if (I_SavePakFile() == 0) {
				last_ticon = ticon;
			} else {
				FilesUsed = -1;
				if (Pak_Data) {
					Z_Free(Pak_Data);
					Pak_Data = NULL;
				}
			}
		}
	} else if ((ticon - last_ticon) >= 60) { // 2 * TICRATE
		return ga_exit;
	}

	return ga_nothing;
}

void M_SavePakDrawer(void)
{
	int i;

	I_ClearFrame();
	// Fill borders with black
	pvr_set_bg_color(0, 0, 0);
	pvr_fog_table_color(0.0f, 0.0f, 0.0f, 0.0f);
	M_DrawBackground(EVIL, 128);

	ST_DrawString(-1, 20, "VMU", text_alpha | 0xc0000000, ST_ABOVE_OVL);

	if (FilesUsed == -1) {
		if (MenuAnimationTic & 2) {
			ST_DrawString(-1, 100, "VMU removed!", 0xc00000ff, ST_ABOVE_OVL);
			ST_DrawString(-1, 120, "Game cannot be saved.", 0xc00000ff, ST_ABOVE_OVL);
		}

		ST_DrawString(-1, 210, "press \x8d to exit", text_alpha | 0xffffff00, ST_ABOVE_OVL);
	} else {
		for (i = linepos; i < (linepos + 6); i++) {
			memset(buffer, 0, 33);
			if (Pak_Data[i * 32] == 0) {
				memcpy(buffer, "empty", 5);
			} else {
				memcpy(buffer, (char *)&Pak_Data[i * 32], 32);
			}

			ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xc0000000, ST_ABOVE_OVL);
		}

		if (linepos != 0)
			ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00, ST_ABOVE_OVL);

		if ((linepos + 6) <= ((Pak_Size >> 5) - 1))
			ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00, ST_ABOVE_OVL);

		ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00, ST_ABOVE_OVL);

		ST_DrawString(-1, 195, "press \x8d to exit", text_alpha | 0xffffff00, ST_ABOVE_OVL);
		// changed to z trigger (A button)
		ST_DrawString(-1, 210, "press \x8c to save", text_alpha | 0xffffff00, ST_ABOVE_OVL);
	}

	I_DrawFrame();
}

void M_LoadPakStart(void)
{
	int i;
	int size;

	cursorpos = 0;
	linepos = 0;

	size = Pak_Size / 32;

	i = 0;
	if (size != 0) {
		do
		{
			if (Pak_Data[i * 32])
				break;

			i++;
		} while (i != size);
	}

	if (i < size) {
		cursorpos = i;

		if (!(size < (i + 6)))
			linepos = i;
		else
			linepos = (size - 6);
	}

	M_FadeInStart();
}

void M_LoadPakStop(int exit)
{
	(void)exit;
	S_StartSound(NULL, sfx_pistol);
	M_FadeOutStart(ga_exit);

	if (Pak_Data) {
		Z_Free(Pak_Data);
		Pak_Data = NULL;
	}
}

int M_LoadPakTicker(void)
{
	static int last_f_gametic = 0;
	unsigned int buttons;
	unsigned int oldbuttons;
	int size;
	int skill;
	int levelnum;
	int exit;

	if (((int)f_gamevbls < (int)f_gametic) && (((int)f_gametic & 3U) == 0)) {
		if (last_f_gametic != (int)f_gametic) {
			last_f_gametic = (int)f_gametic;
			MenuAnimationTic = (MenuAnimationTic + 1) & 7;
		}
	}

	buttons = M_ButtonResponder(ticbuttons[0]);
	oldbuttons = oldticbuttons[0] & 0xffff0000;

	if (!(buttons & ALL_JPAD)) {
		f_m_vframe1 = 0;
	} else {
		f_m_vframe1 -= f_vblsinframe[0];

		if (f_m_vframe1 <= 0) {
			f_m_vframe1 = (float)(TICRATE / 2);

			if (buttons & PAD_DOWN) {
				cursorpos += 1;

				size = (Pak_Size / 32) - 1;

				if (size < cursorpos)
					cursorpos = size;
				else
					S_StartSound(NULL, sfx_switch1);

				if ((linepos + 5) < cursorpos)
					linepos += 1;
			} else if (buttons & PAD_UP) {
				cursorpos -= 1;

				if (cursorpos < 0)
					cursorpos = 0;
				else
					S_StartSound(NULL, sfx_switch1);

				if (cursorpos < linepos)
					linepos -= 1;
			}
		}
	}

	if (!(buttons ^ oldbuttons) || !(buttons & PAD_START)) {
		if (!(buttons ^ oldbuttons) || buttons != PAD_Z_TRIG || (Pak_Data[cursorpos * 32] == 0)) {
			exit = ga_nothing;
		} else {
			// load the password data in text format
			memcpy(&Passwordbuff, &Pak_Data[((cursorpos * 32) + 16)], 16);

			if (M_DecodePassword(Passwordbuff, &levelnum, &skill, 0) == 0) {
				CurPasswordSlot = 0;
				exit = ga_exit;
			} else {
				doPassword = true;
				CurPasswordSlot = 16;

				startmap = gamemap = levelnum;
				startskill = gameskill = skill;

				G_InitSkill(gameskill); // [Immorpher] Initialize new game skill
				exit = ga_warped;
			}
		}
	} else {
		exit = ga_exit;
	}

	return exit;
}

void M_LoadPakDrawer(void)
{
	int i;

	ST_DrawString(-1, 20, "VMU", text_alpha | 0xc0000000, ST_ABOVE_OVL);

	for (i = linepos; i < (linepos + 6); i++) {
		memset(buffer, 0, 33);

		if (FilesUsed == -1)
			memcpy(buffer, "-", 1);
		else if (Pak_Data[i * 32] == 0)
			memcpy(buffer, "no save", 7);
		else
			memcpy(buffer, (char *)&Pak_Data[i * 32], 32);

		ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xc0000000, ST_ABOVE_OVL);
	}

	if (linepos != 0)
		ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00, ST_ABOVE_OVL);

	if ((linepos + 6) <= ((Pak_Size >> 5) - 1))
		ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00, ST_ABOVE_OVL);

	ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00, ST_ABOVE_OVL);

	ST_DrawString(-1, 195, "press \x8D to exit", text_alpha | 0xffffff00, ST_ABOVE_OVL);
	ST_DrawString(-1, 210, "press \x8c to load", text_alpha | 0xffffff00, ST_ABOVE_OVL);
}

int M_ControlPadTicker(void)
{
	static int last_f_gametic = 0;
	unsigned int buttons;
	unsigned int oldbuttons;
	int exit;

	if (((int)f_gamevbls < (int)f_gametic) && ((((int)f_gametic) & 3U) == 0)) {
		if (last_f_gametic != (int)f_gametic) {
			last_f_gametic = (int)f_gametic;
			MenuAnimationTic = (MenuAnimationTic + 1) & 7;
		}
	}

	buttons = M_ButtonResponder(ticbuttons[0] & 0xffff);

	if (!(buttons & ALL_JPAD)) {
		f_m_vframe1 = 0.0f;
	} else {
		f_m_vframe1 = f_m_vframe1 - f_vblsinframe[0];
		if (f_m_vframe1 <= 0.0f) {
			f_m_vframe1 = (float)TICRATE / 2;

			if (buttons & PAD_DOWN) {
				cursorpos += 1;
				if (cursorpos < 14)
					S_StartSound(NULL, sfx_switch1);
				else
					cursorpos = 13;

				if (cursorpos > (linepos + 5))
					linepos += 1;
			} else {
				if (buttons & PAD_UP) {
					cursorpos -= 1;
					if (cursorpos < 0)
						cursorpos = 0;
					else
						S_StartSound(NULL, sfx_switch1);

					if (cursorpos < linepos)
						linepos -= 1;
				}
			}
		}
	}

	buttons = ticbuttons[0] & 0xffff0000;
	oldbuttons = oldticbuttons[0] & 0xffff0000;

	if (buttons & PAD_START) {
		S_StartSound(NULL, sfx_pistol);
		exit = 8;
	} else {
		if (buttons == oldbuttons)
			exit = 0;
		else {
			if (cursorpos == 0) { // Set Default Configuration
				memcpy(ActualConfiguration, DefaultConfiguration[0], (13 * sizeof(int)));

				memcpy(CustomConfiguration, DefaultConfiguration[0], (13 * sizeof(int)));

				if ((buttons & (ALL_BUTTONS | ALL_JPAD)) != 0) {
					S_StartSound(NULL, sfx_switch2);
					return 0;
				}
			}
			exit = 0;
		}
	}
	return exit;
}

static int dc_button_to_symbol(uint32_t code) {
	switch (code) {
	case PAD_DREAMCAST_DPAD_LEFT:
		return 80;
	case PAD_DREAMCAST_DPAD_RIGHT:
		return 81;
	case PAD_DREAMCAST_DPAD_UP:
		return 82;
	case PAD_DREAMCAST_DPAD_DOWN:
		return 83;
	case PAD_DREAMCAST_BUTTON_B:
		return 85;
	case PAD_DREAMCAST_TRIGGER_L:
		return 88;
	case PAD_DREAMCAST_TRIGGER_R:
		return 89;
	case PAD_DREAMCAST_BUTTON_X:
		return 90;
	case PAD_DREAMCAST_BUTTON_Y:
		return 91;
	case PAD_DREAMCAST_BUTTON_A:
		return 92;
	default:
		// N/A
		return 84;
	}
}

#define ST_Down '\x8f'
#define ST_Up '\x8e'
#define ST_Start '\xd'

void M_ControlPadDrawer(void)
{
	int lpos;
	char **text;
	memset(textbuff, 0, 256);
	ST_DrawString(-1, 20, "Gamepad", text_alpha | 0xc0000000, ST_ABOVE_OVL);

	if (linepos < (linepos + 6)) {
		text = &ControlText[linepos];
		lpos = linepos;
		do {
			if (lpos != 0) {
				dc_n64_map_t *next_map = &(((dc_n64_map_t *)&ingame_mapping)[lpos - 1]);
				if (next_map->dcused == 0) {
						ST_DrawSymbol(60, ((lpos - linepos) * 18) + 68, 84, text_alpha | 0xffffff00, ST_ABOVE_OVL);
				} else {
					for (int bc=0;bc<next_map->dcused;bc++) {
						ST_DrawSymbol(60 + (bc*16), ((lpos - linepos) * 18) + 68,
							dc_button_to_symbol(next_map->dcbuttons[bc]), text_alpha | 0xffffff00, ST_ABOVE_OVL);
					}
				}
			}

			if (lpos == 0) {
				if (text)
					sprintf(textbuff, "Your Configuration");
			} else {
				if (text)
					sprintf(textbuff, "%s", *text);
			}

			ST_DrawString(96, ((lpos - linepos) * 18) + 68, textbuff, text_alpha | 0xc0000000, ST_ABOVE_OVL);

			lpos += 1;
			text += 1;
		} while (lpos < (linepos + 6));
	}

	if (linepos != 0)
		ST_DrawString(80, 50, "\x8f more...", text_alpha | 0xffffff00, ST_ABOVE_OVL);

	if ((linepos + 6) < 14)
		ST_DrawString(80, 176, "\x8e more...", text_alpha | 0xffffff00, ST_ABOVE_OVL);

	ST_DrawSymbol(23, (cursorpos - linepos) * 0x12 + 0x3b, MenuAnimationTic + 0x46, text_alpha | 0xffffff00, ST_ABOVE_OVL);

	ST_DrawString(-1, 210, "press \x8d to exit", text_alpha | 0xffffff00, ST_ABOVE_OVL);
}
