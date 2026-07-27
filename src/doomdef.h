/* DoomDef.h */

#include <kos.h>
#include <stdint.h>
#include <strings.h>
#include <math.h>
typedef int fixed_t;

#include "i_main.h"

#define FOG_VERTEX 0

#define ALL_SPRITES_COUNT (575 + 310)

// next power of 2 greater than / equal to v
static inline uint32_t np2(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

static inline uint32_t Swap32(uint32_t val)
{
	return ((((val)&0xff000000) >> 24) | (((val)&0x00ff0000) >> 8) |
		(((val)&0x0000ff00) << 8) | (((val)&0x000000ff) << 24));
}

static inline short SwapShort(short dat)
{
	return ((((dat << 8) | (dat >> 8 & 0xff)) << 16) >> 16);
}

extern float empty_table[129];

typedef enum {
	rumblepak_off,
	rumblepak_on,
	// rumblepak_other,
	NUM_RUMBLEPAKS
} i_rumble_pak_t;

typedef enum {
	rumble_hoof,
	rumble_quake,
	rumble_punch,
	rumble_saw,
	rumble_sawready,
	rumble_missile,
	rumble_bfg,
	rumble_plasma,
	rumble_pistol,
	rumble_shotgun,
	rumble_shotgun2,
	rumble_cgun,
	rumble_laser,
	rumble_oof,
	rumble_thunder,
	NUM_RUMBLE
} i_rumble_t;

typedef enum {
	maple_controller,
	maple_lcd,
	maple_memcard,
	maple_rumble,
	maple_mouse,
	maple_keyboard,

	NUM_MAPLE,
} i_maple_t;

extern maple_device_t *maple_devices[NUM_MAPLE];

extern purupuru_effect_t rumble_patterns[NUM_RUMBLE];

purupuru_effect_t I_GetDamageRumble(int damage);
void I_Rumble(purupuru_effect_t effect);

void I_VMUUpdateFace(uint8_t* image, int force_refresh);
void I_VMUFB(int force_refresh);

#define SETTINGS_SAVE_VERSION 3

typedef enum {
	q_low,
	q_medium,
	q_ultra,
	NUM_QUALITY
} r_quality_t;

typedef struct doom64_settings_s {
	int version;
	int HUDopacity;
	int SfxVolume;
	int MusVolume;
	int brightness;
	int enable_messages;
	int M_SENSITIVITY;
	int MotionBob;
	int Rumble;
	int VideoFilter;
	int Autorun;
	int runintroduction;
	int StoryText;
	int MapStats;
	int HUDmargin;
	int ColoredHUD;
	int Quality;
	int FpsUncap;
	int PlayDeadzone;
	int Interpolate;
	int VmuDisplay;
} doom64_settings_t;

extern doom64_settings_t __attribute__((aligned(32))) menu_settings;

extern void M_ResetSettings(doom64_settings_t *s);
extern int I_ReadPakSettings(doom64_settings_t *s);
extern int I_SavePakSettings(doom64_settings_t *s);
#define halfover1024 0.00048828125f
#define recip16 0.0625f
#define recip60 0.01666666753590106964111328125f
#define recip64 0.015625f
#define recip128 0.0078125f
#define recip255 0.0039215688593685626983642578125f
#define recip256 0.00390625f
#define recip512 0.001953125f
#define recip1k 0.0009765625f
#define recip64k 0.0000152587890625f
#define recip64kx64 2.4220300099206349206349206349206e-7f

#define quarterpi_i754 0.785398185253143310546875f
#define halfpi_i754 1.57079637050628662109375f
#define pi_i754 3.1415927410125732421875f
#define twopi_i754 6.283185482025146484375f

typedef enum {
	PAL_ENEMY,
	PAL_ITEM,
	PAL_FLAT,
	PAL_I8
} d64_palette_t;

#define D64_TARGB (PVR_TXRFMT_ARGB1555 | PVR_TXRFMT_TWIDDLED)
#define D64_TPAL(n) (PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL((n)) | PVR_TXRFMT_TWIDDLED)
#define D64_TBUMP (PVR_TXRFMT_BUMP | PVR_TXRFMT_TWIDDLED)
#define D64_TI4 (PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_TWIDDLED)

#define NUM_DYNLIGHT 16

//https://stackoverflow.com/a/3693557
#define quickDistCheck(dx,dy,lr) (((dx) + (dy)) <= ((lr)<<1))

extern char *fnpre;

#define COMPONENT_INTENSITY 96

typedef struct {
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
	float radius;
	float distance;
} projectile_light_t;

#ifdef DCLOCALDEV
#define STORAGE_PREFIX "/pc"
#else
#define STORAGE_PREFIX "/cd"
#endif

#define MAX_CACHED_SPRITES 256

#define FUNLEVEL(map)	(((map) == 25 || (map) == 26 || (map) == 27 || (map) == 33 || (map) == 40))

#define TR_VERTBUF_SIZE (1536*1024)
#define PT_VERTBUF_SIZE (128*1024)
extern uint8_t __attribute__((aligned(32))) tr_buf[TR_VERTBUF_SIZE];
extern uint8_t __attribute__((aligned(32))) pt_buf[PT_VERTBUF_SIZE];
extern int context_change;

extern unsigned char lightcurve[256];
extern unsigned char lightmax[256];

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MAX3(a,b,c) (MAX(MAX((a),(b)), (c)))
#define MAX4(a,b,c,d) (MAX(MAX3((a),(b),(c)),(d)))

// this was originally doing (((aaa)&1)<<15) to set the alpha bit
// that isn't right especially not when I was setting palette entries for intensity textures
#define get_color_argb1555(rrr, ggg, bbb, aaa)						\
	((uint16_t)(((!!aaa) << 15) | (((rrr >> 3) & 0x1f) << 10) |	\
		    (((ggg >> 3) & 0x1f) << 5) | ((bbb >> 3) & 0x1f)))

#define LOSTLEVEL 34
#define KNEEDEEP 41

typedef struct d64_bg_s {
	int x;
	int y;
	int num;
} d64_bg_t;

typedef enum {
	USLEGAL,
	IDCRED1,
	IDCRED2,
	WMSCRED1,
	WMSCRED2,
	EVIL,
	FINAL,
	TITLE,
	NUM_BG
} d64_bg_enum_t;

// twiddling stuff copied from whatever file which copied it from kmgenc.c
#define TWIDTAB(x)													\
	((x & 1) | ((x & 2) << 1) | ((x & 4) << 2) | ((x & 8) << 3) |	\
	 ((x & 16) << 4) | ((x & 32) << 5) | ((x & 64) << 6) |			\
	 ((x & 128) << 7) | ((x & 256) << 8) | ((x & 512) << 9))

#define TWIDOUT(x, y) (TWIDTAB((y)) | (TWIDTAB((x)) << 1))

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define _PAD8(x) x += (8 - ((uint)x & 7)) & 7

#define UNPACK_R(color) ((color >> 24) & 0xff)
#define UNPACK_G(color) ((color >> 16) & 0xff)
#define UNPACK_B(color) ((color >> 8) & 0xff)
#define UNPACK_A(color) (color & 0xff)

#define D64_PVR_REPACK_COLOR(color)	\
	(((color >> 8) & 0x00ffffff) | (color << 24))
#define D64_PVR_REPACK_COLOR_ALPHA(color, a)	\
	(((color >> 8) & 0x00ffffff) | (a << 24))
#define D64_PVR_PACK_COLOR(a, r, g, b) ((a << 24) | (r << 16) | (g << 8) | b)

#define SCREEN_WD 320

#define RES_RATIO 2.0f

#define RECIP_FINEANGLES 0.0001220703125f

#define doomangletoQ(x)  ((float)x * 0.00000000023283064365386962890625f)
//((float)((x) >> ANGLETOFINESHIFT) * RECIP_FINEANGLES)
//((float)((x) >> ANGLETOFINESHIFT) / (float)FINEANGLES))

extern float last_fps;
#include "sounds.h"
extern sfxhnd_t sounds[NUMSFX];
extern float soundscale;
extern int plasma_loop_channel;
extern void P_StartElectricLoop(void);
extern void P_StopElectricLoop(void);
#define PFS_ERR_NOPACK 1
#define PFS_ERR_ID_FATAL 2

extern int32_t Pak_Memory;
extern uint8_t *Pak_Data;

typedef struct subsector_s subsector_t;

typedef struct {
	// 0
	uint32_t global_lit;
	// 4
	subsector_t *global_sub;
	// 8
	// 0 low 1 med 2 ultra
	uint8_t quality;
	// 9
	// 0 30 1 uncapped
	uint8_t fps_uncap;
	// 10
	uint8_t has_bump;
	// 11
	// 1 floor 2 ceiling 0 other
	uint8_t in_floor;
	// 12
	uint8_t in_things;
	// 13
	uint8_t context_change;
	// 14
	uint8_t floor_split_override;
	// 15
	uint8_t dont_bump;
	// 16
	uint8_t dont_color;
	// 17
	uint8_t pad[3];
	// 20
	float normx;
	// 24
	float normy;
	// 28
	float normz;
	// 32
} render_state_t;
extern render_state_t __attribute__((aligned(32))) global_render_state;

typedef struct {
	pvr_vertex_t *v; // 0
	float w; // 4
	unsigned lit; // 8
	float pad1; // 12
	float r; // 16
	float g; // 20
	float b; // 24
	float pad2; // 28
} d64ListVert_t;

typedef struct {
	unsigned n_verts;
	pvr_poly_hdr_t *hdr;
	d64ListVert_t __attribute__((aligned(32))) dVerts[5];
} d64Poly_t;

void draw_pvr_line(vector_t *v1, vector_t *v2, int color);

#define transform_d64ListVert(d64v) mat_trans_single3_nodivw((d64v)->v->x, (d64v)->v->y, (d64v)->v->z, (d64v)->w)

// only works for positive x
#define approx_recip(x) (1.0f / sqrtf((x)*(x)))

// legacy renderer functions, used by laser and wireframe automap

static inline void transform_vector(vector_t *d64v)
{
	/* no divide, for trivial rejection and near-z clipping */
	mat_trans_single3_nodivw(d64v->x, d64v->y, d64v->z, d64v->w);
}

static inline void perspdiv_vector(vector_t *v)
{
	float invw = approx_recip(v->w);
	v->x *= invw;
	v->y *= invw;
	v->z = invw;
}

void I_ParseMappingFile(char *mapping_file);

extern int __attribute__((aligned(16))) DefaultConfiguration[1][13];

#define PAD_DREAMCAST_DPAD_RIGHT	0x01000000
#define PAD_DREAMCAST_DPAD_LEFT		0x02000000
#define PAD_DREAMCAST_DPAD_DOWN		0x04000000
#define PAD_DREAMCAST_DPAD_UP		0x08000000
#define PAD_DREAMCAST_BUTTON_START	0x10000000
#define PAD_DREAMCAST_TRIGGER_L		0x20000000
#define PAD_DREAMCAST_TRIGGER_R		0x40000000
#define PAD_DREAMCAST_BUTTON_A		0x00100000
#define PAD_DREAMCAST_BUTTON_B		0x00200000
#define PAD_DREAMCAST_BUTTON_X		0x00400000
#define PAD_DREAMCAST_BUTTON_Y		0x00800000
// this is where in-game button mapping actually happens
typedef struct dreamcast_n64_pad_mapping {
	unsigned int n64button;
	unsigned int dcbuttons[2];
	int dcused;
} dc_n64_map_t;
// always defaults to the Default Configuration on the N64 side
// this allows straightforward mapping of DC buttons to N64 ACTIONS, not buttons
typedef struct {
	dc_n64_map_t map_right;
	dc_n64_map_t map_left;
	dc_n64_map_t map_up;
	dc_n64_map_t map_down;
	dc_n64_map_t map_attack;
	dc_n64_map_t map_use;
	dc_n64_map_t map_automap;
	dc_n64_map_t map_speedonoff;
	dc_n64_map_t map_strafeonoff;
	dc_n64_map_t map_strafeleft;
	dc_n64_map_t map_straferight;
	dc_n64_map_t map_weaponbackward;
	dc_n64_map_t map_weaponforward;
} mapped_buttons_t;

// these need to be updated if the layout of `mapped_buttons_t` changes
#define MAP_COUNT 13
#define STRAFE_LEFT_INDEX 9
#define STRAFE_RIGHT_INDEX 10

extern mapped_buttons_t ingame_mapping;

/*-----------*/
/* SYSTEM IO */
/*-----------*/

/*============================================================================= */

/* Fixes and Version Update Here*/

// NEWS Updates
// Nightmare Mode Originally Activated in the project [GEC] Master Edition.

// FIXES

// Fixes for the 'linedef deletion' bug. From PsyDoom
#define FIX_LINEDEFS_DELETION 1

/*============================================================================= */
#define backcheck o_d122b67458594bfc8c1b920e63847f5b
/* all external data is defined here */
#include "doomdata.h"

/* header generated by multigen utility */
#include "doominfo.h"

#define MAXCHAR ((char)0x7f)
#define MAXSHORT ((short)0x7fff)
#define MAXINT ((int)0x7fffffff) /* max pos 32-bit int */
#define MAXLONG ((long)0x7fffffff)

#define MINCHAR ((char)0x80)
#define MINSHORT ((short)0x8000)
#define MININT ((int)0x80000000) /* max negative 32-bit integer */
#define MINLONG ((long)0x80000000)

void ST_Init(void);

/* c_convert.c  */
uint32_t LightGetHSV(uint8_t r, uint8_t g, uint8_t b);
uint32_t LightGetRGB(uint8_t h, uint8_t s, uint8_t v);

/* p* */
void P_RefreshBrightness(void);

typedef float __attribute__((aligned(32))) Matrix[4][4];

// lifted from modern libultra
static inline void R_Ident(Matrix mf)
{
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (i == j)
				mf[i][j] = 1.0;
			else
				mf[i][j] = 0.0;
		}
	}
}

