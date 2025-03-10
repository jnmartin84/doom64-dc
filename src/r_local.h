/* R_local.h */

#ifndef __R_LOCAL__
#define __R_LOCAL__

/* proper screen size would be 160*100, stretched to 224 is 2.2 scale */
#define STRETCH (22 * FRACUNIT / 10)

#define CENTERX (SCREENWIDTH / 2)
#define CENTERY (SCREENHEIGHT / 2)
#define CENTERXFRAC (SCREENWIDTH / 2 * FRACUNIT)
#define CENTERYFRAC (SCREENHEIGHT / 2 * FRACUNIT)
#define PROJECTION CENTERXFRAC

#define ANGLETOSKYSHIFT 22 /* sky map is 256*128*4 maps */

#define BASEYCENTER 100

#define CENTERY (SCREENHEIGHT / 2)
#define WINDOWHEIGHT (SCREENHEIGHT - SBARHEIGHT)

#define MINZ 8
#define MAXZ 256

#define FIELDOFVIEW 2048 /* fineangles in the SCREENWIDTH wide window */

/* */
/* Seg flags */
/* */
#define SGF_VISIBLE_COLS \
	1 /* The seg has at least 1 visible (non fully occluded column) */

/* */
/* lighting constants */
/* */
#define LIGHTLEVELS 256 /* number of diminishing */
#define INVERSECOLORMAP 255

/*
==============================================================================

					INTERNAL MAP TYPES

==============================================================================
*/

/*================ used by play and refresh */

typedef struct {
	fixed_t x, y, dx, dy;
} divline_t;

typedef struct {
	fixed_t x, y;
	fixed_t vx, vy;
	int validcount;
} vertex_t;

typedef struct {
	float x, y;
} fvertex_t;


struct line_s;
struct subsector_s;

typedef struct {
	fixed_t floorheight, ceilingheight;
	fixed_t old_floorheight, old_ceilingheight;
	int floorpic, ceilingpic; /* if ceilingpic == -1,draw sky */
	int colors[5]; // Doom 64 New
	int lightlevel;
	int special, tag;

	int xoffset, yoffset; // Doom 64 New

	int soundtraversed; /* 0 = untraversed, 1,2 = sndlines -1 */
	mobj_t *soundtarget; /* thing that made a sound (or null) */

	int flags; // Psx Doom / Doom 64 New
	int blockbox[4]; /* mapblock bounding box for height changes */
	degenmobj_t soundorg; /* for any sounds played by the sector */

	int validcount; /* if == validcount, already checked */
	mobj_t *thinglist; /* list of mobjs in sector */
	void *specialdata; /* thinker_t for reversable actions */
	int linecount;
	struct line_s **lines; /* [linecount] size */
} sector_t;

typedef struct {
	fixed_t textureoffset; /* add this to the calculated texture col */
	fixed_t rowoffset; /* add this to the calculated texture top */
	int toptexture, bottomtexture, midtexture;
	sector_t *sector;
} side_t;

typedef enum {
	ST_HORIZONTAL,
	ST_VERTICAL,
	ST_POSITIVE,
	ST_NEGATIVE
} slopetype_t;

typedef struct line_s {
	vertex_t *v1, *v2;
	fixed_t dx, dy; /* v2 - v1 for side checking */
	int flags;
	int special, tag;
	int sidenum[2]; /* sidenum[1] will be -1 if one sided */
	fixed_t bbox[4];
	slopetype_t slopetype; /* to aid move clipping */
	sector_t *frontsector, *backsector;
	int validcount; /* if == validcount, already checked */
	void *specialdata; /* thinker_t for reversable actions */
	int fineangle; /* to get sine / cosine for sliding */
} line_t;

typedef struct vissprite_s {
	int zdistance; // *
	mobj_t *thing; // * 4
	int lump; // * 8
	bool flip; // * 12
	sector_t *sector; // * 16
	struct vissprite_s *next; // * 20
} vissprite_t;

typedef struct subsector_s {
	sector_t *sector;		//  0 -> 4
	vissprite_t *vissprite;		//  4 -> 8
	short numlines;			//  8 -> 10
	short firstline;		// 10 -> 12
	short numverts;			// 12 -> 14
	short leaf;			// 14 -> 16
	short drawindex;		// 16 -> 18
	short index;			// 18 -> 20
	short is_split;			// 20 -> 22
	short pad1;			// 22 -> 24
	unsigned lit;			// 24 -> 28
	fixed_t bbox[4];		// 28 -> 44  24 + 8 = 32
	unsigned pad2;			// 44 -> 48
} subsector_t;

typedef struct seg_s {
	vertex_t *v1, *v2;
	fixed_t offset;
	angle_t angle; /* this is not used (keep for padding) */
	side_t *sidedef;
	line_t *linedef;
	sector_t *frontsector;
	sector_t *backsector; /* NULL for one sided lines */
	short flags;
	short length;

	float nx;
	float nz;
} seg_t;

typedef struct {
	divline_t line; /* partition line */
	fixed_t bbox[2][4]; /* bounding box for each child */
	int children[2]; /* if NF_SUBSECTOR its a subsector */
} node_t;

