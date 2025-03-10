/* P_Spec.c */
#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"
#include "st_main.h"

extern mapthing_t *spawnlist;
extern int spawncount;

line_t **linespeciallist;
int numlinespecials;

sector_t **sectorspeciallist;
int numsectorspecials;

animdef_t animdefs[MAXANIMS] =
	{ { 15, "SMONAA", 4, 7, false, false },
	  { 0, "SMONBA", 4, 1, false, false },
	  { 0, "SMONCA", 4, 7, false, false },
	  { 90, "CFACEA", 3, 3, true, false },
	  { 0, "SMONDA", 4, 3, false, false },
	  { 10, "SMONEA", 4, 7, false, false },
	  { 0, "SPORTA", 9, 3, false, true },
	  { 10, "SMONF", 5, 1, true, true },
	  { 10, "STRAKR", 5, 1, true, true },
	  { 10, "STRAKB", 5, 1, true, true },
	  { 10, "STRAKY", 5, 1, true, true },
	  { 50, "C307B", 5, 1, true, true },
	  { 0, "CTEL", 8, 3, false, true },
	  { 0, "CASFL98", 5, 7, true, true },
	  { 0, "HTELA", 4, 1, true, false } };

/*----------
/ anims[8] -> anims[MAXANIMS = 15]
/ For some reason Doom 64 is 8,
/ I will leave it at 15 to avoid problems loading data into this pointer.
/---------*/
anim_t anims[MAXANIMS], *lastanim;

card_t MapBlueKeyType;
card_t MapRedKeyType;
card_t MapYellowKeyType;

void P_AddSectorSpecial(sector_t *sec);

/*
=================
=
= P_Init
=
=================
*/

// PVR texture memory pointers for texture[texnum][palnum]
extern pvr_ptr_t **pvr_texture_ptrs;

// PVR texture memory pointer for bumpmap_texture[texnum]
extern pvr_ptr_t *bump_txr_ptr;
// PVR poly headers for each bumpmap_texture[texnum][palnum]
// for OP list
extern pvr_poly_hdr_t **bump_hdrs;

// PVR poly headers for each diffuse texture when using bumpmap
extern pvr_poly_hdr_t **txr_hdr_bump;
// PVR poly headers for each diffuse texture when not using bumpmap
extern pvr_poly_hdr_t **txr_hdr_nobump;

// make sure we always have enough space to convert textures to ARGB1555
static uint8_t tmp_pal_txr[64*64];
static uint16_t tmp_argb1555_txr[64 * 64];
static uint16_t tmp_pal[16];

extern pvr_ptr_t pvr_spritecache[MAX_CACHED_SPRITES];
extern pvr_poly_hdr_t hdr_spritecache[MAX_CACHED_SPRITES];

extern int lump_frame[575 + 310];
extern int used_lumps[575 + 310];
extern int used_lump_idx;
extern int delidx;

extern int last_flush_frame;

extern int force_filter_flush;
extern int vram_low;

// number of palettes for texture[texnum]
static unsigned int num_pal(int texnum)
{
	switch (texnum) {
	case 37:
		return 5;
	case 261:
		return 5;
	case 287:
		return 5;
	case 327:
		return 5;
	case 328:
		return 5;
	case 329:
		return 5;
	case 414:
		return 5;
	case 418:
		return 8;
	case 488:
		return 9;
	default:
		return 1;
	}
}

// flush only PVR monster sprites
void P_FlushSprites(void)
{
	// force a zone defragment whenever stuff is getting flushed, why not
	Z_Defragment(mainzone);
	force_filter_flush = 1;
	vram_low = 0;

	for (unsigned i = 0; i < ALL_SPRITES_COUNT; i++) {
		if (used_lumps[i] != -1) {
			if (pvr_spritecache[used_lumps[i]]) {
				pvr_mem_free(pvr_spritecache[used_lumps[i]]);
				pvr_spritecache[used_lumps[i]] = NULL;
			}
		}
	}

	memset(used_lumps, 0xff, sizeof(int) * ALL_SPRITES_COUNT);
	memset(lump_frame, 0xff, sizeof(int) * ALL_SPRITES_COUNT);

	used_lump_idx = 0;
	delidx = 0;
	last_flush_frame = NextFrameIdx;
}

extern pvr_ptr_t pvrsky[2];
extern int lastlump[2];
extern pvr_ptr_t pvrbg[2];
extern uint64_t lastname[2];

// flush PVR monster sprites and PVR textures AND BITMAP SKIES AND BACKGROUNDS
void P_FlushAllCached(void) {
	unsigned i, j;

	P_FlushSprites();

	// clear previously cached pvr textures
	for (i = 0; i < (unsigned)numtextures; i++) {
		// for all combo of texture + palette
		for (j = 0; j < num_pal(i); j++) {
			// check if array was allocated first
			if (pvr_texture_ptrs[i]) {
				// a non-zero value means allocated texture in array
				if (pvr_texture_ptrs[i][j])
					pvr_mem_free(pvr_texture_ptrs[i][j]);
			}
		}

		if (bump_txr_ptr[i])
			pvr_mem_free(bump_txr_ptr[i]);

		// free the array of texture pointers
		if (NULL != pvr_texture_ptrs[i]) {
			free(pvr_texture_ptrs[i]);
			// set to 0 so the following calls will start from scratch
			pvr_texture_ptrs[i] = NULL;
		}

		// free the array of contexts
		if (NULL != txr_hdr_bump[i]) {
			free(txr_hdr_bump[i]);
			txr_hdr_bump[i] = NULL;
		}

		if (NULL != txr_hdr_nobump[i]) {
			free(txr_hdr_nobump[i]);
			txr_hdr_nobump[i] = NULL;
		}

		if (NULL != bump_hdrs[i]) {
			free(bump_hdrs[i]);
			bump_hdrs[i] = NULL;
		}
	}

	memset(bump_txr_ptr, 0, sizeof(pvr_ptr_t) * numtextures);

	// possibly reclaim 256*256*2
	if (pvrsky[0]) {
		pvr_mem_free(pvrsky[0]);
		pvrsky[0] = NULL;
	}
	// possibly reclaim 256*256*2
	if (pvrsky[1]) {
		pvr_mem_free(pvrsky[1]);
		pvrsky[1] = NULL;
	}
	lastlump[0] = -1;
	lastlump[1] = -1;

	// possibly reclaim 512*256*2
	if (pvrbg[0]) {
		pvr_mem_free(pvrbg[0]);
		pvrbg[0] = NULL;
	}
	// possibly reclaim 512*256*2
	if (pvrbg[1]) {
		pvr_mem_free(pvrbg[1]);
		pvrbg[1] = NULL;
	}
	lastname[0] = 0xffffffff;
	lastname[1] = 0xffffffff;
}