static inline void R_Frustum(Matrix mf, float l, float r, float b, float t, float n, float f, float scale)
{
	int i, j;
	R_Ident(mf);
	mf[0][0] = 2 * n / (r - l);
	mf[1][1] = 2 * n / (t - b);
	mf[2][0] = (r + l) / (r - l);
	mf[2][1] = (t + b) / (t - b);
	mf[2][2] = -(f + n) / (f - n);
	mf[2][3] = -1;
	mf[3][2] = -2 * f * n / (f - n);
	mf[3][3] = 0;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			mf[i][j] *= scale;
		}
	}
}

// I derived this from the glDC commit that moved screenspace transform into matrix
static inline void R_Viewport(Matrix mf, int x, int y, int width, int height) {
    mf[0][0] = (float)width / 2.0f;
    mf[1][1] = -(float)height / 2.0f;
    mf[2][2] = 1.0f;
    mf[3][3] = 1.0f;

    mf[3][0] = (float)x + ((float)width / 2.0f);
    mf[3][1] = 480.0f - ((float)y + ((float)height / 2.0f));
}

// the matrix setup in Doom 64 RE looks like it was "inlined"
// I broke it out into functions to better understand what it was doing
// and how viewproj was constructed

static inline void R_Translate(Matrix mf, float x, float y, float z)
{
	R_Ident(mf);
	mf[3][0] = x;
	mf[3][1] = y;
	mf[3][2] = z;
}

