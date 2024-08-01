/* W_wad.c */

#include "doomdef.h"

/*=============== */
/*   TYPES */
/*=============== */

file_t wad_file;
file_t s2_file;

void *fullwad;
void *s2wad;

typedef struct
{
	char		identification[4];		/* should be IWAD */
	int			numlumps;
	int			infotableofs;
} wadinfo_t;

/*============= */
/* GLOBALS */
/*============= */

static lumpcache_t	*lumpcache;				//800B2220
static int			numlumps;				//800B2224
static lumpinfo_t	*lumpinfo;				//800B2228 /* points directly to rom image */

static lumpcache_t	*s2_lumpcache;
static int			s2_numlumps;
static lumpinfo_t	*s2_lumpinfo;

static int          mapnumlumps;			//800B2230 psxdoom/doom64
static lumpinfo_t   *maplump;				//800B2234 psxdoom/doom64
static byte         *mapfileptr;			//800B2238 psxdoom/doom64


/*=========*/
/* EXTERNS */
/*=========*/

/*
============================================================================

						LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= W_Init
=
====================
*/

void *pnon_enemy;

pvr_ptr_t pvr_non_enemy;
pvr_poly_cxt_t pvr_sprite_cxt;
pvr_poly_hdr_t pvr_sprite_hdr;
pvr_poly_hdr_t pvr_sprite_hdr_nofilter;

// see doomdef.h
const char *fnpre = STORAGE_PREFIX;

char fnbuf[256];
extern uint8_t __attribute__((aligned(32))) op_buf[VERTBUF_SIZE];
extern uint8_t __attribute__((aligned(32))) tr_buf[VERTBUF_SIZE];

uint16_t *printtex;
pvr_ptr_t dlstex;

