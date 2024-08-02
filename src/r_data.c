
/* R_data.c */

#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"
#include "sheets.h"

int			firsttex;				// 800A632C
int			lasttex;				// 800A6330
int			numtextures;			// 800A6334
int			firstswx;				// 800A6338
int    	    *textures;				// 800A633C

int			firstsprite;			// 800A6320
int			lastsprite;				// 800A6324
int			numsprites;				// 800A6328

int	        skytexture;             // 800A5f14

void R_InitTextures(void);
void R_InitSprites(void);
/*============================================================================ */

#define PI_VAL 3.141592653589793
extern uint32_t next_pow2(uint32_t v);
/*
================
=
= R_InitData
=
= Locates all the lumps that will be used by all views
= Must be called after W_Init
=================
*/
#include <math.h>
#include <dc/pvr.h>
void R_InitStatus(void);
void R_InitFont(void);
void R_InitSymbols(void);
void R_InitData (void) // 80023180
{
	// with single precision float
	// this table is not accurate enough for demos to sync
	// so I generated it offline on PC and put it in tables.c
#if 0
	int i;
	int val = 0;

	for(i = 0; i < (5*FINEANGLES/4); i++)
	{
		finesine[i] = (fixed_t) (sinf((((float) val * (float) PI_VAL) / 8192.0f)) * 65536.0f);
		val += 2;
	}
#endif
	R_InitStatus();
	R_InitFont();
	R_InitSymbols();
	R_InitTextures();
	R_InitSprites();
}

/*
==================
=
= R_InitTextures
=
= Initializes the texture list with the textures from the world map
=
==================
*/

extern short SwapShort(short dat);

pvr_ptr_t **tex_txr_ptr;
pvr_poly_cxt_t **tcxt;
uint16_t tmptex[64*64];
uint16_t tmp_pal[16];
uint16_t tmp_8bpp_pal[256];

uint8_t *num_pal;

pvr_ptr_t pvrstatus;
extern pvr_poly_cxt_t statuscxt;
extern pvr_poly_hdr_t statushdr;

void R_InitStatus(void) {
	uint16_t *status16;
	status16 = malloc(128*16*sizeof(uint16_t));
	pvrstatus = pvr_mem_malloc(128*16*2);
	// 1 tile, not compressed
	void *data = (byte *)W_CacheLumpName("STATUS",PU_CACHE,dec_jag);
	int width = (SwapShort(((spriteN64_t*)data)->width)+7)&~7;
	int height = SwapShort(((spriteN64_t*)data)->height);
	byte *src = data + sizeof(spriteN64_t);
	byte *offset = src + SwapShort(((spriteN64_t*)data)->cmpsize);
	// palette 
	tmp_8bpp_pal[0] = 0;
	short *p = (short *)offset;
	p++;
	for (int j=1;j<256;j++) {
		short val = *p;
		p++;
		val = SwapShort(val);
		// Unpack and expand to 8bpp, then flip from BGR to RGB.
		u8 b = (val & 0x003E) << 2;
		u8 g = (val & 0x07C0) >> 3;
		u8 r = (val & 0xF800) >> 8;
		u8 a = 0xff;//(val & 1);
#if 0
		if (r && g && b) {
			int hsv = LightGetHSV(r,g,b);
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
		}
#endif
		tmp_8bpp_pal[j] = get_color_argb1555(r,g,b,a);
	}

	int i = 0;
	int x1;
	int x2;
	
	for (int h=1;h<16;h+=2) {
		for (i=0;i<width/4;i+=2) {
			int *tmpSrc = (int *)(src + (h*80));
			x1 = *(int *)(tmpSrc + i);
			x2 = *(int *)(tmpSrc + i + 1);

			*(int *)(tmpSrc + i) = x2;
			*(int *)(tmpSrc + i + 1) = x1;
		}	
	}

	for (int h=0;h<height;h++) {
		for (int w=0;w<width;w++) {
			status16[w + (h*128)] = tmp_8bpp_pal[src[w + (h*width)]];
		}
	}

	pvr_txr_load_ex(status16, pvrstatus, 128, 16, PVR_TXRLOAD_16BPP);	
	pvr_poly_cxt_txr(&statuscxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB1555, 128, 16, pvrstatus, PVR_FILTER_NONE);
	pvr_poly_compile(&statushdr,&statuscxt);
	
	free(status16);
}