static inline void R_RotateX(Matrix mf, float in_sin, float in_cos)
{
	R_Ident(mf);
	mf[1][1] = in_cos;
	mf[1][2] = -in_sin;
	mf[2][1] = in_sin;
	mf[2][2] = in_cos;
}

static inline void R_RotateY(Matrix mf, float in_sin, float in_cos)
{
	R_Ident(mf);
	mf[0][0] = in_sin;
	mf[0][2] = -in_cos;
	mf[2][0] = in_cos;
	mf[2][2] = in_sin;
}

// [Striker] Interpolation function
static inline int interpolate(int a, int b, float fraction)
{
	return a + (int)(fraction * (float)(b-a));
}

#define backres o_ad675382a0ccc360672c24686a0f93ee

/*
===============================================================================

						GLOBAL TYPES

===============================================================================
*/

#define MAXPLAYERS 1 /* D64 has one player */
#define TICRATE 30 /* number of tics / second */

#define FRACBITS 16
#define FRACUNIT (1 << FRACBITS)

typedef unsigned angle_t;

#define ANG45 (/* (angle_t) */0x20000000)
#define ANG90 (/* (angle_t) */0x40000000)
#define ANG180 (/* (angle_t) */0x80000000)
#define ANG270 (/* (angle_t) */0xc0000000)
#define ANG5 (/* (angle_t) */0x38e0000) // (ANG90/18)
#define ANG1 (/* (angle_t) */0xb60000) // (ANG45/45)

#define FINEANGLES 8192
#define FINEMASK (FINEANGLES - 1)
#define ANGLETOFINESHIFT 19 /* 0x100000000 to 0x2000 */

#define TRUEANGLES(x) ((float)x * 0.00000008381903171539306640625f)
//(((x) >> ANGLETOFINESHIFT) * 0.0439453125f)
// 								* 360.0f / FINEANGLES

unsigned D_abs(signed val);

extern fixed_t __attribute__((aligned(32))) finesine[5 * FINEANGLES / 4];
extern fixed_t *finecosine;

extern const angle_t __attribute__((aligned(32))) tantoangle[2049];

extern char  __attribute__((aligned(32))) fnbuf[256];

typedef enum { sk_baby, sk_easy, sk_medium, sk_hard, sk_nightmare } skill_t;

typedef enum {
	ga_nothing,
	ga_died,
	ga_completed,
	ga_secretexit, // no used
	ga_warped,
	ga_exitdemo,
	ga_timeout,
	ga_restart,
	ga_exit
} gameaction_t;


#define ABS_LASTLEVEL 34
#define ABS_TOTALMAPS 33

#define LOST_LASTLEVEL 41
#define LOST_TOTALMAPS 40

#define KNEE_LASTLEVEL 50
#define KNEE_TOTALMAPS 49


/* */
/* library replacements */
/* */
#include <string.h>

/*
===============================================================================

							MAPOBJ DATA

===============================================================================
*/

struct mobj_s;

struct thinker_s;

/* think_t is a function pointer to a routine to handle an actor */
typedef void (*think_t)();

/* a latecall is a function that needs to be called after p_base is done */
typedef void (*latecall_t)(struct mobj_s *mo);

typedef struct thinker_s {
	struct thinker_s *prev, *next;
	think_t function;
} thinker_t;

struct player_s;