void W_DrawLoadScreen(char *what, int current, int total) {
#if REAL_SCREEN_WD == 640
		pvr_vertex_t __attribute__((aligned(32))) txtverts[4];

		pvr_vertex_t __attribute__((aligned(32))) verts[12];
		pvr_poly_cxt_t load_cxt;
		pvr_poly_hdr_t load_hdr;
		pvr_poly_cxt_t load2_cxt;
		pvr_poly_hdr_t load2_hdr;
		printtex = (uint16_t *)malloc(256*32*sizeof(uint16_t));
		memset(printtex, 0, 256*32*sizeof(uint16_t));
		dlstex = pvr_mem_malloc(256*32*sizeof(uint16_t));

		pvr_poly_cxt_txr(&load_cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_ARGB1555, 256, 32, dlstex, PVR_FILTER_NONE);
		load_cxt.blend.src = PVR_BLEND_ONE;
		load_cxt.blend.dst = PVR_BLEND_ONE;
		pvr_poly_compile(&load_hdr, &load_cxt);

		pvr_poly_cxt_col(&load2_cxt, PVR_LIST_OP_POLY);
		load2_cxt.blend.src = PVR_BLEND_ONE;
		load2_cxt.blend.dst = PVR_BLEND_ONE;
		pvr_poly_compile(&load2_hdr, &load2_cxt);

		char fullstr[256];
		sprintf(fullstr, "Loading %s", what);
		bfont_set_encoding(BFONT_CODE_ISO8859_1);
		bfont_draw_str_ex(printtex, 256, 0xffffffff, 0xff000000, 16, 1, fullstr);
		pvr_txr_load_ex(printtex, dlstex, 256, 32, PVR_TXRLOAD_16BPP);

		uint32_t color = 0xff404040;
		uint32_t color2 = 0xff800000;
		uint32_t color3 = 0xffc00000;

		pvr_vertex_t *vert = txtverts;
		vert->flags = PVR_CMD_VERTEX;
		vert->x = 242.0f;
		vert->y = (((REAL_SCREEN_HT/2) - 16));
		vert->z = 4.9f;
		vert->u = 0.0f;
		vert->v = 24.0f / 32.0f;
		vert->argb = 0xffffffff;
		vert++;
		vert->flags = PVR_CMD_VERTEX;
		vert->x = 242.0f;
		vert->y = (((REAL_SCREEN_HT/2) - 16)-24);
		vert->z = 4.9f;
		vert->u = 0.0f;
		vert->v = 0.0f;
		vert->argb = 0xffffffff;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 242.0f + 256.0f;
		vert->y = (((REAL_SCREEN_HT/2) - 16));
		vert->z = 4.9f;
		vert->u = 1.0f;
		vert->v = 24.0f / 32.0f;
		vert->argb = 0xffffffff;
		vert++;

		vert->flags = PVR_CMD_VERTEX_EOL;
		vert->x = 242.0f + 256.0f;
		vert->y = (((REAL_SCREEN_HT/2) - 16)-24);
		vert->z = 4.9f;
		vert->u = 1.0f;
		vert->v = 0.0f;
		vert->argb = 0xffffffff;


		vert = verts;
		vert->flags = PVR_CMD_VERTEX;
		vert->x = 240.0f;
		vert->y = (REAL_SCREEN_HT/2) + 8;
		vert->z = 5.0f;
		vert->argb = color;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 240.0f;
		vert->y = (REAL_SCREEN_HT/2) - 16;
		vert->z = 5.0f;
		vert->argb = color;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 480.0f;
		vert->y = (REAL_SCREEN_HT/2) + 8;
		vert->z = 5.0f;
		vert->argb = color;
		vert++;

		vert->flags = PVR_CMD_VERTEX_EOL;
		vert->x = 480.0f;
		vert->y = (REAL_SCREEN_HT/2) - 16;
		vert->z = 5.0f;
		vert->argb = color;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 242.0f;
		vert->y = (REAL_SCREEN_HT/2) + 6;
		vert->z = 5.1f;
		vert->argb = color2;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 242.0f;
		vert->y = (REAL_SCREEN_HT/2) - 14;
		vert->z = 5.1f;
		vert->argb = color2;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 242.0f  + (236.0f * (float)current / (float)total);
		vert->y = (REAL_SCREEN_HT/2) + 6;
		vert->z = 5.1f;
		vert->argb = color2;
		vert++;

		vert->flags = PVR_CMD_VERTEX_EOL;
		vert->x = 242.0f  + (236.0f * (float)current / (float)total);
		vert->y = (REAL_SCREEN_HT/2) - 14;
		vert->z = 5.1f;
		vert->argb = color2;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 243.0f;
		vert->y = (REAL_SCREEN_HT/2) + 5;
		vert->z = 5.2f;
		vert->argb = color3;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 243.0f;
		vert->y = (REAL_SCREEN_HT/2) - 13;
		vert->z = 5.2f;
		vert->argb = color3;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 243.0f  + (234.0f * (float)current / (float)total);
		vert->y = (REAL_SCREEN_HT/2) + 5;
		vert->z = 5.2f;
		vert->argb = color3;
		vert++;

		vert->flags = PVR_CMD_VERTEX_EOL;
		vert->x = 243.0f  + (234.0f * (float)current / (float)total);
		vert->y = (REAL_SCREEN_HT/2) - 13;
		vert->z = 5.2f;
		vert->argb = color3;

		vid_waitvbl();
		pvr_scene_begin();
		pvr_set_vertbuf(PVR_LIST_OP_POLY, op_buf, VERTBUF_SIZE);
		pvr_set_vertbuf(PVR_LIST_TR_POLY, tr_buf, VERTBUF_SIZE);

		pvr_list_prim(PVR_LIST_OP_POLY, &load_hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(PVR_LIST_OP_POLY, &txtverts, sizeof(txtverts));	

		pvr_list_prim(PVR_LIST_OP_POLY, &load2_hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(PVR_LIST_OP_POLY, &verts, sizeof(verts));	

		pvr_scene_finish();
		pvr_wait_ready();
		free(printtex);
		pvr_mem_free(dlstex);
#endif
}

void W_Init (void)
{
	wadinfo_t *wadfileptr;
	wadinfo_t *s2_wadfileptr;
	int infotableofs;
	int s2_infotableofs;
	short *pal1;
	short *pal2;

	W_DrawLoadScreen("Palettes", 0, 100);
	timer_spin_sleep(15);
	sprintf(fnbuf, "%s/doom64monster.pal", fnpre);
	fs_load(fnbuf, (void **)&pal1);
	W_DrawLoadScreen("Palettes", 50, 100);
	timer_spin_sleep(15);
	sprintf(fnbuf, "%s/doom64nonenemy.pal", fnpre);
	fs_load(fnbuf, (void **)&pal2);
	W_DrawLoadScreen("Palettes", 100, 100);
	timer_spin_sleep(15);

	pvr_set_pal_format(PVR_PAL_ARGB1555);
	for(int i=1;i<256;i++) {
#if 1
		pvr_set_pal_entry(i, 0x8000 | pal1[i]);
#else
		// if you want to modify the enemy palette as it gets loaded
		uint8_t r = (pal1[i] >> 10) & 0x1f;
		uint8_t g = (pal1[i] >> 5) & 0x1f;
		uint8_t b = pal1[i] & 0x1f;
		int hsv = LightGetHSV(r << 3,g << 3,b << 3);
		int h = (hsv >> 16)&0xff;
		int s = (hsv >> 8)&0xff;
		int v = hsv &0xff;

		// 2% intensity increase
		v = (v * 102) / 100;
		if (v > 255)
			v = 255;
		int rgb = LightGetRGB(h,s,v);
		r = (rgb>>16)&0xff;
		g = (rgb>>8)&0xff;
		b = rgb&0xff;
		pvr_set_pal_entry(i, get_color_argb1555(r,g,b,1));
#endif
	}

	for(int i=1;i<256;i++) {
#if 1
		pvr_set_pal_entry(256 + i, 0x8000 | pal2[i]);
#else
		// if you want to modify the non-enemy palette as it gets loaded
		uint8_t r = (pal2[i] >> 10) & 0x1f;
		uint8_t g = (pal2[i] >> 5) & 0x1f;
		uint8_t b = pal2[i] & 0x1f;
		int hsv = LightGetHSV(r << 3,g << 3,b << 3);
		int h = (hsv >> 16)&0xff;
		int s = (hsv >> 8)&0xff;
		int v = hsv &0xff;

		// 2% intensity increase
		v = (v * 102) / 100;
		if (v > 255)
			v = 255;
		int rgb = LightGetRGB(h,s,v);
		r = (rgb>>16)&0xff;
		g = (rgb>>8)&0xff;
		b = rgb&0xff;
		pvr_set_pal_entry(256 + i, get_color_argb1555(r,g,b,1));
#endif
	}
	
	// color 0 is always transparent (replacing RGB ff 00 ff)
	pvr_set_pal_entry(0,0);
	pvr_set_pal_entry(256,0);

	// doom64 wad
	dbgio_printf("W_Init: Loading IWAD into RAM...\n");

	wadfileptr = (wadinfo_t *)Z_Alloc(sizeof(wadinfo_t), PU_STATIC, NULL);
	sprintf(fnbuf, "%s/pow2.wad", fnpre); // doom64.wad
	wad_file = fs_open(fnbuf, O_RDONLY);
	if (-1 == wad_file) {
		I_Error("Could not open %s for reading.\n", fnbuf);
	}

	size_t full_wad_size = fs_seek(wad_file, 0, SEEK_END);
	size_t wad_rem_size = full_wad_size;
	fullwad = malloc(wad_rem_size);
	size_t wad_read = 0;
	fs_seek(wad_file, 0, SEEK_SET);
	while(wad_rem_size > (256*1024)) {
		fs_read(wad_file, (void*)fullwad + wad_read, (256*1024));
		wad_read += (256*1024);
		wad_rem_size -= (256*1024);
		W_DrawLoadScreen("Doom 64 IWAD", wad_read, full_wad_size);
	}
	fs_read(wad_file, (void*)fullwad + wad_read, wad_rem_size);
	wad_read += wad_rem_size;
	W_DrawLoadScreen("Doom 64 IWAD", wad_read, full_wad_size);
	malloc_stats();
	dbgio_printf("Done.\n");
	fs_close(wad_file);

	memcpy((void*)wadfileptr, fullwad + 0, sizeof(wadinfo_t));
	if (D_strncasecmp(wadfileptr->identification, "IWAD", 4))
		I_Error("W_Init: invalid main IWAD id");
	numlumps = (wadfileptr->numlumps);
	lumpinfo = (lumpinfo_t *) Z_Malloc(numlumps * sizeof(lumpinfo_t), PU_STATIC, 0);
	infotableofs = (wadfileptr->infotableofs);
	memcpy((void*)lumpinfo, fullwad + infotableofs, numlumps * sizeof(lumpinfo_t));
	lumpcache = (lumpcache_t *) Z_Malloc(numlumps * sizeof(lumpcache_t), PU_STATIC, 0);
	D_memset(lumpcache, 0, numlumps * sizeof(lumpcache_t));
	Z_Free(wadfileptr);

	// alternate palette sprite wad
	dbgio_printf("W_Init: Loading alt sprite PWAD into RAM...\n");

	s2_wadfileptr = (wadinfo_t *)Z_Alloc(sizeof(wadinfo_t), PU_STATIC, NULL);
	sprintf(fnbuf, "%s/alt.wad", fnpre);
	s2_file = fs_open(fnbuf, O_RDONLY);
	if (-1 == s2_file) {
		I_Error("Could not open %s for reading.\n", fnbuf);
	}
	size_t alt_wad_size = fs_seek(s2_file, 0, SEEK_END);
	wad_rem_size = alt_wad_size;
	s2wad = malloc(wad_rem_size);
	wad_read = 0;
	fs_seek(s2_file, 0, SEEK_SET);
	while(wad_rem_size > (256*1024)) {
		fs_read(s2_file, (void*)s2wad + wad_read, (256*1024));
		wad_read += (256*1024);
		wad_rem_size -= (256*1024);
		W_DrawLoadScreen("Sprite WAD", wad_read, alt_wad_size);
	}
	fs_read(s2_file, (void*)s2wad + wad_read, wad_rem_size);
	wad_read += wad_rem_size;
	W_DrawLoadScreen("Sprite WAD", wad_read, alt_wad_size);
	dbgio_printf("Done.\n");
	fs_close(s2_file);
	malloc_stats();

	memcpy((void*)s2_wadfileptr, s2wad + 0, sizeof(wadinfo_t));
	if (D_strncasecmp(s2_wadfileptr->identification, "PWAD", 4))
		I_Error("W_Init: invalid alt sprite PWAD id");
	s2_numlumps = (s2_wadfileptr->numlumps);
	s2_lumpinfo = (lumpinfo_t *) Z_Malloc(s2_numlumps * sizeof(lumpinfo_t), PU_STATIC, 0);
	s2_infotableofs = (s2_wadfileptr->infotableofs);
	memcpy((void*)s2_lumpinfo, s2wad + s2_infotableofs, s2_numlumps * sizeof(lumpinfo_t));
	s2_lumpcache = (lumpcache_t *) Z_Malloc(s2_numlumps * sizeof(lumpcache_t), PU_STATIC, 0);
	D_memset(s2_lumpcache, 0, s2_numlumps * sizeof(lumpcache_t));
	Z_Free(s2_wadfileptr);

	// all non-enemy sprites are in an uncompressed, pretwiddled 8bpp 1024^2 sheet texture
	W_DrawLoadScreen("Item Tex", 0, 100);
	timer_spin_sleep(15);
	sprintf(fnbuf, "%s/vq/non_enemy.tex", fnpre);
	size_t vqsize = fs_load(fnbuf, &pnon_enemy);
	W_DrawLoadScreen("Item Tex", 50, 100);
	timer_spin_sleep(15);
	dbgio_printf("non_enemy loaded size is %d\n", vqsize);
	pvr_non_enemy = pvr_mem_malloc(vqsize);
	pvr_txr_load(pnon_enemy, pvr_non_enemy, vqsize);
	free(pnon_enemy);
	W_DrawLoadScreen("Item Tex", 100, 100);
	timer_spin_sleep(15);
	dbgio_printf("PVR mem free after non_enemy: %lu\n", pvr_mem_available());

	// common shared poly context/header used for all non-enemy sprites
	pvr_poly_cxt_txr(&pvr_sprite_cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(1) | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_non_enemy, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt.gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt.gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt.gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr, &pvr_sprite_cxt);
	
	pvr_poly_cxt_txr(&pvr_sprite_cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(1) | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_non_enemy, PVR_FILTER_NONE);
	pvr_sprite_cxt.gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt.gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt.gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr_nofilter, &pvr_sprite_cxt);	
}

static char retname[9];
// return human-readable "uncompressed" name for a lump number
char *W_GetNameForNum(int num)
{
	memset(retname,0,9);
    int ln_len = strlen(lumpinfo[num].name);
    if(ln_len > 8) ln_len = 8;
    memcpy(retname, lumpinfo[num].name, ln_len);
	retname[0] &= 0x7f;

	return retname;
}

/*
====================
=
= W_CheckNumForName
=
= Returns -1 if name not found
=
====================
*/
int W_CheckNumForName(char *name, int hibit1, int hibit2)
{
	lumpinfo_t      *lump_p;
	char name8[8];
	char hibit[8];
	hibit[0] = (hibit1 >> 24);
	hibit[1] = (hibit1 >> 16)&0xff;
	hibit[2] = (hibit1 >> 8)&0xff;
	hibit[3] = (hibit1 >> 0)&0xff;
	hibit[4] = (hibit2 >> 24);
	hibit[5] = (hibit2 >> 16)&0xff;
	hibit[6] = (hibit2 >> 8)&0xff;
	hibit[7] = (hibit2 >> 0)&0xff;

	memset(name8, 0, 8);
	int n_len = strlen(name);
	if(n_len > 8) n_len = 8;
	memcpy(name8, name, n_len);

	lump_p = lumpinfo;

	for (int i=0;i<numlumps;i++) {
        char lumpname[8];
		memset(lumpname, 0, 8);
        int ln_len = strlen(lump_p->name);
        if(ln_len > 8) ln_len = 8;
        memcpy(lumpname, lump_p->name, ln_len);
		lumpname[0] &= hibit[0];
		lumpname[1] &= hibit[1];
		lumpname[2] &= hibit[2];
		lumpname[3] &= hibit[3];
		lumpname[4] &= hibit[4];
		lumpname[5] &= hibit[5];
		lumpname[6] &= hibit[6];
		lumpname[7] &= hibit[7];

		int res = memcmp(name8, lumpname, 8);
		if(!res) {
			return i;
		}
		lump_p++;
	}

	return -1;
}

/*
====================
=
= W_GetNumForName
=
= Calls W_CheckNumForName, but bombs out if not found
=
====================
*/

int	W_GetNumForName (char *name) // 8002C1B8
{
	int	i;

	i = W_CheckNumForName (name, 0x7fffffff, 0xFFFFFFFF);
	if (i != -1)
		return i;

#if RANGECHECK
	I_Error ("W_GetNumForName: %s not found!",name);
#endif
	return -1;
}


/*
====================
=
= W_LumpLength
=
= Returns the buffer size needed to load the given lump
=
====================
*/

int W_LumpLength (int lump) // 8002C204
{
#if RANGECHECK
    if ((lump < 0) || (lump >= numlumps))
		I_Error ("W_LumpLength: lump %i out of range",lump);
#endif
	return lumpinfo[lump].size;
}


/*
====================
=
= W_ReadLump
=
= Loads the lump into the given buffer, which must be >= W_LumpLength()
=
====================
*/
// 256 kb buffer to replace Z_Alloc in W_ReadLump
static u64 input_w_readlump[32768]; 
static byte *input = (byte*)input_w_readlump;

void W_ReadLump (int lump, void *dest, decodetype dectype) // 8002C260
{
	lumpinfo_t *l;
	int lumpsize;
#if RANGECHECK
	if ((lump < 0) || (lump >= numlumps))
		I_Error ("W_ReadLump: lump %i out of range",lump);
#endif
	l = &lumpinfo[lump];
	if(dectype != dec_none)
	{
		if ((l->name[0] & 0x80)) /* compressed */
		{
			lumpsize = l[1].filepos - (l->filepos);
			memcpy((void*)input, fullwad + l->filepos, lumpsize);
			if (dectype == dec_jag)
				DecodeJaguar((byte *)input, (byte *)dest);
			else // dec_d64
				DecodeD64((byte *)input, (byte *)dest);

			return;
		}
	}

	if (l->name[0] & 0x80)
		lumpsize = l[1].filepos - (l->filepos);
	else
		lumpsize = (l->size);

	memcpy((void*)dest, fullwad + l->filepos, lumpsize);
}

/*
====================
=
= W_CacheLumpNum
=
====================
*/

void *W_CacheLumpNum (int lump, int tag, decodetype dectype) // 8002C430
{
	int lumpsize;
	lumpcache_t *lc;
#if RANGECHECK
	if ((lump < 0) || (lump >= numlumps))
		I_Error ("W_CacheLumpNum: lump %i out of range",lump);
#endif
	lc = &lumpcache[lump];

	if (!lc->cache)
	{	/* read the lump in */
		if (dectype == dec_none)
			lumpsize = lumpinfo[lump + 1].filepos - lumpinfo[lump].filepos;
		else
			lumpsize = lumpinfo[lump].size;

		Z_Malloc(lumpsize, tag, &lc->cache);
		W_ReadLump(lump, lc->cache, dectype);
	}
	else
	{
        if (tag & PU_CACHE) {
            Z_Touch(lc->cache);
        }
	}

	return lc->cache;
}

/*
====================
=
= W_CacheLumpName
=
====================
*/

void *W_CacheLumpName (char *name, int tag, decodetype dectype) // 8002C57C
{
	return W_CacheLumpNum (W_GetNumForName(name), tag, dectype);
}

/*
alt sprite routines
*/
/*
====================
=
= W_S2_CheckNumForName
=
= Returns -1 if name not found
=
====================
*/
int W_S2_CheckNumForName(char *name, int hibit1, int hibit2)
{
	lumpinfo_t      *lump_p;
	char name8[8];
	char hibit[8];
	hibit[0] = (hibit1 >> 24);
	hibit[1] = (hibit1 >> 16)&0xff;
	hibit[2] = (hibit1 >> 8)&0xff;
	hibit[3] = (hibit1 >> 0)&0xff;
	hibit[4] = (hibit2 >> 24);
	hibit[5] = (hibit2 >> 16)&0xff;
	hibit[6] = (hibit2 >> 8)&0xff;
	hibit[7] = (hibit2 >> 0)&0xff;

	memset(name8, 0, 8);
	int n_len = strlen(name);
	if(n_len > 8) n_len = 8;
	memcpy(name8, name, n_len);

	lump_p = s2_lumpinfo;

	for (int i=0;i<s2_numlumps;i++) {
        char lumpname[8];
		memset(lumpname, 0, 8);
        int ln_len = strlen(lump_p->name);
        if(ln_len > 8) ln_len = 8;
        memcpy(lumpname, lump_p->name, ln_len);
		lumpname[0] &= hibit[0];
		lumpname[1] &= hibit[1];
		lumpname[2] &= hibit[2];
		lumpname[3] &= hibit[3];
		lumpname[4] &= hibit[4];
		lumpname[5] &= hibit[5];
		lumpname[6] &= hibit[6];
		lumpname[7] &= hibit[7];

		int res = memcmp(name8, lumpname, 8);
		if(!res) {
			return i;
		}
		lump_p++;
	}

	return -1;
}

/*
====================
=
= W_S2_GetNumForName
=
= Calls W_S2_CheckNumForName, but bombs out if not found
=
====================
*/

int	W_S2_GetNumForName (char *name) // 8002C1B8
{
	int	i;

	i = W_S2_CheckNumForName (name, 0x7fffffff, 0xFFFFFFFF);
	if (i != -1)
		return i;

#if RANGECHECK
	I_Error ("W_S2_GetNumForName: %s not found!",name);
#endif
	return -1;
}


/*
====================
=
= W_S2_LumpLength
=
= Returns the buffer size needed to load the given lump
=
====================
*/

int W_S2_LumpLength (int lump) // 8002C204
{
#if RANGECHECK
    if ((lump < 0) || (lump >= s2_numlumps))
		I_Error ("W_S2_LumpLength: lump %i out of range",lump);
#endif
	return s2_lumpinfo[lump].size;
}


/*
====================
=
= W_S2_ReadLump
=
= Loads the lump into the given buffer, which must be >= W_S2_LumpLength()
=
====================
*/
void W_S2_ReadLump (int lump, void *dest, decodetype dectype) // 8002C260
{
	lumpinfo_t *l;
	int lumpsize;
#if RANGECHECK
	if ((lump < 0) || (lump >= s2_numlumps))
		I_Error ("W_S2_ReadLump: lump %i out of range",lump);
#endif
	l = &s2_lumpinfo[lump];
	if(dectype != dec_none)
	{
		if ((l->name[0] & 0x80)) /* compressed */
		{
			lumpsize = l[1].filepos - (l->filepos);
			memcpy((void*)input, s2wad + l->filepos, lumpsize);
			if (dectype == dec_jag)
				DecodeJaguar((byte *)input, (byte *)dest);
			else // dec_d64
				DecodeD64((byte *)input, (byte *)dest);

			return;
		}
	}

	if (l->name[0] & 0x80)
		lumpsize = l[1].filepos - (l->filepos);
	else
		lumpsize = (l->size);

	memcpy((void*)dest, s2wad + l->filepos, lumpsize);
}

/*
====================
=
= W_S2_CacheLumpNum
=
====================
*/

void *W_S2_CacheLumpNum (int lump, int tag, decodetype dectype) // 8002C430
{
	int lumpsize;
	lumpcache_t *lc;
#if RANGECHECK
	if ((lump < 0) || (lump >= s2_numlumps))
		I_Error ("W_S2_CacheLumpNum: lump %i out of range",lump);
#endif
	lc = &s2_lumpcache[lump];

	if (!lc->cache)
	{	/* read the lump in */
		if (dectype == dec_none)
			lumpsize = s2_lumpinfo[lump + 1].filepos - s2_lumpinfo[lump].filepos;
		else
			lumpsize = s2_lumpinfo[lump].size;

		Z_Malloc(lumpsize, tag, &lc->cache);
		W_S2_ReadLump(lump, lc->cache, dectype);
	}
	else
	{
        if (tag & PU_CACHE) {
            Z_Touch(lc->cache);
        }
	}

	return lc->cache;
}

/*
====================
=
= W_S2_CacheLumpName
=
====================
*/

void *W_S2_CacheLumpName (char *name, int tag, decodetype dectype) // 8002C57C
{
	return W_S2_CacheLumpNum (W_S2_GetNumForName(name), tag, dectype);
}



/*
============================================================================

MAP LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= W_OpenMapWad
=
= Exclusive Psx Doom / Doom64
====================
*/

void W_OpenMapWad(int mapnum) // 8002C5B0
{
	int lump, size, infotableofs;
	char name [8];

    name[0] = 'M';
    name[1] = 'A';
    name[2] = 'P';
    name[3] = '0' + (char)(mapnum / 10);
    name[4] = '0' + (char)(mapnum % 10);
    name[5] = 0;

    lump = W_GetNumForName(name);
    size = W_LumpLength(lump);

    mapfileptr = Z_Alloc(size, PU_STATIC, NULL);

    W_ReadLump(lump, mapfileptr, dec_d64);

    mapnumlumps = (((wadinfo_t*)mapfileptr)->numlumps);
    infotableofs = (((wadinfo_t*)mapfileptr)->infotableofs);

	maplump = (lumpinfo_t*)(mapfileptr + infotableofs);
}

/*
====================
=
= W_FreeMapLump
=
= Exclusive Doom64
====================
*/

void W_FreeMapLump(void) // 8002C748
{
    Z_Free(mapfileptr);
    mapnumlumps = 0;
}

/*
====================
=
= W_MapLumpLength
=
= Exclusive Psx Doom / Doom64
====================
*/

int W_MapLumpLength(int lump) // 8002C77C
{
#if RANGECHECK
	if (lump >= mapnumlumps)
		I_Error("W_MapLumpLength: %i out of range", lump);
#endif
	return maplump[lump].size;
}


/*
====================
=
= W_MapGetNumForName
=
= Exclusive Psx Doom / Doom64
====================
*/

int W_MapGetNumForName(char *name) // 8002C7D0
{
    char	name8[12];
	char	c, *tmp;
	int		i;
	lumpinfo_t	*lump_p;

	/* make the name into two integers for easy compares */

	*(int *)&name8[4] = 0;
	*(int *)&name8[0] = 0;

	tmp = name8;
	while ((c = *name) != 0)
	{
		*tmp++ = c;

		if ((tmp >= name8+8))
			break;

		name++;
	}

	/* scan backwards so patch lump files take precedence */

	lump_p = maplump;
	for(i = 0; i < mapnumlumps; i++) {
	        if ((*(int *)&name8[0] == (*(int *)&lump_p->name[0] & 0x7fffffff)) &&
        	    (*(int *)&name8[4] == (*(int *)&lump_p->name[4])))
                	return i;

	        lump_p++;
    	}

    return -1;
}

/*
====================
=
= W_GetMapLump
=
= Exclusive Doom64
====================
*/

void  *W_GetMapLump(int lump) // 8002C890
{
#if RANGECHECK
	if (lump >= mapnumlumps)
		I_Error("W_GetMapLump: lump %d out of range", lump);
#endif
    return (void *) ((byte *)mapfileptr + maplump[lump].filepos);
}