static pvr_poly_cxt_t cpt_txr_cxt;
static pvr_poly_cxt_t cpt_bump_cxt;
void *P_CachePvrTexture(int i, int tag)
{
	// get texture from WAD, decompress, cache it
	void *data = W_CacheLumpNum(i + firsttex, tag, dec_d64);

	// if P_CachePvrTexture has been called for this texture before
	// pvr_texture_ptrs[i] will have been set to a non-zero value
	// texture is in PVR memory and wad cache, return early
	if (pvr_texture_ptrs[i])
		return data;

	// Doom 64 Tech Bible says this needs special handling
	// no alpha for color 0
	int slime = 0;
//	if (((W_CheckNumForName("SLIMEA") - firsttex) == i) ||
//		((W_CheckNumForName("SLIMEB") - firsttex) == i))
    if (((1325 - firsttex) == i) || ((1326 - firsttex) == i))
		slime = 1;

	// most textures have one palette
	// 9 textures have more than one (5, 8 or 9 palettes)
	short numpalfortex = SwapShort(((textureN64_t *)data)->numpal);

	// for each palette, allocate a pointer to a pvr_ptr_t
	// we are going to create an argb1555 texture for each palette
	pvr_texture_ptrs[i] = (pvr_ptr_t *)malloc(numpalfortex * sizeof(pvr_ptr_t));
	if (NULL == pvr_texture_ptrs[i])
		I_Error("could not allocate\n"
			"tex_txr_ptr array for %d\n", i);

	// for each palette, allocate a header
	// we have a header for each palette
	// we first create them for when the texture is used with bump-mapping
	// requires non-default blend settings
	txr_hdr_bump[i] = (pvr_poly_hdr_t *)memalign(32,numpalfortex * sizeof(pvr_poly_hdr_t));
	if (NULL == txr_hdr_bump[i])
		I_Error("could not allocate\n"
			"txr_hdr_bump array for %d\n", i);

	// we then create them for when the texture is used without bump-mapping
	// these use default blend settings
	txr_hdr_nobump[i] = (pvr_poly_hdr_t *)memalign(32,numpalfortex * sizeof(pvr_poly_hdr_t));
	if (NULL == txr_hdr_nobump[i])
		I_Error("could not allocate\n"
			"txr_hdr_nobump array for %d\n", i);

	// textureN64_t, unlike other Doom 64 graphics, are always pow2, thankfully
	int wshift = SwapShort(((textureN64_t *)data)->wshift);
	int hshift = SwapShort(((textureN64_t *)data)->hshift);
	unsigned width = (1 << wshift);
	unsigned height = (1 << hshift);
	// size -- 4bpp
	unsigned size = (width * height) >> 1;

	// pixels start here
	uintptr_t src = (uintptr_t)data + sizeof(textureN64_t);
	memcpy(tmp_pal_txr, (void *)src, size);

	// get the name of the given texture index
	char *bname = W_GetNameForNum(i + firsttex);
	// skip these "YOU SUCK AT MAKING MAPS" texture
	// this also skips 'BLOOD*' but we don't have those currently
	if (bname[0] != '?' && (bname[0] != 'B')) {
		// find bumpmap WAD lump number for texture name
		int bump_lumpnum = W_Bump_GetNumForName(bname);
		// not -1 means a bumpmap exists for this texture
		if (bump_lumpnum != -1) {
			// 16bpp (S,R) format
			int bumpsize = (width * height * 2);

			// allocate PVR texture memory for bumpmap
			bump_txr_ptr[i] = pvr_mem_malloc(bumpsize);
			if (!bump_txr_ptr[i]) {
				//dbgio_printf("P_CachePvrTexture code saw low vram normal map\n");
				P_FlushSprites();
				bump_txr_ptr[i] = pvr_mem_malloc(bumpsize);
				if (!bump_txr_ptr[i])
					I_Error("PVR OOM for normal map %d after sprite flush", i);
			}

			bump_hdrs[i] = (pvr_poly_hdr_t *)memalign(32,1 * sizeof(pvr_poly_hdr_t));
			if (NULL == bump_hdrs[i])
				I_Error("could not allocate\n"
					"bump_hdrs array for %d\n", i);

			// read bumpmap from WAD directly into PVR memory
			// there is decompression and twiddling happening under the hood
			W_Bump_ReadLump(bump_lumpnum, (uint8_t *)bump_txr_ptr[i], width, height);

			// PVR context for rendering a bump poly with this texture
			pvr_poly_cxt_txr(&cpt_bump_cxt, PVR_LIST_OP_POLY, D64_TBUMP, width, height, bump_txr_ptr[i], PVR_FILTER_BILINEAR);

			// settings required for bump texturing
			cpt_bump_cxt.gen.specular = PVR_SPECULAR_ENABLE;
			cpt_bump_cxt.txr.env = PVR_TXRENV_DECAL;

			pvr_poly_compile(&bump_hdrs[i][0], &cpt_bump_cxt);
		}
	}

	// already unscrambled, twiddled
	// in 8 bit format
	if ((numpalfortex == 1) && (bname[0] != '?') && !((bname[0] == 'B' && (bname[2] == 'A')))) {
		// 8BPP texture allocation in PVR memory
		pvr_texture_ptrs[i][0] = pvr_mem_malloc(width * height);
		if (!pvr_texture_ptrs[i][0]) {
			P_FlushSprites();
			pvr_texture_ptrs[i][0] = pvr_mem_malloc(width * height);
			if (!pvr_texture_ptrs[i][0])
				I_Error("PVR OOM for texture [%d][0] after sprite flush", i);
		}

		// slime rewrite 0 to 30 -- fix holes in Hangar ceiling
		if (slime) {
			uint8_t *gfxdata = (uint8_t *)(data + sizeof(textureN64_t));
			for (int g=0;g<width*height;g++) {
				if (gfxdata[g] == 0)
					gfxdata[g] = 30;
			}
		}

		pvr_txr_load((void *)src, pvr_texture_ptrs[i][0], width*height);

		// set of poly header with blend src/dst settings for bump-mapping

		if (i + firsttex >= 1300 && i + firsttex <= 1321)
			pvr_poly_cxt_txr(&cpt_txr_cxt, PVR_LIST_PT_POLY, D64_TPAL(PAL_FLAT), width, height, pvr_texture_ptrs[i][0], PVR_FILTER_BILINEAR);
		else
			pvr_poly_cxt_txr(&cpt_txr_cxt, PVR_LIST_TR_POLY, D64_TPAL(PAL_FLAT), width, height, pvr_texture_ptrs[i][0], PVR_FILTER_BILINEAR);

		// specular field holds lighting color
		cpt_txr_cxt.gen.specular = PVR_SPECULAR_ENABLE;
		// Doom 64 fog
		cpt_txr_cxt.gen.fog_type = PVR_FOG_TABLE;
		cpt_txr_cxt.gen.fog_type2 = PVR_FOG_TABLE;
		cpt_txr_cxt.blend.src = PVR_BLEND_DESTCOLOR;
		cpt_txr_cxt.blend.dst = PVR_BLEND_ZERO;
		pvr_poly_compile(&txr_hdr_bump[i][0], &cpt_txr_cxt);

		// ====================================================================
		if (i + firsttex >= 1323 && i + firsttex <= 1330) {
			// second set of poly headers with default blend src/dst settings
			// used without bump-mapping
			pvr_poly_cxt_txr(&cpt_txr_cxt, PVR_LIST_TR_POLY, D64_TPAL(PAL_FLAT), width, height, pvr_texture_ptrs[i][0], PVR_FILTER_BILINEAR);
			// specular field holds lighting color
			cpt_txr_cxt.gen.specular = PVR_SPECULAR_ENABLE;
			// Doom 64 fog
			cpt_txr_cxt.gen.fog_type = PVR_FOG_TABLE;
			cpt_txr_cxt.gen.fog_type2 = PVR_FOG_TABLE;
		} else {
			pvr_poly_cxt_txr(&cpt_txr_cxt, PVR_LIST_PT_POLY, D64_TPAL(PAL_FLAT), width, height, pvr_texture_ptrs[i][0], PVR_FILTER_BILINEAR);
			// specular field holds lighting color
			cpt_txr_cxt.gen.specular = PVR_SPECULAR_ENABLE;
			// Doom 64 fog
			cpt_txr_cxt.gen.fog_type = PVR_FOG_TABLE;
			cpt_txr_cxt.gen.fog_type2 = PVR_FOG_TABLE;
		}

		pvr_poly_compile(&txr_hdr_nobump[i][0], &cpt_txr_cxt);

		// ====================================================================
	} else  {
		// Flip nibbles per byte
		uint8_t *src8 = (uint8_t *)tmp_pal_txr;
		unsigned mask = width >> 3;
		for (unsigned k = 0; k < size; k++) {
			uint8_t tmp = src8[k];
			src8[k] = (tmp >> 4);
			src8[k] |= ((tmp & 0xf) << 4);
		}

		size >>= 2;

		// Flip each sets of dwords based on texture width
		int *src32 = (int *)tmp_pal_txr;
		for (unsigned k = 0; k < size; k += 2) {
			int x1;
			int x2;
			if (k & mask) {
				x1 = *(int *)(src32 + k);
				x2 = *(int *)(src32 + k + 1);
				*(int *)(src32 + k) = x2;
				*(int *)(src32 + k + 1) = x1;
			}
		}

		// pixels are in correct order at this point but still 4bpp

		// most textures have a single palette,
		// 494 out of 503 total
		//
		// the list of those that do not:
		// C307B has 5 palettes
		// SMONF has 5 palettes
		// SPACEAZ has 5 palettes
		// STRAKB has 5 palettes
		// STRAKR has 5 palettes
		// STRAKY has 5 palettes
		// CASFL98 has 5 palettes
		// CTEL has 8 palettes
		// SPORTA has 9 palettes

		for (unsigned k = 0; k < (unsigned)numpalfortex; k++) {
			// ARGB1555 texture allocation in PVR memory
			pvr_texture_ptrs[i][k] = pvr_mem_malloc(width * height * sizeof(uint16_t));
			if (!pvr_texture_ptrs[i][k]) {
				P_FlushSprites();

				pvr_texture_ptrs[i][k] = pvr_mem_malloc(width * height * sizeof(uint16_t));
				if (!pvr_texture_ptrs[i][k])
					I_Error("PVR OOM for texture [%d][%d] after sprite flush", i, k);
			}

			// pointer to N64 format 16-color palette for this texture/palnum combination
			// skip 4 textureN64_t fields, skip (w*h/2) bytes of pixels, skip (k*32) bytes
			// to get to palette k
			short *p = (short *)(src + (uintptr_t)((width * height) >> 1) + (uintptr_t)(k << 5));

			// these are all 16 color palettes (4bpp)
			for (unsigned j = 0; j < 16; j++) {
				short val = SwapShort(*p++);
				uint8_t r = (val & 0xF800) >> 8;
				uint8_t g = (val & 0x07C0) >> 3;
				uint8_t b = (val & 0x003E) << 2;

				// Doom 64 EX Tech Bible says this needs special handling
				// color 0 transparent only if not slime
				if (slime == 0 && j == 0 && r == 0 && g == 0 && b == 0)
					tmp_pal[j] = get_color_argb1555(0, 0, 0, 0);
				else
					tmp_pal[j] = get_color_argb1555(r, g, b, 1);
			}

			// 16-bit conversion of texture data in memory
			for (unsigned j = 0; j < (width * height); j += 2) {
				uint8_t pair_pix4bpp = src8[j >> 1];
				tmp_argb1555_txr[j    ] = tmp_pal[(pair_pix4bpp     ) & 0xf];
				tmp_argb1555_txr[j + 1] = tmp_pal[(pair_pix4bpp >> 4) & 0xf];
			}

			// optimized twiddle directly into PVR texture memory
			// take advantage of known shifts for texture size
			int twmin = MIN(width, height);
			int shiftmin = MIN(wshift, hshift);
			int twmask = twmin - 1;
			twmin *= twmin;
			uint16_t *twidbuffer = (uint16_t *)pvr_texture_ptrs[i][k];
			for (unsigned y = 0; y < height; y++) {
				unsigned yshift = y >> shiftmin;
				for (unsigned x = 0; x < width; x++) {
					twidbuffer[TWIDOUT(x & twmask, y & twmask) +
						((x >> shiftmin) + yshift) * twmin] = tmp_argb1555_txr[(y << wshift) + x];
				}
			}

			// ====================================================================

			// set of poly headers with blend src/dst settings for bump-mapping
			pvr_poly_cxt_txr(&cpt_txr_cxt, PVR_LIST_TR_POLY, D64_TARGB, width, height, pvr_texture_ptrs[i][k], PVR_FILTER_BILINEAR);

			// specular field holds lighting color
			cpt_txr_cxt.gen.specular = PVR_SPECULAR_ENABLE;
			// Doom 64 fog
			cpt_txr_cxt.gen.fog_type = PVR_FOG_TABLE;
			cpt_txr_cxt.gen.fog_type2 = PVR_FOG_TABLE;
			cpt_txr_cxt.blend.src = PVR_BLEND_DESTCOLOR;
			cpt_txr_cxt.blend.dst = PVR_BLEND_ZERO;

			pvr_poly_compile(&txr_hdr_bump[i][k], &cpt_txr_cxt);

			// ====================================================================

			// second set of poly headers with default blend src/dst settings
			// used without bump-mapping
			if (i + firsttex >= 1323 && i + firsttex <= 1330) {
				pvr_poly_cxt_txr(&cpt_txr_cxt, PVR_LIST_TR_POLY, D64_TARGB, width, height, pvr_texture_ptrs[i][k], PVR_FILTER_BILINEAR);
				// specular field holds lighting color
				cpt_txr_cxt.gen.specular = PVR_SPECULAR_ENABLE;
				// Doom 64 fog
				cpt_txr_cxt.gen.fog_type = PVR_FOG_TABLE;
				cpt_txr_cxt.gen.fog_type2 = PVR_FOG_TABLE;
			} else {
				pvr_poly_cxt_txr(&cpt_txr_cxt, PVR_LIST_PT_POLY, D64_TARGB, width, height, pvr_texture_ptrs[i][k], PVR_FILTER_BILINEAR);
				// specular field holds lighting color
				cpt_txr_cxt.gen.specular = PVR_SPECULAR_ENABLE;
				// Doom 64 fog
				cpt_txr_cxt.gen.fog_type = PVR_FOG_TABLE;
				cpt_txr_cxt.gen.fog_type2 = PVR_FOG_TABLE;
			}
			pvr_poly_compile(&txr_hdr_nobump[i][k], &cpt_txr_cxt);

			// ====================================================================
		}
	}
	return data;
}