typedef struct mobj_s {
	/* info for drawing */
	fixed_t x, y, z;
	/* for movement interpolation */
	fixed_t old_x, old_y, old_z;

	struct subsector_s *subsector;

	int flags;
	struct player_s *player; /* only valid if type == MT_PLAYER */

	struct mobj_s *prev, *next;
	struct mobj_s *snext, *sprev; /* links in sector (if needed) */
	struct mobj_s *bnext, *bprev; /* links in blocks (if needed) */

	struct mobj_s *target; /* thing being chased/attacked (or NULL) */
	struct mobj_s *tracer; /* Thing being chased/attacked for tracers. */

	angle_t angle;
	int sprite; /* used to find patch_t and flip value */
	int frame; /* might be ord with FF_FULLBRIGHT */
	fixed_t floorz, ceilingz; /* closest together of contacted secs */
	fixed_t radius, height; /* for movement checking */
	fixed_t momx, momy, momz; /* momentums */

	mobjtype_t type;
	mobjinfo_t *info; /* &mobjinfo[mobj->type] */
	int tics; /* state tic counter	 */
	float f_tics;
	state_t *state;

	int health;
	int movedir; /* 0-7 */
	int movecount; /* when 0, select a new dir */

	/* also the originator for missiles */
	int reactiontime; /* if non 0, don't attack yet */
	/* used by player to freeze a bit after */
	/* teleporting */
	int threshold; /* if >0, the target will be chased */
	/* no matter what (even if shot) */

	int alpha; /* [D64] alpha value */

	void *extradata; /* for latecall functions */

	latecall_t latecall; /* set in p_base if more work needed */

	int tid; /* [D64] tid value */

	int sfx_chn;
} mobj_t;

/* each sector has a degenmobj_t in it's center for sound origin purposes */
struct subsector_s;
typedef struct {
	fixed_t x, y, z;
	struct subsector_s *subsec; // Psx Doom / Doom 64 New
} degenmobj_t;

typedef struct laserdata_s {
	fixed_t x1, y1, z1;
	fixed_t x2, y2, z2;
	fixed_t slopex, slopey, slopez;
	fixed_t distmax, dist;
	mobj_t *marker;
	struct laserdata_s *next;
} laserdata_t;

typedef struct laser_s {
	thinker_t thinker;
	laserdata_t *laserdata;
	mobj_t *marker;
} laser_t;

/* */
/* frame flags */
/* */
#define FF_FULLBRIGHT 0x8000 /* flag in thing->frame */
#define FF_FRAMEMASK 0x7fff

/* */
/* mobj flags */
/* */
#define MF_SPECIAL 1 /* call P_SpecialThing when touched */
#define MF_SOLID 2
#define MF_SHOOTABLE 4
#define MF_NOSECTOR 8 /* don't use the sector links */
/* (invisible but touchable)  */
#define MF_NOBLOCKMAP 16 /* don't use the blocklinks  */
/* (inert but displayable) */
#define MF_AMBUSH 32
#define MF_JUSTHIT 64 /* try to attack right back */
#define MF_JUSTATTACKED 128 /* take at least one step before attacking */
#define MF_SPAWNCEILING 256 /* hang from ceiling instead of floor */
//#define	MF_NOGRAVITY	512			/* don't apply gravity every tic */
#define MF_GRAVITY 512 /* apply gravity every tic */

/* movement flags */
#define MF_DROPOFF 0x400 /* allow jumps from high places */
#define MF_PICKUP 0x800 /* for players to pick up items */
#define MF_NOCLIP 0x1000 /* player cheat */
#define MF_TRIGDEATH 0x2000 /* [d64] trigger line special on death */
#define MF_FLOAT 0x4000 /* allow moves to any height, no gravity */
#define MF_TELEPORT 0x8000 /* don't cross lines or look at heights */
#define MF_MISSILE 0x10000 /* don't hit same species, explode on block */

#define MF_DROPPED 0x20000 /* dropped by a demon, not level spawned */
#define MF_TRIGTOUCH 0x40000 /* [d64] trigger line special on touch/pickup */
#define MF_NOBLOOD 0x80000 /* don't bleed when shot (use puff) */
#define MF_CORPSE 0x100000 /* don't stop moving halfway off a step */
#define MF_INFLOAT 0x200000 /* floating to a height for a move, don't */
/* auto float to target's height */
#define MF_COUNTKILL 0x400000 /* count towards intermission kill total */
#define MF_COUNTITEM 0x800000 /* count towards intermission item total */

#define MF_SKULLFLY 0x1000000 /* skull in flight */
#define MF_NOTDMATCH 0x2000000 /* don't spawn in death match (key cards) */

#define MF_SEETARGET 0x4000000 /* is target visible? */

/* Doom 64 New Flags */
#define MF_COUNTSECRET \
	0x8000000 /* [d64] Count as secret when picked up (for intermissions) */
#define MF_RENDERLASER 0x10000000 /* [d64] Exclusive to MT_LASERMARKER only */
#define MF_SHADOW 0x40000000 /* temporary player invisibility powerup. */
#define MF_NOINFIGHTING 0x80000000 /* [d64] Do not switch targets */

//(val << 0 < 0) 0x80000000
//(val << 1 < 0) 0x40000000
//(val << 2 < 0) 0x20000000
//(val << 3 < 0) 0x10000000
//(val << 4 < 0) 0x8000000
//(val << 5 < 0) 0x4000000
//(val << 6 < 0) 0x2000000
//(val << 7 < 0) 0x1000000
//(val << 8 < 0) 0x800000
//(val << 9 < 0) 0x400000
//(val << a < 0) 0x200000
//(val << b < 0) 0x100000
//(val << c < 0) 0x80000
//(val << d < 0) 0x40000
//(val << e < 0) 0x20000
//(val << f < 0) 0x10000

/* Exclusive Psx Doom Flags */
//#define	MF_BLENDMASK1	0x10000000
//#define	MF_BLENDMASK2	0x20000000
//#define	MF_BLENDMASK3	0x40000000
//#define	MF_ALL_BLEND_MASKS  (MF_BLENDMASK1 | MF_BLENDMASK2 | MF_BLENDMASK3)

/*============================================================================= */
typedef enum {
	PST_LIVE, /* playing */
	PST_DEAD, /* dead on the ground */
	PST_REBORN /* ready to restart */
} playerstate_t;

/* psprites are scaled shapes directly on the view screen */
/* coordinates are given for a 320*200 view screen */
typedef enum {
	ps_weapon,
	ps_flash,
	ps_flashalpha, // New Doom64
	NUMPSPRITES
} psprnum_t;

typedef struct {
	state_t *state; /* a NULL state means not active */
	int tics;
	int alpha;
	fixed_t sx, sy;
} pspdef_t;

typedef enum {
	it_bluecard,
	it_yellowcard,
	it_redcard,
	it_blueskull,
	it_yellowskull,
	it_redskull,
	NUMCARDS
} card_t;

typedef enum {
	wp_chainsaw = 0,
	wp_fist,
	wp_pistol,
	wp_shotgun,
	wp_supershotgun, // [psx]
	wp_chaingun,
	wp_missile,
	wp_plasma,
	wp_bfg,
	wp_laser, // [d64]
	NUMWEAPONS,
	wp_nochange
} weapontype_t;

extern void W_ReplaceWeaponBumps(weapontype_t wepn);