extern pvr_ptr_t pvrfont;
extern pvr_poly_cxt_t fontcxt;
extern pvr_poly_hdr_t fonthdr;

void R_InitFont(void) {
	uint8_t *fontcopy;
	uint16_t *font16;
	int fontlump = W_GetNumForName("SFONT");
	pvrfont = pvr_mem_malloc(256*16*2);
	void *data = W_CacheLumpNum(fontlump, PU_CACHE, dec_jag);
	int width = SwapShort(((spriteN64_t*)data)->width);
	int height = SwapShort(((spriteN64_t*)data)->height);
	byte *src = data + sizeof(spriteN64_t);
	byte *offset = src + 0x800;//SwapShort(((spriteN64_t*)data)->cmpsize);

	font16 = (uint16_t *)malloc(256*16*sizeof(uint16_t));
	fontcopy = (uint8_t *)malloc(256*16/2);
	// palette 
	short *p = (short *)offset;
	tmp_8bpp_pal[0] = 0;
	p++;
	for (int j=1;j<16;j++) {
		short val = *p;
		p++;
		val = SwapShort(val);
		// Unpack and expand to 8bpp, then flip from BGR to RGB.
		u8 b = (val & 0x003E) << 2;
		u8 g = (val & 0x07C0) >> 3;
		u8 r = (val & 0xF800) >> 8;
		u8 a = 0xff;
#if 0
		if (r && g && b) {
			int hsv = LightGetHSV(r,g,b);
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
		}
#endif
		tmp_8bpp_pal[j] = get_color_argb1555(r,g,b,a);
	}
	tmp_8bpp_pal[0] = 0;
	
	int size = (width * height) / 2;
	memset(fontcopy, 0, 256 * 8);
	memcpy(fontcopy, src, size);
	uint8_t *copy = fontcopy;

	int mask = 32;//256 / 8;
	// Flip nibbles per byte
	for (int k = 0; k < size; k++) {
		byte tmp = copy[k];
		copy[k] = (tmp >> 4);
		copy[k] |= ((tmp & 0xf) << 4);
	}
	int *tmpSrc = (int *)(copy);
	// Flip each sets of dwords based on texture width
	for (int k = 0; k < size / 4; k += 2) {
		int x1;
		int x2;
		if (k & mask) {
			x1 = *(int *)(tmpSrc + k);
			x2 = *(int *)(tmpSrc + k + 1);
			*(int *)(tmpSrc + k) = x2;
			*(int *)(tmpSrc + k + 1) = x1;
		}
	}		
	
	uint8_t *srcp = (uint8_t *)fontcopy;

	for (int j=0; j < (width*height);j+=2) {
		uint8_t sps = srcp[j>>1];
		font16[j] = tmp_8bpp_pal[sps & 0xf];
		font16[j+1] = tmp_8bpp_pal[(sps >> 4) & 0xf];
	}	

	pvr_txr_load_ex(font16, pvrfont, 256, 16, PVR_TXRLOAD_16BPP);	
	pvr_poly_cxt_txr(&fontcxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB1555, 256, 16, pvrfont, PVR_FILTER_NONE);
	pvr_poly_compile(&fonthdr,&fontcxt);
	
	free(font16);
	free(fontcopy);
}

uint16_t *symbols16;
extern pvr_ptr_t pvr_symbols;
int symbols16size = 0;
int symbols16_w;
int symbols16_h;
int rawsymbol_w;
int rawsymbol_h;