int donebefore = 0;

extern pvr_ptr_t pvr_spritecache[MAX_CACHED_SPRITES];
extern int lump_frame[(575 + 310)];
extern int used_lumps[(575 + 310)];
extern int used_lump_idx;
extern int delidx;

void P_Init(void)
{
	unsigned i;
	sector_t *sector;
	side_t *side;

	P_FlushAllCached();

	side = sides;
	for (i = 0; i < (unsigned)numsides; i++, side++) {
		P_CachePvrTexture(side->toptexture, PU_LEVEL);
		P_CachePvrTexture(side->bottomtexture, PU_LEVEL);
		P_CachePvrTexture(side->midtexture, PU_LEVEL);
	}

	sector = sectors;
	for (i = 0; i < (unsigned)numsectors; i++, sector++) {
		if (sector->ceilingpic >= 0)
			P_CachePvrTexture(sector->ceilingpic, PU_LEVEL);

		if (sector->floorpic >= 0)
			P_CachePvrTexture(sector->floorpic, PU_LEVEL);

		if (sector->flags & MS_LIQUIDFLOOR)
			P_CachePvrTexture(sector->floorpic + 1, PU_LEVEL);
	}
}

/*
==============================================================================

							SPECIAL SPAWNING

==============================================================================
*/
/*
================================================================================
= P_SpawnSpecials
=
= After the map has been loaded, scan for specials that
= spawn thinkers
=
===============================================================================
*/
extern mobj_t *rp1_rk, *rp1_bk, *rp1_yk;