typedef enum {
	am_clip, /* pistol / chaingun */
	am_shell, /* shotgun */
	am_cell, /* BFG / plasma / #$&%*/
	am_misl, /* missile launcher */
	NUMAMMO,
	am_noammo /* chainsaw / fist */
} ammotype_t;

typedef enum {
	ART_FAST = 1,
	ART_DOUBLE = 2,
	ART_TRIPLE = 3,
} artifacts_t;

typedef struct {
	ammotype_t ammo;
	int upstate;
	int downstate;
	int readystate;
	int atkstate;
	int flashstate;
} weaponinfo_t;

extern weaponinfo_t weaponinfo[NUMWEAPONS]; // 8005AD80

typedef enum {
	pw_invulnerability,
	pw_strength,
	pw_invisibility,
	pw_ironfeet,
	pw_allmap,
	pw_infrared,
	NUMPOWERS
} powertype_t;

#define INVULNTICS (30 * 30)
#define INVISTICS (60 * 30)
#define INFRATICS (120 * 30)
#define IRONTICS (60 * 30)
#define STRTICS (3 * 30)

#define MSGTICS (5 * 30)

/*
================
=
= player_t
=
================
*/

typedef struct player_s {
	mobj_t *mo;
	playerstate_t playerstate;

	fixed_t forwardmove, sidemove; /* built from ticbuttons */
	/*angle_t*/int angleturn; /* built from ticbuttons */

	fixed_t viewz; /* focal origin above r.z */
	fixed_t viewheight; /* base height above floor for viewz */
	fixed_t deltaviewheight; /* squat speed */
	fixed_t bob; /* bounded/scaled total momentum */
	fixed_t recoilpitch; /* [D64] new*/
	// consider cache line size for ordering of all of these fields
	fixed_t lerpZ; // [Striker] Z for lerp code.

	int health; /* only used between levels, mo->health */
	/* is used during levels	 */
	int armorpoints, armortype; /* armor type is 0-2 */

	int powers[NUMPOWERS]; /* invinc and invis are tic counters	 */
	float f_powers[NUMPOWERS];
	boolean cards[NUMCARDS];
	int artifacts; /* [d64]*/
	boolean backpack;
	int frags; /* kills of other player */
	weapontype_t readyweapon;
	weapontype_t pendingweapon; /* wp_nochange if not changing */
	boolean weaponowned[NUMWEAPONS];
	int ammo[NUMAMMO];
	int maxammo[NUMAMMO];
	int attackdown, usedown; /* true if button down last tic */
	int cheats; /* bit flags */

	int refire; /* refired shots are less accurate */

	int killcount, itemcount, secretcount; /* for intermission */
	char *message; /* hint messages */
	char *message1; // [Immorpher] additional message levels
	char *message2; // [Immorpher] additional message levels
	char *message3; // [Immorpher] additional message levels
	int messagetic; /* messages tic countdown*/
	int messagetic1; // [Immorpher] message tic buffer
	int messagetic2; // [Immorpher] message tic buffer
	int messagetic3; // [Immorpher] message tic buffer
	unsigned int messagecolor; // [Immorpher] message color
	unsigned int messagecolor1; // [Immorpher] message color 1
	unsigned int messagecolor2; // [Immorpher] message color 2
	unsigned int messagecolor3; // [Immorpher] message color 3
	int damagecount, bonuscount; /* for screen flashing */
	float f_damagecount, f_bonuscount;
	int bfgcount; /* for bfg screen flashing */
	float f_bfgcount;
	mobj_t *attacker; /* who did damage (NULL for floors) */
	int extralight; /* so gun flashes light up areas */
	pspdef_t psprites[NUMPSPRITES]; /* view sprites (gun, etc) */

	void *lastsoundsector; /* don't flood noise every time */

	int automapx, automapy, automapscale, automapflags;

	int turnheld; /* for accelerative turning */
	float f_turnheld; /* for accelerative turning */
	int onground; /* [d64] */
} player_t;

#define CF_NOCLIP 1 // no use
#define CF_GODMODE 2
#define CF_ALLMAP 4
#define CF_LOCKMOSTERS 0x800
#define CF_WALLBLOCKING 0x1000
#define CF_WEAPONS 0x2000
#define CF_HEALTH 0x4000
#define CF_ALLKEYS 0x8000

#define CF_NOCOLORS 0x20000 // [GEC] NEW CHEAT CODE
#define CF_FULLBRIGHT 0x40000 // [GEC] NEW CHEAT CODE
#define CF_GAMMA 0x80000 // [Immorpher] NEW CHEAT CODE

#define AF_LINES 1 /* automap active on lines mode */
#define AF_SUBSEC 2 /* automap active on subsector mode */
#define AF_FOLLOW 4

/*
===============================================================================

					GLOBAL VARIABLES

===============================================================================
*/
extern dirent_t __attribute__((aligned(32))) FileState[200];

/*================================== */

extern int gamevbls; // 80063130 /* may not really be vbls in multiplayer */
extern int gametic; // 80063134
extern int ticsinframe; // 80063138 /* how many tics since last drawer */
extern int ticon; // 8006313C
extern int lastticon; // 80063140
extern int vblsinframe[MAXPLAYERS]; // 80063144 /* range from 4 to 8 */
extern int ticbuttons[MAXPLAYERS]; // 80063148
extern int oldticbuttons[MAXPLAYERS]; // 8006314C

extern float f_ticon;
extern float f_lastticon;
extern float f_ticsinframe;
extern float f_gamevbls;
extern float f_gametic;
// for interpolation
extern float f_lastgametic;
extern float f_vblsinframe[MAXPLAYERS];

extern boolean gamepaused;

extern int DrawerStatus;

//extern	int		maxlevel;			/* highest level selectable in menu (1-25) */
extern int in_menu;
int MiniLoop(void (*start)(void), void (*stop)(int), int (*ticker)(void),
	     void (*drawer)(void));

int G_Ticker(void);
void G_Drawer(void);
void G_RunGame(void);

/*================================== */

extern gameaction_t gameaction;

#define SBARHEIGHT 32 /* status bar height at bottom of screen */

typedef enum { gt_single, gt_coop, gt_deathmatch } gametype_t;

extern player_t players[MAXPLAYERS];

extern skill_t gameskill;
extern int gamemap;
extern int nextmap;
extern int totalkills, totalitems, totalsecret; /* for intermission */

extern mapthing_t playerstarts[MAXPLAYERS];

/*
===============================================================================

					GLOBAL FUNCTIONS

===============================================================================
*/

