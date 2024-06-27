/* W_wad.c */

#include "doomdef.h"

void W_OpenMusWad(void);

/*=============== */
/*   TYPES */
/*=============== */

file_t wad_file;
file_t map_file;

void *fullwad;

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
void *pplaytrooposs;
void *pskulbosshead;
void *ppainbsp;
void *pfattrect1, *pfattrect2, *pfattrect3;
void *psargfirstcybr, *premcybr;
void *pbarniteshot, *pspectre, *pwepnshee;

pvr_ptr_t pvr_non_enemy;
pvr_ptr_t pvr_playtrooposs;
pvr_ptr_t pvr_skulbosshead;
pvr_ptr_t pvr_painbsp;
pvr_ptr_t pvr_fattrect1, pvr_fattrect2, pvr_fattrect3;
pvr_ptr_t pvr_sargfirstcybr,pvr_remcybr;
pvr_ptr_t pvr_barniteshot,pvr_spectre,pvr_wepnshee;
pvr_poly_cxt_t pvr_sprite_cxt[11];
pvr_poly_hdr_t pvr_sprite_hdr[11];

const char *fnpre = "/cd";

char fnbuf[256];

void W_Init (void)
{
	wadinfo_t *wadfileptr;
	int infotableofs;

	short *pal1;
	sprintf(fnbuf, "%s/doom64monster.pal", fnpre);
	fs_load(fnbuf, (void **)&pal1);

	short *pal2;
	sprintf(fnbuf, "%s/doom64nonenemy.pal", fnpre);
	fs_load(fnbuf, (void **)&pal2);

	pvr_set_pal_format(PVR_PAL_ARGB1555);
	for(int i=1;i<256;i++) {
#if 0
		uint8_t r = (pal1[i] >> 10) & 0x1f;
		uint8_t g = (pal1[i] >> 5) & 0x1f;
		uint8_t b = pal1[i] & 0x1f;
//		if (r && g && b) {
			int hsv = LightGetHSV(r << 3,g << 3,b << 3);
			int h = (hsv >> 16)&0xff;
			int s = (hsv >> 8)&0xff;
			int v = hsv &0xff;

			v = (v * 102) / 100;
			if (v > 255)
				v = 255;
			int rgb = LightGetRGB(h,s,v);
			r = (rgb>>16)&0xff;
			g = (rgb>>8)&0xff;
			b = rgb&0xff;
//		}
		pvr_set_pal_entry(i, get_color_argb1555(r,g,b,1));//pal1[i]);
#endif
		pvr_set_pal_entry(i, pal1[i]);
	}
	for(int i=1;i<256;i++) {
#if 0
		uint8_t r = (pal2[i] >> 10) & 0x1f;
		uint8_t g = (pal2[i] >> 5) & 0x1f;
		uint8_t b = pal2[i] & 0x1f;
//		if (r && g && b) {
			int hsv = LightGetHSV(r << 3,g << 3,b << 3);
			int h = (hsv >> 16)&0xff;
			int s = (hsv >> 8)&0xff;
			int v = hsv &0xff;

			v = (v * 102) / 100;
			if (v > 255)
				v = 255;
			int rgb = LightGetRGB(h,s,v);
			r = (rgb>>16)&0xff;
			g = (rgb>>8)&0xff;
			b = rgb&0xff;
//		}		
		pvr_set_pal_entry(256 + i, get_color_argb1555(r,g,b,1));//pal1[i]);	
#endif
		pvr_set_pal_entry(256+i, pal2[i]);
	}
	
	pvr_set_pal_entry(0,0);
	pvr_set_pal_entry(256,0);

	dbgio_printf("PVR mem free before sprites: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/non_enemy.tex", fnpre);
	size_t vqsize = fs_load(fnbuf, &pnon_enemy);
	dbgio_printf("non_enemy loaded size is %d\n", vqsize);
	pvr_non_enemy = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + pnon_enemy, pvr_non_enemy, vqsize - 16);
	free(pnon_enemy);
	dbgio_printf("PVR mem free after non_enemy: %lu\n", pvr_mem_available());