void P_SpawnSpecials(void)
{
	mobj_t *mo;
	sector_t *sector;
	line_t *line;
	int i, j;
	int lump;

	// Init animation aka (P_InitPicAnims)
	lastanim = anims;
	for (i = 0; i < MAXANIMS; i++) {
		lump = W_GetNumForName(animdefs[i].startname);

		lastanim->basepic = (lump - firsttex);
		lastanim->tics = animdefs[i].speed;
		lastanim->delay = animdefs[i].delay;
		lastanim->delaycnt = lastanim->delay;
		lastanim->f_delaycnt = lastanim->delay;
		lastanim->isreverse = animdefs[i].isreverse;

		if (animdefs[i].ispalcycle == false) {
			lastanim->current = (lump << 4);
			lastanim->picstart = (lump << 4);
			lastanim->picend = (animdefs[i].frames + lump - 1) << 4;
			lastanim->frame = 16;

			// Load the following graphics for animation
			for (j = 0; j < animdefs[i].frames; j++) {
				W_CacheLumpNum(lump, PU_LEVEL, dec_d64);
				lump++;
			}
		} else {
			lastanim->current = (lump << 4);
			lastanim->picstart = (lump << 4);
			lastanim->picend = (lump << 4) | (animdefs[i].frames - 1);
			lastanim->frame = 1;
		}

		lastanim++;
	}

	// Init Macro Variables
	activemacro = NULL;
	macrocounter = 0;
	macroidx1 = 0;
	macroidx2 = 0;

	// Init special SECTORs
	scrollfrac = 0;
	// Restart count
	numsectorspecials = 0;
	sector = sectors;
	for (i = 0; i < numsectors; i++, sector++) {
		P_AddSectorSpecial(sector);
		if (sector->flags & MS_SECRET) {
			totalsecret++;
		}

		if (sector->flags &
		    (MS_SCROLLCEILING | MS_SCROLLFLOOR | MS_SCROLLLEFT |
		     MS_SCROLLRIGHT | MS_SCROLLUP | MS_SCROLLDOWN)) {
			numsectorspecials++;
		}
	}

	sectorspeciallist = (sector_t **)Z_Malloc(numsectorspecials * sizeof(void *), PU_LEVEL, NULL);
	sector = sectors;
	for (i = 0, j = 0; i < numsectors; i++, sector++) {
		if (sector->flags &
		    (MS_SCROLLCEILING | MS_SCROLLFLOOR | MS_SCROLLLEFT |
		     MS_SCROLLRIGHT | MS_SCROLLUP | MS_SCROLLDOWN)) {
			sectorspeciallist[j] = sector;
			j++;
		}
	}

	// Init line EFFECTs
	numlinespecials = 0;
	line = lines;
	for (i = 0; i < numlines; i++, line++) {
		if (line->flags & (ML_SCROLLRIGHT | ML_SCROLLLEFT | ML_SCROLLUP | ML_SCROLLDOWN))
			numlinespecials++;
	}

	linespeciallist = (line_t **)Z_Malloc(numlinespecials * sizeof(void *), PU_LEVEL, NULL);
	line = lines;
	for (i = 0, j = 0; i < numlines; i++, line++) {
		if (line->flags & (ML_SCROLLRIGHT | ML_SCROLLLEFT | ML_SCROLLUP | ML_SCROLLDOWN)) {
			linespeciallist[j] = line;
			j++;
		}
	}

	// Init Keys
	MapBlueKeyType = it_bluecard;
	MapYellowKeyType = it_yellowcard;
	MapRedKeyType = it_redcard;
	for (mo = mobjhead.next; mo != &mobjhead; mo = mo->next) {
		if (mo->type == MT_ITEM_BLUESKULLKEY || mo->type == MT_ITEM_BLUECARDKEY)
			rp1_bk = mo;
		else if (mo->type == MT_ITEM_YELLOWSKULLKEY || mo->type == MT_ITEM_YELLOWCARDKEY)
			rp1_yk = mo;
		else if (mo->type == MT_ITEM_REDSKULLKEY || mo->type == MT_ITEM_REDCARDKEY)
			rp1_rk = mo;

		if ((mo->type == MT_ITEM_BLUESKULLKEY) || (mo->type == MT_ITEM_YELLOWSKULLKEY) || (mo->type == MT_ITEM_REDSKULLKEY)) {
			MapBlueKeyType = it_blueskull;
			MapYellowKeyType = it_yellowskull;
			MapRedKeyType = it_redskull;
			break;
		}
	}

	for (i = 0; i < spawncount; i++) {
		if ((spawnlist[i].type == 40) || (spawnlist[i].type == 39) || (spawnlist[i].type == 38)) {
			MapBlueKeyType = it_blueskull;
			MapYellowKeyType = it_yellowskull;
			MapRedKeyType = it_redskull;
			break;
		}
	}

	// Init other misc stuff
	memset(activeceilings, 0, MAXCEILINGS * sizeof(ceiling_t *));
	memset(activeplats, 0, MAXPLATS * sizeof(plat_t *));
	memset(buttonlist, 0, MAXBUTTONS * sizeof(button_t));
}