fixed_t FixedMul(fixed_t a, fixed_t b);
// used by engine code
fixed_t FixedDivFloat(fixed_t a, fixed_t b);
// used by setup code
fixed_t FixedDiv(fixed_t a, fixed_t b);

/*----------- */
/*MEMORY ZONE */
/*----------- */
/* tags < 8 are not overwritten until freed */
#define PU_STATIC 1 /* static entire execution time */
#define PU_LEVEL 2 /* static until level exited */
#define PU_LEVSPEC 4 /* a special thinker in a level */
/* tags >= 8 are purgable whenever needed */
#define PU_PURGELEVEL 8
#define PU_CACHE 16

#define ZONEID 0x1d4a

#define BLOCKALIGN(size,align) (((size) + ((align)-1)) & ~((align)-1))
#define MEM_HEAP_SIZE (0x528000) 
#define MINFRAGMENT 32

typedef struct memblock_s {
	int size; /* including the header and possibly tiny fragments */
	void **user; /* NULL if a free block */
	int tag; /* purgelevel */
	int id; /* should be ZONEID */
	int lockframe; /* don't purge on the same frame */
	struct memblock_s *next;
	struct memblock_s *prev;
	void *gfxcache; /* New on Doom64 */
} memblock_t;

typedef struct {
	int size; /* total bytes malloced, including header */
	memblock_t *rover;
	memblock_t *rover2; /* New on Doom64 */
	memblock_t *rover3; /* New on Doom64 */
	memblock_t blocklist; /* start / end cap for linked list */
} memzone_t;

extern memzone_t *mainzone;

void Z_Init(void);
memzone_t *Z_InitZone(uint8_t *base, int size);
void Z_SetAllocBase(memzone_t *mainzone);
int Z_FreeMemory(memzone_t *mainzone);
void Z_Defragment(memzone_t *mainzone);
void Z_DumpHeap(memzone_t *mainzone);

#if RANGECHECK

void *__Z_Malloc2(memzone_t *mainzone, int size, int tag, void *ptr, uintptr_t retaddr, const char *file, int line);
void *__Z_Alloc2(memzone_t *mainzone, int size, int tag,
	       void *user, uintptr_t retaddr, const char *file, int line); // PsxDoom / Doom64
void __Z_Free2(memzone_t *mainzone, void *ptr, const char *file, int line);

#define Z_Malloc(x, y, z) __Z_Malloc2(mainzone, x, y, z, arch_get_ret_addr(), __FILE__,__LINE__)
#define Z_Alloc(x, y, z) __Z_Alloc2(mainzone, x, y, z,arch_get_ret_addr(), __FILE__,__LINE__)
#define Z_Free(x) __Z_Free2(mainzone, x,__FILE__,__LINE__)

void __Z_FreeTags(memzone_t *mainzone, int tag, const char *file, int line);
#define Z_FreeTags(a,b) __Z_FreeTags(a,b,__FILE__,__LINE__)
void __Z_Touch(void *ptr, const char *file, int line);
#define Z_Touch(a) __Z_Touch(a,__FILE__,__LINE__)
void __Z_CheckZone(memzone_t *mainzone, const char *file, int line);
#define Z_CheckZone(a) __Z_CheckZone(a,__FILE__,__LINE__)
void __Z_ChangeTag(void *ptr, int tag, const char *file, int line);
#define Z_ChangeTag(a,b) __Z_ChangeTag(a,b,__FILE__,__LINE__)

#else

void *Z_Malloc2(memzone_t *mainzone, int size, int tag, void *ptr);
void *Z_Alloc2(memzone_t *mainzone, int size, int tag,
	       void *user); // PsxDoom / Doom64
void Z_Free2(memzone_t *mainzone, void *ptr);

#define Z_Malloc(x, y, z) Z_Malloc2(mainzone, x, y, z)
#define Z_Alloc(x, y, z) Z_Alloc2(mainzone, x, y, z)
#define Z_Free(x) Z_Free2(mainzone, x)

void Z_FreeTags(memzone_t *mainzone, int tag);
void Z_Touch(void *ptr);
void Z_CheckZone(memzone_t *mainzone);
void Z_ChangeTag(void *ptr, int tag);

#endif

/*------- */
/*WADFILE */
/*------- */

// New Doom64
typedef enum { dec_none, dec_jag, dec_d64 } decodetype;

typedef struct {
	int filepos; /* also texture_t * for comp lumps */
	int size;
	char name[8];
} lumpinfo_t;

typedef struct {
	void *cache;
} lumpcache_t;

void W_Init(void);

char *W_GetNameForNum(int lump);

int W_CheckNumForName(char *name);
int W_GetNumForName(char *name);

int W_LumpLength(int lump);
void W_ReadLump(int lump, void *dest, decodetype dectype);

void *W_CacheLumpNum(int lump, int tag, decodetype dectype);
void *W_CacheLumpName(char *name, int tag, decodetype dectype);

int W_S2_CheckNumForName(char *name);
int W_S2_GetNumForName(char *name);

int W_S2_LumpLength(int lump);
void W_S2_ReadLump(int lump, void *dest);

void *W_S2_CacheLumpNum(int lump, int tag);
void *W_S2_CacheLumpName(char *name, int tag);

#if 1
int W_Bump_CheckNumForName(char *name);
int W_Bump_GetNumForName(char *name);

int W_Bump_LumpLength(int lump);
void W_Bump_ReadLump(int lump, void *dest, int w, int h);
#endif

void W_OpenMapWad(int mapnum);
void W_FreeMapLump(void);
int W_MapLumpLength(int lump);
int W_MapGetNumForName(char *name);
void *W_GetMapLump(int lump);

/*---------*/
/* DECODES */
/*---------*/
void DecodeD64(unsigned char *input, unsigned char *output);
void DecodeJaguar(unsigned char *input, unsigned char *output);
void decode_bumpmap(uint8_t *in, uint8_t *out, int w, int h);

/*------------*/
/* BASE LEVEL */
/*------------*/

/*--------*/
/* D_MAIN */
/*--------*/

void D_DoomMain(void);

/*------*/
/* GAME */
/*------*/

extern boolean demoplayback;
extern int *demo_p, *demobuffer;

void G_InitNew(skill_t skill, int map, gametype_t gametype);
void G_InitSkill(skill_t skill); // [Immorpher] skill initialize
void G_CompleteLevel(void);
void G_RecordDemo(void);
int G_PlayDemoPtr(int skill, int map);
void G_PlayerFinishLevel(int player);

/*------*/
/* PLAY */
/*------*/