#if 0
	sprintf(fnbuf, "%s/vq/playtrooposs.vq", fnpre);
	vqsize = fs_load(fnbuf, &pplaytrooposs);
	pvr_playtrooposs = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + pplaytrooposs, pvr_playtrooposs, vqsize - 16);
	free(pplaytrooposs);
	dbgio_printf("PVR mem free after playtrooposs: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/skulbosshead.vq", fnpre);
	vqsize = fs_load(fnbuf, &pskulbosshead);
	pvr_skulbosshead = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + pskulbosshead, pvr_skulbosshead, vqsize - 16);
	free(pskulbosshead);
	dbgio_printf("PVR mem free after skulbosshead: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/painbsp.vq", fnpre);
	vqsize = fs_load(fnbuf, &ppainbsp);
	pvr_painbsp = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + ppainbsp, pvr_painbsp, vqsize - 16);
	free(ppainbsp);
	dbgio_printf("PVR mem free after painbsp: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/fattrect1.vq", fnpre);
	vqsize = fs_load(fnbuf, &pfattrect1);
	pvr_fattrect1 = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + pfattrect1, pvr_fattrect1, vqsize - 16);
	free(pfattrect1);
	dbgio_printf("PVR mem free after fattrect1: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/fattrect2.vq", fnpre);
	vqsize = fs_load(fnbuf, &pfattrect2);
	pvr_fattrect2 = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + pfattrect2, pvr_fattrect2, vqsize - 16);
	free(pfattrect2);
	dbgio_printf("PVR mem free after fattrect2: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/fattrect3.vq", fnpre);
	vqsize = fs_load(fnbuf, &pfattrect3);
	pvr_fattrect3 = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + pfattrect3, pvr_fattrect3, vqsize - 16);
	free(pfattrect3);
	dbgio_printf("PVR mem free after fattrect3: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/sargfirstcybr.vq", fnpre);
	vqsize = fs_load(fnbuf, &psargfirstcybr);
	pvr_sargfirstcybr = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + psargfirstcybr, pvr_sargfirstcybr, vqsize - 16);
	free(psargfirstcybr);
	dbgio_printf("PVR mem free after sargfirstcybr: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/remcybr.vq", fnpre);
	vqsize = fs_load(fnbuf, &premcybr);
	pvr_remcybr = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + premcybr, pvr_remcybr, vqsize - 16);
	free(premcybr);
	dbgio_printf("PVR mem free after remcybr: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/barniteshot.vq", fnpre);
	vqsize = fs_load(fnbuf, &pbarniteshot);
	pvr_barniteshot = pvr_mem_malloc(vqsize);	
	pvr_txr_load(16 + pbarniteshot, pvr_barniteshot, vqsize - 16);
	free(pbarniteshot);
	dbgio_printf("PVR mem free after barniteshot: %lu\n", pvr_mem_available());

	sprintf(fnbuf, "%s/vq/spectre.vq", fnpre);
	vqsize = fs_load(fnbuf, &pspectre);
	pvr_spectre = pvr_mem_malloc(vqsize);
	pvr_txr_load(16 + pspectre, pvr_spectre, vqsize - 16);
	free(pspectre);
	dbgio_printf("PVR mem free after spectre: %lu\n", pvr_mem_available());
#endif
	fullwad = malloc(6828604);//5997660);//6101168);

	wadfileptr = (wadinfo_t *)Z_Alloc(sizeof(wadinfo_t), PU_STATIC, NULL);
	sprintf(fnbuf, "%s/pow2.wad", fnpre); // doom64.wad
	wad_file = fs_open(fnbuf, O_RDONLY);

	dbgio_printf("W_Init: Loading IWAD into RAM...\n");
	fs_read(wad_file, (void*)fullwad, 6828604);//5997660);//6101168);
	dbgio_printf("Done.\n");
	fs_close(wad_file);

	dbgio_printf("Done.\n");

	pvr_poly_cxt_txr(&pvr_sprite_cxt[0], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(1) | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_non_enemy, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[0].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[0].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[0].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[0], &pvr_sprite_cxt[0]);	