/*
==============================================================================

							UTILITIES

==============================================================================
*/

/*================================================================== */
/* */
/*	Return sector_t * of sector next to current. NULL if not two-sided line */
/* */
/*================================================================== */
sector_t *getNextSector(line_t *line, sector_t *sec)
{
	if (!(line->flags & ML_TWOSIDED)) {
		return NULL;
	}

	if (line->frontsector == sec) {
		return line->backsector;
	}

	return line->frontsector;
}

/*================================================================== */
/* */
/*	FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t P_FindLowestFloorSurrounding(sector_t *sec)
{
	int i;
	line_t *check;
	sector_t *other;
	fixed_t floor = sec->floorheight;

	for (i = 0; i < sec->linecount; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);
		if (!other) {
			continue;
		}
		if (other->floorheight < floor) {
			floor = other->floorheight;
		}
	}
	return floor;
}

/*================================================================== */
/* */
/*	FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t P_FindHighestFloorSurrounding(sector_t *sec)
{
	int i;
	line_t *check;
	sector_t *other;
	fixed_t floor = -500 * FRACUNIT;

	for (i = 0; i < sec->linecount; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);
		if (!other) {
			continue;
		}
		if (other->floorheight > floor) {
			floor = other->floorheight;
		}
	}
	return floor;
}

/*================================================================== */
/* */
/*	FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight)
{
	int i;
	int h;
	int min;
	line_t *check;
	sector_t *other;
	fixed_t height = currentheight;
	fixed_t heightlist[20]; /* 20 adjoining sectors max! */

	heightlist[0] = 0;

	for (i = 0, h = 0; i < sec->linecount; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);
		if (!other) {
			continue;
		}
		if (other->floorheight > height) {
			heightlist[h++] = other->floorheight;
		}
	}

	// Find lowest height in list
	min = heightlist[0];
	for (i = 1; i < h; i++) {
		if (heightlist[i] < min) {
			min = heightlist[i];
		}
	}
	return min;
}

/*================================================================== */
/* */
/*	FIND LOWEST CEILING IN THE SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t P_FindLowestCeilingSurrounding(sector_t *sec)
{
	int i;
	line_t *check;
	sector_t *other;
	fixed_t height = MAXINT;

	for (i = 0; i < sec->linecount; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);
		if (!other) {
			continue;
		}
		if (other->ceilingheight < height) {
			height = other->ceilingheight;
		}
	}
	return height;
}

/*================================================================== */
/* */
/*	FIND HIGHEST CEILING IN THE SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec)
{
	int i;
	line_t *check;
	sector_t *other;
	fixed_t height = 0;

	for (i = 0; i < sec->linecount; i++) {
		check = sec->lines[i];
		other = getNextSector(check, sec);
		if (!other) {
			continue;
		}
		if (other->ceilingheight > height) {
			height = other->ceilingheight;
		}
	}
	return height;
}

/*================================================================== */
/* */
/*	RETURN NEXT SECTOR # THAT LINE TAG REFERS TO */
/* */
/*================================================================== */
int P_FindSectorFromLineTag(int tag, int start)
{
	int i;

	for (i = start + 1; i < numsectors; i++) {
		if (sectors[i].tag == tag) {
			return i;
		}
	}
	return -1;
}

/*================================================================== */
/* */
/*	RETURN NEXT LIGHT # THAT LINE TAG REFERS TO */
/*	Exclusive Doom 64 */
/* */
/*================================================================== */
int P_FindLightFromLightTag(int tag, int start)
{
	int i;

	for (i = (start + 256 + 1); i < numlights; i++) {
		if (lights[i].tag == tag) {
			return i;
		}
	}
	return -1;
}

/*================================================================== */
/* */
/*	RETURN TRUE OR FALSE */
/*	Exclusive Doom 64 */
/* */
/*================================================================== */
#if RANGECHECK
bool P_ActivateLineByTag(int tag, mobj_t *thing, int level)
#else
bool P_ActivateLineByTag(int tag, mobj_t *thing)
#endif
{
	int i;
	line_t *li;

#if RANGECHECK
	// infinite recursion???
	if (level > 4) {
		arch_stk_trace(0);
		I_Error("deep recursion");
		return false;
	}
#endif	
	li = lines;
	for (i = 0; i < numlines; i++, li++) {
		if (li->tag == tag)
#if RANGECHECK		
			return P_UseSpecialLine(li, thing, level + 1);
#else
			return P_UseSpecialLine(li, thing);
#endif
	}
	return false;
}

/*
==============================================================================
							EVENTS

Events are operations triggered by using, crossing, or shooting special lines,
or by timed thinkers
==============================================================================
*/

/*
===============================================================================
P_UpdateSpecials
Animate planes, scroll walls, etc
===============================================================================
*/

#define SCROLLLIMIT (FRACUNIT * 127)