mobj_t *P_SpawnMapThing(mapthing_t *mthing);
void P_SetupLevel(int map, skill_t skill);
void P_Init(void);

void P_Start(void);
void P_Stop(int exit);
int P_Ticker(void);
void P_Drawer(void);

/*---------*/
/* IN_MAIN */
/*---------*/

void IN_Start(void);
void IN_Stop(int);
int IN_Ticker(void);
void IN_Drawer(void);

/*--------*/
/* M_MAIN */
/*--------*/

typedef void (*menufunc_t)(void);

typedef struct {
	int casepos;
	int x;
	int y;
} menuitem_t;

typedef struct {
	menuitem_t *menu_item;
	int item_lines;
	menufunc_t menu_call;
	int cursor_pos;
} menudata_t;

extern menudata_t MenuData[8]; // 800A54F0
extern menuitem_t Menu_Game[5]; // 8005AAA4
extern int MenuAnimationTic; // 800a5570
extern int cursorpos; // 800A5574
//extern int m_vframe1; // 800A5578
extern float f_m_vframe1; // 800A5578
extern menuitem_t *MenuItem; // 800A5578
extern int itemlines; // 800A5580
extern menufunc_t MenuCall; // 800A5584

extern int linepos; // 800A5588
extern int text_alpha_change_value; // 800A558C
extern int MusicID; // 800A5590
extern int m_actualmap; // 800A5594
extern int last_ticon; // 800a5598

extern skill_t startskill; // 800A55A0
extern int startmap; // 800A55A4
extern int UseVMU; // 800A55A8

//-----------------------------------------

extern int MenuIdx; // 8005A7A4
extern int text_alpha; // 8005A7A8
extern int ConfgNumb; // 8005A7AC
extern int Display_X; // 8005A7B0
extern int Display_Y; // 8005A7B4
//extern boolean enable_messages; // 8005A7B8
//extern int HUDopacity; // [Immorpher] HUD 0pacity options
//extern int SfxVolume; // 8005A7C0
//extern int MusVolume; // 8005A7C4
//extern int brightness; // 8005A7C8
//extern int M_SENSITIVITY; // 8005A7CC
extern const boolean FeaturesUnlocked; // 8005A7D0
//extern int MotionBob; // [Immorpher] Motion Bob
//extern int VideoFilter; // [GEC & Immorpher] VideoFilter

#define FLASH_BRIGHTNESS 16

//extern int FlashBrightness; // [Immorpher] Strobe brightness adjustment
//extern boolean Autorun; // [Immorpher] Autorun
//extern boolean runintroduction; // [Immorpher] New introduction text
//extern boolean StoryText; // [Immorpher] Enable story text
//extern boolean MapStats; // [Immorpher] Enable automap statistics
//extern int HUDmargin; // [Immorpher] HUD margin options
//extern boolean ColoredHUD; // [Immorpher] Colored hud

int M_RunTitle(void); // 80007630

int M_ControllerPak(void); // 80007724
int M_ButtonResponder(int buttons); // 80007960

void M_AlphaInStart(void); // 800079E0
void M_AlphaOutStart(void); // 800079F8
int M_AlphaInOutTicker(void); // 80007A14
void M_FadeInStart(void); // 80007AB4
void M_FadeOutStart(int exitmode); // 80007AEC

void M_SaveMenuData(void); // 80007B2C
void M_RestoreMenuData(boolean alpha_in); // 80007BB8
void M_MenuGameDrawer(void); // 80007C48
int M_MenuTicker(void); // 80007E0C
void M_MenuClearCall(int); // 80008E6C

void M_MenuTitleDrawer(void); // 80008E7C
void M_FeaturesDrawer(void); // 800091C0
void M_VolumeDrawer(void); // 800095B4
void M_MovementDrawer(void); // 80009738
void M_VideoDrawer(void); // 80009884
void M_DisplayDrawer(void); // [Immorpher] new menu
void M_StatusHUDDrawer(void); // [Immorpher] new menu
void M_DefaultsDrawer(void); // [Immorpher] new menu
void M_CreditsDrawer(void); // [Immorpher] new menu

void M_DrawBackground(d64_bg_enum_t bg, int alpha);
void M_DrawOverlay(void);

int M_ScreenTicker(void); // 8000A0F8

void M_ControllerPakDrawer(void); // 8000A3E4

void M_SavePakStart(void); // 8000A6E8
void M_SavePakStop(int); // 8000A7B4
int M_SavePakTicker(void); // 8000A804
void M_SavePakDrawer(void); // 8000AB44

void M_LoadPakStart(void); // 8000AEEC
void M_LoadPakStop(int); // 8000AF8C
int M_LoadPakTicker(void); // 8000AFE4
void M_LoadPakDrawer(void); // 8000B270

int M_CenterDisplayTicker(void); // 8000B4C4
void M_CenterDisplayDrawer(void); // 8000B604

int M_ControlPadTicker(void); // 8000B694
void M_ControlPadDrawer(void); // 8000B988

/*----------*/
/* PASSWORD */
/*----------*/

extern char *passwordChar; // 8005AC60
extern uint8_t __attribute__((aligned(32))) Passwordbuff[16]; // 800A55B0
extern boolean doPassword; // 8005ACB8
extern int CurPasswordSlot; // 8005ACBC

void M_EncodePassword(uint8_t *buff); //8000BC10
int M_DecodePassword(uint8_t *inbuff, int *levelnum, int *skill, player_t *player); // 8000C194
void M_PasswordStart(void); // 8000C710
void M_PasswordStop(int); // 8000C744
int M_PasswordTicker(void); // 8000C774
void M_PasswordDrawer(void); // 8000CAF0

/*--------*/
/* F_MAIN */
/*--------*/

void F_StartIntermission(void);
void F_StopIntermission(int);
int F_TickerIntermission(void);
void F_DrawerIntermission(void);

void F_Start(void);
void F_Stop(int);
int F_Ticker(void);
void F_Drawer(void);

void BufferedDrawSprite(int type, state_t *state, int rotframe, int color,
			int xpos, int ypos);

/*---------*/
/* AM_MAIN */
/*---------*/

void AM_Start(void);
void AM_Control(player_t *player);
void AM_Drawer(void);

/*-----------*/
/* D_SCREENS */
/*-----------*/