extern pvr_poly_hdr_t pvr_sym_hdr;
extern pvr_poly_cxt_t pvr_sym_cxt;
void R_InitSymbols(void)
{
	int symlump = W_GetNumForName("SYMBOLS");
	void *data = W_CacheLumpNum(symlump, PU_CACHE, dec_jag);
	byte *src = data + sizeof(gfxN64_t);

	int width = SwapShort(((gfxN64_t*)data)->width);
	int height = SwapShort(((gfxN64_t*)data)->height);

	symbols16_w = next_pow2(width);
	symbols16_h = next_pow2(height);
	symbols16size = (symbols16_w * symbols16_h * 2);

	rawsymbol_w = width;
	rawsymbol_h = height;

	pvr_symbols = pvr_mem_malloc(symbols16_w * symbols16_h * 2);

	symbols16 = (uint16_t*)malloc(symbols16size);

	// Load Palette Data
	int offset = SwapShort(((gfxN64_t*)data)->width) * SwapShort(((gfxN64_t*)data)->height);
	offset = (offset + 7) & ~7;
	// palette 
	short *p = data + offset + sizeof(gfxN64_t);
	for (int j=0;j<256;j++) {
		short val = *p;
		p++;
		val = SwapShort(val);
		// Unpack and expand to 8bpp, then flip from BGR to RGB.
		u8 b = (val & 0x003E) << 2;
		u8 g = (val & 0x07C0) >> 3;
		u8 r = (val & 0xF800) >> 8;
		u8 a = (val & 1);
#if 0
		if (r && g && b) {
			int hsv = LightGetHSV(r,g,b);
			int h = (hsv >> 16)&0xff;
			int s = (hsv >> 8)&0xff;
			int v = hsv &0xff;

			v = (v * 102) / 100;
			if(v > 255)
				v = 255;
			int rgb = LightGetRGB(h,s,v);
			r = (rgb>>16)&0xff;
			g = (rgb>>8)&0xff;
			b = rgb&0xff;				
		}
#endif    
		tmp_8bpp_pal[j] = get_color_argb1555(r,g,b,a);
	}
	tmp_8bpp_pal[0] = 0;

	for (int h=0;h<height;h++) {
		for( int w=0;w<width;w++) {
			symbols16[w + (h*symbols16_w)] = tmp_8bpp_pal[src[w + (h*width)]];
		}
	}

	pvr_txr_load_ex(symbols16, pvr_symbols, symbols16_w, symbols16_h, PVR_TXRLOAD_16BPP);
	free(symbols16);
	pvr_poly_cxt_txr(&pvr_sym_cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB1555, symbols16_w, symbols16_h, pvr_symbols, PVR_FILTER_NONE);
	pvr_poly_compile(&pvr_sym_hdr, &pvr_sym_cxt);
}

#define _PAD8(x)	x += (8 - ((uint) x & 7)) & 7
void R_InitTextures(void)
{
	int swx, i;

	firsttex = W_GetNumForName("T_START") + 1;
	lasttex = W_GetNumForName("T_END") - 1;
	numtextures = (lasttex - firsttex) + 1;
	tex_txr_ptr = (pvr_ptr_t **)malloc(numtextures * sizeof(pvr_ptr_t*));
	tcxt = (pvr_poly_cxt_t **)malloc(numtextures * sizeof(pvr_poly_cxt_t*));
	num_pal = (uint8_t*)malloc(numtextures);
	memset(tex_txr_ptr, 0, sizeof(pvr_ptr_t *) * numtextures);
	memset(tcxt, 0, sizeof(pvr_poly_cxt_t *) * numtextures);
	memset(num_pal, 0, numtextures);

	textures = Z_Malloc(numtextures * sizeof(int), PU_STATIC, NULL);

	for (i = 0; i < numtextures; i++)
	{
		int texture = (i + firsttex) << 4;
		textures[i] = texture;
	}

	swx = W_CheckNumForName("SWX", 0x7fffff00, 0);
	firstswx = (swx - firsttex);
}

/*
================
=
= R_InitSprites
=
=================
*/

void R_InitSprites(void) // 80023378
{
	firstsprite = W_GetNumForName("S_START") + 1;
	lastsprite = W_GetNumForName("S_END") - 1;
	numsprites = (lastsprite - firstsprite) + 1;
	
	setup_sprite_headers();
}