void P_UpdateSpecials(void)
{
	static int last_f_gametic = 0;
	anim_t *anim;
	line_t *line;
	sector_t *sector;
	fixed_t speed;
	int i;
	int neg;

	int update_lfg = 0;

	// ANIMATE FLATS AND TEXTURES GLOBALY
	for (anim = anims; anim < lastanim; anim++) {
		anim->f_delaycnt -= f_vblsinframe[0] * 0.5f;
		anim->delaycnt = anim->f_delaycnt;
		if ((anim->delaycnt <= 0) && !((int)f_gametic & anim->tics)) {
			if (last_f_gametic != (int)f_gametic)
				update_lfg = 1;

			anim->current += anim->frame;

			if ((anim->current < anim->picstart) || (anim->picend < anim->current)) {
				neg = -anim->frame;

				if (anim->isreverse) {
					anim->frame = neg;
					anim->current += neg;
					if (anim->delay == 0)
						anim->current += neg + neg;
				} else {
					anim->current = anim->picstart;
				}

				anim->delaycnt = anim->delay;
				anim->f_delaycnt = anim->delay;
			}

			textures[anim->basepic] = anim->current;
		}
	}

	if (update_lfg) last_f_gametic = (int)f_gametic;


	//	ANIMATE LINE SPECIALS
	for (i = 0; i < numlinespecials; i++) {
		line = linespeciallist[i];

		if (line->flags & ML_SCROLLRIGHT) {
			sides[line->sidenum[0]].textureoffset += FRACUNIT;
			sides[line->sidenum[0]].textureoffset &= SCROLLLIMIT;
		} else if (line->flags & ML_SCROLLLEFT) {
			sides[line->sidenum[0]].textureoffset -= FRACUNIT;
			sides[line->sidenum[0]].textureoffset &= SCROLLLIMIT;
		}

		if (line->flags & ML_SCROLLUP) {
			sides[line->sidenum[0]].rowoffset += FRACUNIT;
			sides[line->sidenum[0]].rowoffset &= SCROLLLIMIT;
		} else if (line->flags & ML_SCROLLDOWN) {
			sides[line->sidenum[0]].rowoffset -= FRACUNIT;
			sides[line->sidenum[0]].rowoffset &= SCROLLLIMIT;
		}
	}

	//	ANIMATE SECTOR SPECIALS
	scrollfrac = (scrollfrac + (FRACUNIT / 2)) & ((64 << FRACBITS) - 1);

	for (i = 0; i < numsectorspecials; i++) {
		sector = sectorspeciallist[i];

		if (sector->flags & MS_SCROLLFAST)
			speed = 3 * FRACUNIT;
		else
			speed = FRACUNIT;

		if (sector->flags & MS_SCROLLLEFT)
			sector->xoffset += speed;
		else if (sector->flags & MS_SCROLLRIGHT)
			sector->xoffset -= speed;

		if (sector->flags & MS_SCROLLUP)
			sector->yoffset -= speed;
		else if (sector->flags & MS_SCROLLDOWN)
			sector->yoffset += speed;

		// graphical corruption when these integer overflow
//		if (sector->flags & MS_LIQUIDFLOOR) {
		sector->xoffset &= ((64 << FRACBITS) - 1);
		sector->yoffset &= ((64 << FRACBITS) - 1);
//		}
	}

	/* */
	/*	DO BUTTONS */
	/* */
	for (i = 0; i < MAXBUTTONS; i++) {
		if (buttonlist[i].btimer > 0) {
//			buttonlist[i].btimer -= vblsinframe[0];
			// this could get sketchy
			buttonlist[i].btimer -= (int)(f_vblsinframe[0]);

			if (buttonlist[i].btimer <= 0) {
				switch (buttonlist[i].where) {
				case top:
					buttonlist[i].side->toptexture = buttonlist[i].btexture;
					break;
				case middle:
					buttonlist[i].side->midtexture = buttonlist[i].btexture;
					break;
				case bottom:
					buttonlist[i].side->bottomtexture = buttonlist[i].btexture;
					break;
				}
				S_StartSound((mobj_t *)buttonlist[i].soundorg, sfx_switch1);
				memset(&buttonlist[i], 0, sizeof(button_t));
			}
		}
	}
}

/*
==============================================================================

							UTILITIES

==============================================================================
*/

// Will return a side_t* given the number of the current sector,
// the line number, and the side (0/1) that you want.
side_t *getSide(int currentSector, int line, int side)
{
	return &sides[(sectors[currentSector].lines[line])->sidenum[side]];
}

// Will return a sector_t* given the number of the current sector,
// the line number and the side (0/1) that you want.
sector_t *getSector(int currentSector, int line, int side)
{
	return sides[(sectors[currentSector].lines[line])->sidenum[side]].sector;
}

// Given the sector number and the line number, will tell you whether
// the line is two-sided or not.
int twoSided(int sector, int line)
{
	return (sectors[sector].lines[line])->flags & ML_TWOSIDED;
}

/*
==============================================================================
							EVENTS
Events are operations triggered by using, crossing, or shooting special lines, 
or by timed thinkers
==============================================================================
*/

void P_AddSectorSpecial(sector_t *sector)
{
	if ((sector->flags & MS_SYNCSPECIALS) && (sector->special)) {
		P_CombineLightSpecials(sector);
		return;
	}

	switch (sector->special) {
	case 0:
		sector->lightlevel = 0;
		break;

	case 1:
		/* FLICKERING LIGHTS */
		P_SpawnLightFlash(sector);
		break;

	case 2:
		/* STROBE FAST */
		P_SpawnStrobeFlash(sector, FASTDARK);
		break;

	case 3:
		/* STROBE SLOW */
		P_SpawnStrobeFlash(sector, SLOWDARK);
		break;

	case 8:
		/* GLOWING LIGHT */
		P_SpawnGlowingLight(sector, PULSENORMAL);
		break;

	case 9:
		P_SpawnGlowingLight(sector, PULSESLOW);
		break;

	case 11:
		P_SpawnGlowingLight(sector, PULSERANDOM);
		break;

	case 17:
		P_SpawnFireFlicker(sector);
		break;

	case 202:
		P_SpawnStrobeAltFlash(sector, 3);
		break;

	case 204:
		P_SpawnStrobeFlash(sector, 7);
		break;

	case 205:
		P_SpawnSequenceLight(sector, true);
		break;

	case 206:
		P_SpawnStrobeFlash(sector, 90);
		break;

	case 208:
		P_SpawnStrobeAltFlash(sector, 6);
		break;

	case 666:
		break;
	}
}