#if 0
	pvr_poly_cxt_txr(&pvr_sprite_cxt[1], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_playtrooposs, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[1].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[1].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[1].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[1], &pvr_sprite_cxt[1]);	

	pvr_poly_cxt_txr(&pvr_sprite_cxt[2], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_skulbosshead, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[2].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[2].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[2].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[2], &pvr_sprite_cxt[2]);	

	pvr_poly_cxt_txr(&pvr_sprite_cxt[3], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_painbsp, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[3].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[3].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[3].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[3], &pvr_sprite_cxt[3]);

	pvr_poly_cxt_txr(&pvr_sprite_cxt[4], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_fattrect1, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[4].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[4].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[4].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[4], &pvr_sprite_cxt[4]);	

	pvr_poly_cxt_txr(&pvr_sprite_cxt[5], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_fattrect2, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[5].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[5].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[5].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[5], &pvr_sprite_cxt[5]);	

	pvr_poly_cxt_txr(&pvr_sprite_cxt[6], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_fattrect3, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[6].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[6].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[6].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[6], &pvr_sprite_cxt[6]);	

	pvr_poly_cxt_txr(&pvr_sprite_cxt[7], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_sargfirstcybr, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[7].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[7].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[7].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[7], &pvr_sprite_cxt[7]);	
	
	pvr_poly_cxt_txr(&pvr_sprite_cxt[8], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_remcybr, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[8].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[8].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[8].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[8], &pvr_sprite_cxt[8]);	

	pvr_poly_cxt_txr(&pvr_sprite_cxt[9], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_barniteshot, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[9].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[9].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[9].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[9], &pvr_sprite_cxt[9]);	

	pvr_poly_cxt_txr(&pvr_sprite_cxt[10], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, 1024, 1024, pvr_spectre, PVR_FILTER_BILINEAR);
	pvr_sprite_cxt[10].gen.specular = PVR_SPECULAR_ENABLE;
	pvr_sprite_cxt[10].gen.fog_type = PVR_FOG_TABLE;
	pvr_sprite_cxt[10].gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&pvr_sprite_hdr[10], &pvr_sprite_cxt[10]);	
#endif
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
}

char retname[9];

char *W_GetNameForNum(int num) {
	memset(retname,0,9);
    int ln_len = strlen(lumpinfo[num].name);
    if(ln_len > 8) ln_len = 8;
    memcpy(retname, lumpinfo[num].name, ln_len);
	retname[0] &= 0x7f;

	return retname;//lumpinfo[num].name;
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

	I_Error ("W_GetNumForName: %s not found!",name);
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
    if ((lump < 0) || (lump >= numlumps))
		I_Error ("W_LumpLength: lump %i out of range",lump);

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
// 96 kb buffer to replace Z_Alloc in W_ReadLump
static u64 input_w_readlump[16384]; 
static byte *input = (byte*)input_w_readlump;

void W_ReadLump (int lump, void *dest, decodetype dectype) // 8002C260
{
	lumpinfo_t *l;
	int lumpsize;

	if ((lump < 0) || (lump >= numlumps))
		I_Error ("W_ReadLump: lump %i out of range",lump);

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

	if ((lump < 0) || (lump >= numlumps))
		I_Error ("W_CacheLumpNum: lump %i out of range",lump);

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
	if (lump >= mapnumlumps)
		I_Error("W_MapLumpLength: %i out of range", lump);

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
	if (lump >= mapnumlumps)
		I_Error("W_GetMapLump: lump %d out of range", lump);

    return (void *) ((byte *)mapfileptr + maplump[lump].filepos);
}