typedef struct {
	vertex_t *vertex;
	seg_t *seg;
} leaf_t;

//
// Light Data Doom64
//
typedef struct {
	int rgba;
	int tag;
} light_t;

//
// Macros Doom64
//
typedef struct {
	int id, tag, special;
} macro_t;

/*
==============================================================================

						OTHER TYPES

==============================================================================
*/

/* Sprites are patches with a special naming convention so they can be  */
/* recognized by R_InitSprites.  The sprite and frame specified by a  */
/* thing_t is range checked at run time. */
/* a sprite is a patch_t that is assumed to represent a three dimensional */
/* object and may have multiple rotations pre drawn.  Horizontal flipping  */
/* is used to save space. Some sprites will only have one picture used */
/* for all views.   */

typedef struct {
	bool rotate; /* if false use 0 for any position */
	int lump[8]; /* lump to use for view angles 0-7 */
	uint8_t flip[8]; /* flip (1 = flip) to use for view angles 0-7 */
} spriteframe_t;

typedef struct {
	spriteframe_t *spriteframes;
	int numframes;
} spritedef_t;

extern spritedef_t sprites[NUMSPRITES];

/*
===============================================================================

							MAP DATA

===============================================================================
*/

extern int numvertexes;
extern vertex_t *vertexes;

extern int numsegs;
extern seg_t *segs;

extern int numsectors;
extern sector_t *sectors;

extern int numsubsectors;
extern subsector_t *subsectors;

extern int numnodes;
extern node_t *nodes;

extern int numlines;
extern line_t *lines;

extern int numsides;
extern side_t *sides;

extern int numleafs;
extern leaf_t *leafs;

extern light_t *lights;
extern int numlights;

extern macro_t **macros;
extern int nummacros;

/*============================================================================= */

/*------*/
/*R_main*/
/*------*/
extern mobj_t *cameratarget;
extern angle_t camviewpitch;
extern fixed_t scrollfrac;
extern sector_t *frontsector;
extern int globallump;
extern int globalcm;
extern int infraredFactor;
extern int FlashEnvColor;
extern fixed_t quakeviewx;
extern fixed_t quakeviewy;

/*------*/
/*R_data*/
/*------*/
void R_InitData(void);

/*--------*/
/*r_phase1*/
/*--------*/
void R_BSP(void);
void R_RenderBSPNode(int bspnum);

/*--------*/
/*r_phase2*/
/*--------*/
typedef void (*skyfunc_t)();
extern fixed_t FogNear;
extern int FogColor;
extern skyfunc_t R_RenderSKY;
extern int Skyfadeback;

void R_SetupSky(void);

/*--------*/
/*r_phase3*/
/*--------*/
void R_RenderAll(void);
void R_RenderPSprites(void);

/* to get a global angle from cartesian coordinates, the coordinates are */
/* flipped until they are in the first octant of the coordinate system, then */
/* the y (<=x) is scaled and divided by x to get a tangent (slope) value */
/* which is looked up in the tantoangle() table.  The +1 size is to handle */
/* the case when x==y without additional checking. */
#define SLOPERANGE 2048
#define SLOPEBITS 11
#define DBITS (FRACBITS - SLOPEBITS)

#define VIEW_3D_H 200

#define HEIGHTBITS 6
#define SCALEBITS 9

#define FIXEDTOSCALE (FRACBITS - SCALEBITS)
#define FIXEDTOHEIGHT (FRACBITS - HEIGHTBITS)

#define HALF_SCREEN_W (SCREENWIDTH / 2)

extern fixed_t viewx, viewy, viewz;
extern angle_t viewangle;
extern fixed_t viewcos, viewsin;

extern player_t *viewplayer;

extern int validcount;

/* */
/* R_data.c */
/* */
extern bool rendersky;
#define SOLIDCOLSC 2560
extern uint8_t __attribute__((aligned(32))) solidcols[SOLIDCOLSC];

#if SOLIDCOLSC == 320
#define XOYSCALE 9
#elif SOLIDCOLSC == 640
#define XOYSCALE 8
#elif SOLIDCOLSC == 1280
#define XOYSCALE 7
#elif SOLIDCOLSC == 2560
#define XOYSCALE 6
#elif SOLIDCOLSC == 5120
#define XOYSCALE 5
#elif SOLIDCOLSC == 10240
#define XOYSCALE 4
#endif

/* Maximum number of subsectors to scan */
#define MAXSUBSECTORS 512

/* List of valid ranges to scan through */
extern subsector_t *solidsubsectors[MAXSUBSECTORS];
/* Pointer to the first free entry */
extern subsector_t **endsubsector;
extern int numdrawsubsectors;

#define MAXVISSPRITES 256
extern vissprite_t vissprites[MAXVISSPRITES];
extern vissprite_t *visspritehead;
extern int numdrawvissprites;

extern int firsttex, lasttex, numtextures, firstswx;
extern int *textures;
extern int skytexture;

extern int firstsprite, lastsprite, numsprites;

#endif /* __R_LOCAL__ */