/*
==============================================================================
P_UseSpecialLine
Called when a thing uses a special line
Only the front sides of lines are usable
===============================================================================
*/
#if RANGECHECK
bool P_UseSpecialLine(line_t *line, mobj_t *thing, int level)
#else
bool P_UseSpecialLine(line_t *line, mobj_t *thing)
#endif
{
	player_t *player;
	bool ok;
	int actionType;

#if RANGECHECK
	// infinite recursion????
//	if (level > 9) return false;
	if (level > 4) {
		arch_stk_trace(0);
		I_Error("deep recursion");
		return false;
	}
#endif

	actionType = SPECIALMASK(line->special);

	if (actionType == 0) {
		return false;
	}

	player = thing->player;

	// Switches that other things can activate
	if (!player) {
		// Missiles should NOT trigger specials...
		if (thing->flags & MF_MISSILE) {
			return false;
		}

		if (!(line->flags & ML_THINGTRIGGER)) {
			// never open secret doors
			if (line->flags & ML_SECRET) {
				return false;
			}

			// never allow a non-player mobj to use lines with these useflags
			if (line->special & (MLU_BLUE | MLU_YELLOW | MLU_RED)) {
				return false;
			}

			/*
				actionType == 1 // MANUAL DOOR RAISE
				actionType == 2 // OPEN DOOR IMPACT
				actionType == 4 // RAISE DOOR
				actionType == 10 // PLAT DOWN-WAIT-UP-STAY TRIGGER
				actionType == 39 // TELEPORT TRIGGER
				actionType == 125 // TELEPORT MONSTERONLY TRIGGER
			*/

			if (!((line->special & MLU_USE && actionType == 1) ||
			      (line->special & MLU_CROSS &&
			       (actionType == 4 || actionType == 10 ||
				actionType == 39 || actionType == 125)) ||
			      (line->special & MLU_SHOOT && actionType == 2))) {
				return false;
			}
		}
	} else {
		// Blue Card Lock
		if (line->special & MLU_BLUE) {
			if (!player->cards[it_bluecard] &&
			    !player->cards[it_blueskull]) {
				player->message = "You need a blue key.";
				player->messagetic = MSGTICS;
				player->messagecolor = 0x0080ff00;
				S_StartSound(thing, sfx_oof);

				if (player == &players[0]) {
					tryopen[MapBlueKeyType] = true;
				}
				return true;
			}
		}

		// Yellow Card Lock
		if (line->special & MLU_YELLOW) {
			if (!player->cards[it_yellowcard] &&
			    !player->cards[it_yellowskull]) {
				player->message = "You need a yellow key.";
				player->messagetic = MSGTICS;
				player->messagecolor = 0xC4C40000;
				S_StartSound(thing, sfx_oof);

				if (player == &players[0]) {
					tryopen[MapYellowKeyType] = true;
				}

				return true;
			}
		}

		// Red Card Lock
		if (line->special & MLU_RED) {
			if (!player->cards[it_redcard] &&
			    !player->cards[it_redskull]) {
				player->message = "You need a red key.";
				player->messagetic = MSGTICS;
				player->messagecolor = 0xff404000;
				S_StartSound(
					thing,
					sfx_oof);

				if (player == &players[0]) {
					tryopen[MapRedKeyType] = true;
				}

				return true;
			}
		}

		/*
			actionType == 90 // ARTIFACT SWITCH 1
			actionType == 91 // ARTIFACT SWITCH 2
			actionType == 92 // ARTIFACT SWITCH 3
		*/

		if ((actionType == 90 || actionType == 91 ||
		     actionType == 92) &&
		    (((player->artifacts & 1) << ((actionType + 6) & 0x1f)) ==
		     0)) {
			player->message =
				"You lack the ability to activate it.";
			player->messagetic = MSGTICS;
			player->messagecolor = 0xC4C4C400;
			S_StartSound(thing, sfx_oof);

			return false;
		}
	}

	if (actionType >= 256) {
		return P_StartMacro(actionType, line, thing);
	}

	ok = false;

	// do something
	switch (SPECIALMASK(line->special)) {
	case 1: /* Vertical Door */
	case 31: /* Manual Door Open */
	case 117: /* Blazing Door Raise */
	case 118: /* Blazing Door Open */
		EV_VerticalDoor(line, thing);
		ok = true;
		break;
	case 2: /* Open Door */
		ok = EV_DoDoor(line, DoorOpen);
		break;
	case 3: /* Close Door */
		ok = EV_DoDoor(line, DoorClose);
		break;
	case 4: /* Raise Door */
		ok = EV_DoDoor(line, Normal);
		break;
	case 5: /* Raise Floor */
		ok = EV_DoFloor(line, raiseFloor, FLOORSPEED);
		break;
	case 6: /* Fast Ceiling Crush & Raise */
		ok = EV_DoCeiling(line, fastCrushAndRaise, CEILSPEED * 2);
		break;
	case 8: /* Build Stairs */
		ok = EV_BuildStairs(line, build8);
		break;
	case 10: /* PlatDownWaitUp */
		ok = EV_DoPlat(line, downWaitUpStay, 0);
		break;
	case 16: /* Close Door 30 */
		ok = EV_DoDoor(line, Close30ThenOpen);
		break;
	case 17: /* Start Light Strobing */
		ok = EV_StartLightStrobing(line);
		break;
	case 19: /* Lower Floor */
		ok = EV_DoFloor(line, lowerFloor, FLOORSPEED);
		break;
	case 22: /* Raise floor to nearest height and change texture */
		ok = EV_DoPlat(line, raiseToNearestAndChange, 0);
		break;
	case 25: /* Ceiling Crush and Raise */
		ok = EV_DoCeiling(line, crushAndRaise, CEILSPEED);
		break;
	case 30: // Raise floor to shortest texture height on either side of lines
		ok = EV_DoFloor(line, raiseToTexture, FLOORSPEED);
		break;
	case 36: /* Lower Floor (TURBO) */
		ok = EV_DoFloor(line, turboLower, FLOORSPEED * 4);
		break;
	case 37: /* LowerAndChange */
		ok = EV_DoFloor(line, lowerAndChange, FLOORSPEED);
		break;
	case 38: /* Lower Floor To Lowest */
		ok = EV_DoFloor(line, lowerFloorToLowest, FLOORSPEED);
		break;
	case 39: /* TELEPORT! */
		EV_Teleport(line, thing);
		ok = false;
		break;
	case 43: /* Lower Ceiling to Floor */
		ok = EV_DoCeiling(line, lowerToFloor, CEILSPEED);
		break;
	case 44: /* Ceiling Crush */
		ok = EV_DoCeiling(line, lowerAndCrush, CEILSPEED);
		break;
	case 52: /* EXIT! */
		P_ExitLevel(); //G_ExitLevel
		ok = true;
		break;
	case 53: /* Perpetual Platform Raise */
		ok = EV_DoPlat(line, perpetualRaise, 0);
		break;
	case 54: /* Platform Stop */
		ok = EV_StopPlat(line);
		break;
	case 56: /* Raise Floor Crush */
		ok = EV_DoFloor(line, raiseFloorCrush, FLOORSPEED);
		break;
	case 57: /* Ceiling Crush Stop */
		ok = EV_CeilingCrushStop(line);
		break;
	case 58: /* Raise Floor 24 */
		ok = EV_DoFloor(line, raiseFloor24, FLOORSPEED);
		break;
	case 59: /* Raise Floor 24 And Change */
		ok = EV_DoFloor(line, raiseFloor24AndChange, FLOORSPEED);
		break;
	case 66: /* Raise Floor 24 and change texture */
		ok = EV_DoPlat(line, raiseAndChange, 24);
		break;
	case 67: /* Raise Floor 32 and change texture */
		ok = EV_DoPlat(line, raiseAndChange, 32);
		break;
	case 90: /* Artifact Switch 1 */
	case 91: /* Artifact Switch 2 */
	case 92: /* Artifact Switch 3 */
#if RANGECHECK	
		ok = P_ActivateLineByTag(line->tag + 1, thing, level + 1);
#else
		ok = P_ActivateLineByTag(line->tag + 1, thing);
#endif
		break;
	case 93: /* Modify mobj flags */
		ok = P_ModifyMobjFlags(line->tag, MF_NOINFIGHTING);
		break;
	case 94: /* Noise Alert */
		ok = P_AlertTaggedMobj(line->tag, thing);
		break;
	case 100: /* Build Stairs Turbo 16 */
		ok = EV_BuildStairs(line, turbo16);
		break;
	case 108: /* Blazing Door Raise (faster than TURBO!) */
		ok = EV_DoDoor(line, BlazeRaise);
		break;
	case 109: /* Blazing Door Open (faster than TURBO!) */
		ok = EV_DoDoor(line, BlazeOpen);
		break;
	case 110: /* Blazing Door Close (faster than TURBO!) */
		ok = EV_DoDoor(line, BlazeClose);
		break;
	case 119: /* Raise floor to nearest surr. floor */
		ok = EV_DoFloor(line, raiseFloorToNearest, FLOORSPEED);
		break;
	case 121: /* Blazing PlatDownWaitUpStay */
		ok = EV_DoPlat(line, blazeDWUS, 0);
		break;
	case 122: /* PlatDownWaitUpStay */
		ok = EV_DoPlat(line, upWaitDownStay, 0);
		break;
	case 123: /* Blazing PlatUpWaitDownStay */
		ok = EV_DoPlat(line, blazeUWDS, 0);
		break;
	case 124: /* Secret EXIT */
		P_SecretExitLevel(line->tag); //(G_SecretExitLevel)
		ok = true;
		break;
	case 125: /* TELEPORT MonsterONLY */
		if (!thing->player) {
			EV_Teleport(line, thing);
			ok = false;
		}
		break;
	case 141: /* Silent Ceiling Crush & Raise (Demon Disposer)*/
		ok = EV_DoCeiling(line, silentCrushAndRaise, CEILSPEED * 2);
		break;
	case 200: /* Set Lookat Camera */
		ok = P_SetAimCamera(line, true);
		break;
	case 201: /* Set Camera */
		ok = P_SetAimCamera(line, false);
		break;
	case 202: /* Invoke Dart */
		ok = EV_SpawnTrapMissile(line, thing, MT_PROJ_DART);
		break;
	case 203: /* Delay Thinker */
		P_SpawnDelayTimer(line->tag, NULL);
		ok = true;
		break;
	case 204: /* Set global integer */
		macrointeger = line->tag;
		ok = true;
		break;
	case 205: /* Modify sector color */
		P_ModifySectorColor(line, LIGHT_FLOOR, macrointeger);
		ok = true;
		break;
	case 206: /* Modify sector color */
		ok = P_ModifySectorColor(line, LIGHT_CEILING, macrointeger);
		break;
	case 207: /* Modify sector color */
		ok = P_ModifySectorColor(line, LIGHT_THING, macrointeger);
		break;
	case 208: /* Modify sector color */
		ok = P_ModifySectorColor(line, LIGHT_UPRWALL, macrointeger);
		break;
	case 209: /* Modify sector color */
		ok = P_ModifySectorColor(line, LIGHT_LWRWALL, macrointeger);
		break;
	case 210: /* Modify sector ceiling height */
		ok = EV_DoCeiling(line, customCeiling, CEILSPEED);
		break;
	case 212: /* Modify sector floor height */
		ok = EV_DoFloor(line, customFloor, FLOORSPEED);
		break;
	case 214: /* Elevator Sector */
		ok = EV_SplitSector(line, true);
		break;
	case 218: /* Modify Line Flags */
		ok = P_ModifyLineFlags(line, macrointeger);
		break;
	case 219: /* Modify Line Texture */
		ok = P_ModifyLineTexture(line, macrointeger);
		break;
	case 220: /* Modify Sector Flags */
		ok = P_ModifySector(line, macrointeger, mods_flags);
		break;
	case 221: /* Modify Sector Specials */
		ok = P_ModifySector(line, macrointeger, mods_special);
		break;
	case 222: /* Modify Sector Lights */
		ok = P_ModifySector(line, macrointeger, mods_lights);
		break;
	case 223: /* Modify Sector Flats */
		ok = P_ModifySector(line, macrointeger, mods_flats);
		break;
	case 224: /* Spawn Thing */
		ok = EV_SpawnMobjTemplate(line->tag);
		break;
	case 225: /* Quake Effect */
		P_SpawnQuake(line->tag);
		ok = true;
		break;
	case 226: /* Modify sector ceiling height */
		ok = EV_DoCeiling(line, customCeiling, CEILSPEED * 4);
		break;
	case 227: /* Modify sector ceiling height */
		ok = EV_DoCeiling(line, customCeiling, 4096 * FRACUNIT);
		break;
	case 228: /* Modify sector floor height */
		ok = EV_DoFloor(line, customFloor, FLOORSPEED * 4);
		break;
	case 229: /* Modify sector floor height */
		ok = EV_DoFloor(line, customFloor, 4096 * FRACUNIT);
		break;
	case 230: /* Modify Line Special */
		ok = P_ModifyLineData(line, macrointeger);
		break;
	case 231: /* Invoke Revenant Missile */
		ok = EV_SpawnTrapMissile(line, thing, MT_PROJ_TRACER);
		break;
	case 232: /* Fast Ceiling Crush & Raise */
		ok = EV_DoCeiling(line, crushAndRaiseOnce, CEILSPEED * 4);
		break;
	case 233: /* Freeze Player */
		thing->reactiontime = line->tag;
		ok = true;
		break;
	case 234: /* Change light by light tag */
		ok = P_ChangeLightByTag(macrointeger, line->tag);
		break;
	case 235: /* Modify Light Data */
		ok = P_DoSectorLightChange(macrointeger, line->tag);
		break;
	case 236: /* Modify platform */
		ok = EV_DoPlat(line, customDownUp, 0);
		break;
	case 237: /* Modify platform */
		ok = EV_DoPlat(line, customDownUpFast, 0);
		break;
	case 238: /* Modify platform */
		ok = EV_DoPlat(line, customUpDown, 0);
		break;
	case 239: /* Modify platform */
		ok = EV_DoPlat(line, customUpDownFast, 0);
		break;
	case 240: /* Execute random line trigger */
		ok = P_RandomLineTrigger(line, thing);
		break;
	case 241: /* Split Open Sector */
		ok = EV_SplitSector(line, false);
		break;
	case 242: /* Fade Out Thing */
		ok = EV_FadeOutMobj(line->tag);
		break;
	case 243: /* Move and Aim Camera */
		P_SetMovingCamera(line);
		ok = false;
		break;
	case 244: /* Modify Sector Floor */
		ok = EV_DoFloor(line, customFloorToHeight, 4096 * FRACUNIT);
		break;
	case 245: /* Modify Sector Ceiling */
		ok = EV_DoCeiling(line, customCeilingToHeight, 4096 * FRACUNIT);
		break;
	case 246: /* Restart Macro at ID */
		P_RestartMacro(line, macrointeger);
		ok = false;
		break;
	case 247: /* Modify Sector Floor */
		ok = EV_DoFloor(line, customFloorToHeight, FLOORSPEED);
		break;
	case 248: /* Suspend a macro script */
		ok = P_SuspendMacro();
		break;
	case 249: /* Silent Teleport */
		ok = EV_SilentTeleport(line, thing);
		break;
	case 250: /* Toggle macros on */
		P_ToggleMacros(line->tag, true);
		ok = true;
		break;
	case 251: /* Toggle macros off */
		P_ToggleMacros(line->tag, false);
		ok = true;
		break;
	case 252: /* Modify Sector Ceiling */
		ok = EV_DoCeiling(line, customCeilingToHeight, CEILSPEED);
		break;
	case 253: /* Unlock Cheat Menu */
//		if (!demoplayback) {
//			FeaturesUnlocked = true;
//		}
		ok = true;
		break;
	case 254: /* D64 Map33 Logo */
		Skyfadeback = true;
		break;

	default:
		return false;
	}

	if (ok) {
		if (line == &macrotempline) {
			return true;
		}

		P_ChangeSwitchTexture(line, line->special & MLU_REPEAT);

		if (line->special & MLU_REPEAT) {
			return true;
		}

		line->special = 0;
	}

	return true;
}