int D_RunDemo(char *name, skill_t skill, int map); // 8002B2D0
int D_TitleMap(void); // 8002B358
int D_WarningTicker(void); // 8002B3E8
void D_DrawWarning(void); // 8002B430
int D_LegalTicker(void); // 8002B5F8
void D_DrawLegal(void); // 8002B644
int D_NoPakTicker(void); // 8002B7A0
void D_DrawNoPak(void); // 8002B7F4
void D_SplashScreen(void); // 8002B988
int D_Credits(void); // 8002BA34
int D_CreditTicker(void); // 8002BA88
void D_CreditDrawer(void); // 8002BBE4
void D_OpenControllerPak(void); // 8002BE28

/*--------*/
/* REFRESH */
/*--------*/

void R_RenderPlayerView(void);
void R_Init(void);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
struct subsector_s *R_PointInSubsector(fixed_t x, fixed_t y);

/*------*/
/* MISC */
/*------*/
typedef struct {
	char *name;
	int MusicSeq;
} mapinfo_t;

extern mapinfo_t MapInfo[];

extern const unsigned char rndtable[256];
int M_Random(void);
int P_Random(void);
int I_Random(void); // [Immorpher] new random table
void M_ClearRandom(void);
void M_ClearBox(fixed_t *box);
void M_AddToBox(fixed_t *box, fixed_t x, fixed_t y);

/*---------*/
/* S_SOUND */
/*---------*/

/* header generated by Dave's sound utility */
#include "sounds.h"

void init_all_sounds(void);

void S_Init(void);
void S_SetSoundVolume(int volume);
void S_SetMusicVolume(int volume);
void S_StartMusic(int mus_seq);
void S_StopMusic(void);
void S_PauseSound(void);
void S_ResumeSound(void);
void S_StopSound(mobj_t *origin, int seqnum);
void S_StopAll(void);

void S_RemoveOrigin(mobj_t *origin);
void S_ResetSound(void);
void S_UpdateSounds(void);

int S_SoundStatus(int seqnum);
int S_StartSound(mobj_t *origin, int sound_id);
int S_AdjustSoundParams(mobj_t *listener, mobj_t *origin, int *vol, int *pan);

/*--------*/
/* I_MAIN */
/*--------*/

extern uint32_t vid_side;

extern boolean disabledrawing;
extern volatile int32_t vsync;
extern volatile int32_t drawsync2;
extern volatile int32_t drawsync1;
extern uint32_t NextFrameIdx;
extern int32_t FilesUsed;

void I_Start(void);
void *I_IdleGameThread(void *arg);
void *I_Main(void *arg);
void *I_SystemTicker(void *arg);
void I_Init(void);

void __attribute__((noreturn)) __I_Error(const char *funcname, char *error, ...);

#define I_Error(...) __I_Error(__func__, __VA_ARGS__)

int I_GetControllerData(void);

void I_CheckGFX(void);
void I_ClearFrame(void);
void I_DrawFrame(void);
void I_GetScreenGrab(void);

int I_CheckControllerPak(void);
int I_DeletePakFile(dirent_t *de);
int I_SavePakFile(void);
int I_ReadPakFile(void);
int I_CreatePakFile(void);

void I_WIPE_MeltScreen(void);
void I_WIPE_FadeOutScreen(void);

/*---------*/
/* Doom 64 */
/*---------*/

#define PACKRGBA(r, g, b, a) (((r) << 24) | ((g) << 16) | ((b) << 8) | (a))

/* CONTROL PAD */
#define PAD_A 0x80000000
#define PAD_DREAMCAST_X PAD_A

#define PAD_B 0x40000000
#define PAD_DREAMCAST_Y PAD_B

#define PAD_Z_TRIG 0x20000000
#define PAD_DREAMCAST_A PAD_Z_TRIG

#define PAD_START 0x10000000

#define PAD_UP 0x08000000
#define PAD_DOWN 0x04000000
#define PAD_LEFT 0x02000000
#define PAD_RIGHT 0x01000000

#define PAD_UP_C 0x00080000
#define PAD_DOWN_C 0x00040000
#define PAD_LEFT_C 0x00020000
#define PAD_RIGHT_C 0x00010000
#define PAD_DREAMCAST_B PAD_RIGHT_C

#define PAD_L_TRIG 0x00200000
#define PAD_R_TRIG 0x00100000

#define ALL_JPAD (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT)

#define ALL_CBUTTONS (PAD_UP_C | PAD_DOWN_C | PAD_LEFT_C | PAD_RIGHT_C)

#define ALL_BUTTONS													\
	(PAD_L_TRIG | PAD_R_TRIG | PAD_UP_C | PAD_DOWN_C | PAD_LEFT_C |	\
	PAD_RIGHT_C | PAD_A | PAD_B | PAD_Z_TRIG)

#define ALL_TRIG (PAD_L_TRIG | PAD_R_TRIG | PAD_Z_TRIG)

#define startupfile "\x25""s\057w\x61""r\1563\x2E""d\164"

typedef struct {
	unsigned int BT_RIGHT;
	unsigned int BT_LEFT;
	unsigned int BT_FORWARD;
	unsigned int BT_BACK;
	unsigned int BT_ATTACK;
	unsigned int BT_USE;
	unsigned int BT_MAP;
	unsigned int BT_SPEED;
	unsigned int BT_STRAFE;
	unsigned int BT_STRAFELEFT;
	unsigned int BT_STRAFERIGHT;
	unsigned int BT_WEAPONBACKWARD;
	unsigned int BT_WEAPONFORWARD;
} buttons_t;

extern buttons_t *BT_DATA[MAXPLAYERS];

typedef struct {
	short compressed;
	short numpal;
	short width;
	short height;
	uint8_t data[0];
} gfxN64_t;

typedef struct {
	short id;
	short numpal;
	short wshift;
	short hshift;
	uint8_t data[0];
} textureN64_t;

typedef struct {
	unsigned short tiles; // 0
	short compressed; // 2
	unsigned short cmpsize; // 4
	short xoffs; // 6
	short yoffs; // 8
	unsigned short width; // 10
	unsigned short height; // 12
	unsigned short tileheight; // 14
	uint8_t data[0]; // 16+	
} spriteN64_t;

typedef struct {
	unsigned short	width;
	unsigned short	height;
	short			xoffs;
	short			yoffs;
	uint8_t			data[0];
} spriteDC_t;

static inline int external_pal(int lump) {
	if (lump >= 349 && lump <= 923)
		return 1;

	return 0;
}

#define waderrstr "\x54""a\155p\x65""r\145d\x2C"" \160r\x6F""b\141b\x6C""y\040p\x69""r\141t\x65""d\056 \x54""e\154l\x20""S\143o\x74""t\040S\x74"" \107e\x6F""r\147e\x20""t\157 \x67""o\040f\x75""c\153 \x68""i\155s\x65""l\146."
