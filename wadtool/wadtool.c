#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include "lumpinfo.h"
#include "newlumpslist.h"
#include "newd64pals.h"
#include "nonenemy_sheet.h"
#define ORIGINAL_DOOM64_WAD_SIZE 6101168

// original code jnmartin84

typedef struct {
    unsigned char R, G, B;
} RGBTriple;
typedef struct {
    int size;
    RGBTriple* table;
} RGBPalette;
typedef struct {
    int width, height;
    RGBTriple* pixels;
} RGBImage;
typedef struct {
    int width, height;
    unsigned char* pixels;
} PalettizedImage;
unsigned char *encode(unsigned char *input, int inputlen, int *size);
int decodedsize(unsigned char *input);
void decode(unsigned char *input, unsigned char *output);

uint8_t *expand_4to8(uint8_t *src, int width, int height);
void unscramble(uint8_t *img, int width, int height, int tileheight, int compressed);

RGBPalette sawgpal;
RGBPalette pungpal;
RGBPalette pisgpal;
RGBPalette sht1pal;
RGBPalette sht2pal;
RGBPalette chggpal;
RGBPalette rockpal;
RGBPalette plaspal;
RGBPalette bfggpal;
RGBPalette lasrpal;

void init_gunpals(void) {
sawgpal.size = 256;
pungpal.size = 256;
pisgpal.size = 256;
sht1pal.size = 256;
sht2pal.size = 256;
chggpal.size = 256;
rockpal.size = 256;
plaspal.size = 256;
bfggpal.size = 256;
lasrpal.size = 256;
sawgpal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
pungpal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
pisgpal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
sht1pal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
sht2pal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
chggpal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
rockpal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
plaspal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
bfggpal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
lasrpal.table = (RGBTriple *)malloc(256*sizeof(RGBTriple));
}

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
/* Linear/iterative twiddling algorithm from Marcus' tatest */
#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
                     ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )

#define MIN(a, b) ( (a)<(b)? (a):(b) )

void load_twid(void *dst, void *src, uint32_t w, uint32_t h) {
    uint32_t x, y, yout, min, mask;

    min = MIN(w, h);
    mask = min - 1;

    uint8_t * pixels;
    uint16_t * vtex;
    pixels = (uint8_t *) src;
    vtex = (uint16_t*)dst;

    for(y = 0; y < h; y += 2) {
        yout = y;

        for(x = 0; x < w; x++) {
            vtex[TWIDOUT((yout & mask) / 2, x & mask) +
                (x / min + yout / min)*min * min / 2] =
                pixels[y * w + x] | (pixels[(y + 1) * w + x] << 8);
        }
    }
}

typedef struct
{
	int	filepos;
	int	size;
	char	name[8];
} lumpinfo_t;

typedef struct
{
	char	identification[4];
	int	numlumps;
	int	infotableofs;
} wadinfo_t;

wadinfo_t wadfileptr;
int infotableofs;
int numlumps;
lumpinfo_t *lumpinfo;

typedef struct
{
	unsigned short  tiles;      // 0
	short           compressed; // 2
	unsigned short  cmpsize;    // 4
	short           xoffs;      // 6
	short           yoffs;      // 8
	unsigned short  width;      // 10
	unsigned short  height;     // 12
	unsigned short  tileheight; // 14
	uint8_t		data[0];	// all of the sprite data itself
} spriteN64_t;

char identifier[4] = {'I','W','A','D'};
uint8_t *doom64wad;

#define LUMPDATASZ (256*1024)
uint8_t lumpdata[LUMPDATASZ];

short SwapShort(short dat)
{
	return ((((dat << 8) | (dat >> 8 & 0xff)) << 16) >> 16);
}

RGBPalette *fromDoom64Palette(short *data, int count);
RGBImage *fromDoom64Sprite(uint8_t *data, int w, int h, RGBPalette *pal);
PalettizedImage *FloydSteinbergDither(RGBImage *image, RGBPalette *palette);
PalettizedImage *Palettize(RGBImage *image, RGBPalette *palette);
void Resize(PalettizedImage *PalImg, int nw, int nh);

PalettizedImage *allImages[966 + 355];
spriteN64_t *allSprites[966 + 355];

int main (int argc, char **argv) {

	int z64_fd = open("doom64.z64", O_RDONLY);
	init_gunpals();
	memset(allImages, 0, sizeof(PalettizedImage *) * (966+355));

	doom64wad = (uint8_t *)malloc(ORIGINAL_DOOM64_WAD_SIZE);

	lseek(z64_fd, 408848, SEEK_SET);
	read(z64_fd, doom64wad, ORIGINAL_DOOM64_WAD_SIZE);
	close(z64_fd);

	memcpy(&wadfileptr, doom64wad, sizeof(wadinfo_t));

	if (strncasecmp(wadfileptr.identification, "IWAD", 4)) {
		printf("invalid iwad id %c %c %c %c\n",
			wadfileptr.identification[0],
			wadfileptr.identification[1],
			wadfileptr.identification[2],
			wadfileptr.identification[3]
		);
		exit(-1);
	}

	numlumps = wadfileptr.numlumps;
	lumpinfo = (lumpinfo_t *)malloc(numlumps * sizeof(lumpinfo_t));
	infotableofs = wadfileptr.infotableofs;

	memcpy((void*)lumpinfo, doom64wad + infotableofs, numlumps * sizeof(lumpinfo_t));

	for (int i=1;i<966;i++) {
		char name[9];
		memset(name, 0, 9);
		memcpy(name, lumpinfo[i].name, 8);
		name[0] &= 0x7f;
		//printf("%s\n", name);
// 1 - 346 are mostly 16 color sprites I think
// 347 - 905 are 256 color sprites or palettes
// 906 - 947 are weapons I dont know if they're 16 or 256 color


// 179 - 182 no dither

// these use external palettes
// SARG two pals 347 348		2
// PLAY three pals 395 396 397		2 + 3
// TROO two pals 448 449		5 + 2
// BOSS two pals 518 519		7 + 2
// FATT one pal 566			9 + 1
// SKUL one pal 618			10 + 1
// PAIN one pal 659			11 + 1
// BSPI one pal 688			12 + 1
// POSS two pals 725 726		13 + 2
// HEAD one pal 776			15 + 1
// CYBR one pal 818			16 + 1
// RECT one pal 876			17 + 1 == 18

		//printf("lump %d - %s\n", i, name);

		//printf("%02x %02x %02x %02x\n", lumpinfo[i].name[0], lumpinfo[i].name[1], lumpinfo[i].name[2], lumpinfo[i].name[3]);

		// EXTERNAL MONSTER PALETTES
		if (name[0] == 'P' && name[1] == 'A' && name[2] == 'L') {
			continue;
		} else if ( (!(lumpinfo[i].name[0] & 0x80)) && ((i > 0) && (i < 347))) {
			//printf("\tuncompressed lump %s\n", name);
			uint8_t *tmpdata = malloc(lumpinfo[i].size);
			memcpy(tmpdata, doom64wad + lumpinfo[i].filepos, lumpinfo[i].size);

			spriteN64_t *sprite = (spriteN64_t *)tmpdata;
			short compressed = SwapShort(sprite->compressed);
			unsigned short cmpsize = (unsigned short)SwapShort(sprite->cmpsize);
			unsigned short width = (unsigned short)SwapShort(sprite->width);
			unsigned short height = (unsigned short)SwapShort(sprite->height);
			unsigned short tileheight = (unsigned short)SwapShort(sprite->tileheight);
			uint8_t *src = (uint8_t *)((uintptr_t)tmpdata + sizeof(spriteN64_t));

			void *paldata;
			RGBPalette *curPal;
			RGBImage *curImg;
			//printf("\t\t\t16 color\n");
			if (compressed == -1) {
				width = (width + 7) & ~7;
				unscramble(src, width, height, tileheight, compressed);
				paldata = (void*)sprite + sizeof(spriteN64_t) + (width*height);
				curPal = fromDoom64Palette((short *)paldata, 256);
				curImg = fromDoom64Sprite(src, width, height, curPal);
			} else {
				width = (width + 15) & ~15;
				uint8_t *expandedimg = expand_4to8(src, width, height);
				unscramble(expandedimg, width, height, tileheight, compressed);
				paldata = (void*)sprite + sizeof(spriteN64_t) + cmpsize;
				curPal = fromDoom64Palette((short *)paldata, 16);
				curImg = fromDoom64Sprite(expandedimg, width, height, curPal);
				free(expandedimg);
			}

			RGBPalette *comPal = (RGBPalette *)malloc(sizeof(RGBPalette));
			comPal->size = 256;
			comPal->table = (RGBTriple *)malloc(256 * sizeof(RGBTriple));
			for (int ci=0;ci<256;ci++) {
				comPal->table[ci].R = D64NONENEMY[ci][0];
				comPal->table[ci].G = D64NONENEMY[ci][1];
				comPal->table[ci].B = D64NONENEMY[ci][2];
			}

			PalettizedImage *palImg;
			// 179 - 182 no dither
			if (i >= 179 && i <= 182) {
				palImg = Palettize(curImg, comPal);
			} else {
				palImg = FloydSteinbergDither(curImg, comPal);
			}
			//printf("\t\t\t\t\t%08x\n", palImg);
			allImages[i] = palImg;
//			free(palImg->pixels); free(palImg);
			free(curImg->pixels); free(curImg);
			free(curPal->table); free(curPal);
			free(comPal->table); free(comPal);

			free(tmpdata);
		}
		
		else if (lumpinfo[i].name[0] & 0x80) {
			//printf("\tcompressed lump\n");
			if (i > 0 && i < 966) {
				//printf("\t\tsprite\n");
				uint8_t *tmpdata = malloc(lumpinfo[i+1].filepos - lumpinfo[i].filepos);
				memcpy(tmpdata, doom64wad + lumpinfo[i].filepos, lumpinfo[i+1].filepos - lumpinfo[i].filepos);
				memset(lumpdata,0,LUMPDATASZ);
				decode(tmpdata, lumpdata);
				//printf("lump dec size %d\n", decsize);

				spriteN64_t *sprite = (spriteN64_t *)lumpdata;
				unsigned short tiles = (unsigned short)SwapShort(sprite->tiles) << 1;
				short compressed = SwapShort(sprite->compressed);
				unsigned short cmpsize = (unsigned short)SwapShort(sprite->cmpsize);
				short xoffs = SwapShort(sprite->xoffs);
				short yoffs = SwapShort(sprite->yoffs);
				unsigned short width = (unsigned short)SwapShort(sprite->width);
				unsigned short height = (unsigned short)SwapShort(sprite->height);
				unsigned short tileheight = (unsigned short)SwapShort(sprite->tileheight);
				uint8_t *src = (uint8_t *)((uintptr_t)lumpdata + sizeof(spriteN64_t));

				void *paldata;

				if(compressed < 0) {
					//printf("\t\t\t256 color\n");
					width = (width + 7) & ~7;
					unscramble(src, width, height, tileheight, compressed);

					if ((cmpsize & 1) && (i > 348) && (i < 924)) {
						//printf("\t\t\t\tmonster\n");
						char first4[4];
						memcpy(first4, lumpinfo[i].name, 4);
						first4[0] &= 0x7f;

						RGBPalette *curPal = (RGBPalette *)malloc(sizeof(RGBPalette));
						curPal->size = 256;
						curPal->table = (RGBTriple *)malloc(256 * sizeof(RGBTriple));

						RGBPalette *altPal = (RGBPalette *)malloc(sizeof(RGBPalette));
						altPal->size = 256;
						altPal->table = (RGBTriple *)malloc(256 * sizeof(RGBTriple));

						RGBPalette *altPal2 = (RGBPalette *)malloc(sizeof(RGBPalette));
						altPal2->size = 256;
						altPal2->table = (RGBTriple *)malloc(256 * sizeof(RGBTriple));

						for (int ci=0;ci<256;ci++) {
							if (!memcmp(first4, "SARG", 4)) {
								curPal->table[ci].R = PALSARG0[ci][0];
								curPal->table[ci].G = PALSARG0[ci][1];
								curPal->table[ci].B = PALSARG0[ci][2];
								altPal->table[ci].R = PALSARG1[ci][0];
								altPal->table[ci].G = PALSARG1[ci][1];
								altPal->table[ci].B = PALSARG1[ci][2];
							} else if (!memcmp(first4, "PLAY", 4)) {
								curPal->table[ci].R = PALPLAY0[ci][0];
								curPal->table[ci].G = PALPLAY0[ci][1];
								curPal->table[ci].B = PALPLAY0[ci][2];
								altPal->table[ci].R = PALPLAY1[ci][0];
								altPal->table[ci].G = PALPLAY1[ci][1];
								altPal->table[ci].B = PALPLAY1[ci][2];
								altPal2->table[ci].R = PALPLAY2[ci][0];
								altPal2->table[ci].G = PALPLAY2[ci][1];
								altPal2->table[ci].B = PALPLAY2[ci][2];
							} else if (!memcmp(first4, "TROO", 4)) {
								curPal->table[ci].R = PALTROO0[ci][0];
								curPal->table[ci].G = PALTROO0[ci][1];
								curPal->table[ci].B = PALTROO0[ci][2];
								altPal->table[ci].R = PALTROO1[ci][0];
								altPal->table[ci].G = PALTROO1[ci][1];
								altPal->table[ci].B = PALTROO1[ci][2];
							} else if (!memcmp(first4, "BOSS", 4)) {
								curPal->table[ci].R = PALBOSS0[ci][0];
								curPal->table[ci].G = PALBOSS0[ci][1];
								curPal->table[ci].B = PALBOSS0[ci][2];
								altPal->table[ci].R = PALBOSS1[ci][0];
								altPal->table[ci].G = PALBOSS1[ci][1];
								altPal->table[ci].B = PALBOSS1[ci][2];
							} else if (!memcmp(first4, "FATT", 4)) {
								curPal->table[ci].R = PALFATT0[ci][0];
								curPal->table[ci].G = PALFATT0[ci][1];
								curPal->table[ci].B = PALFATT0[ci][2];
							} else if (!memcmp(first4, "SKUL", 4)) {
								curPal->table[ci].R = PALSKUL0[ci][0];
								curPal->table[ci].G = PALSKUL0[ci][1];
								curPal->table[ci].B = PALSKUL0[ci][2];
							} else if (!memcmp(first4, "PAIN", 4)) {
								curPal->table[ci].R = PALPAIN0[ci][0];
								curPal->table[ci].G = PALPAIN0[ci][1];
								curPal->table[ci].B = PALPAIN0[ci][2];
							} else if (!memcmp(first4, "BSPI", 4)) {
								curPal->table[ci].R = PALBSPI0[ci][0];
								curPal->table[ci].G = PALBSPI0[ci][1];
								curPal->table[ci].B = PALBSPI0[ci][2];
							} else if (!memcmp(first4, "POSS", 4)) {
								curPal->table[ci].R = PALPOSS0[ci][0];
								curPal->table[ci].G = PALPOSS0[ci][1];
								curPal->table[ci].B = PALPOSS0[ci][2];
								altPal->table[ci].R = PALPOSS1[ci][0];
								altPal->table[ci].G = PALPOSS1[ci][1];
								altPal->table[ci].B = PALPOSS1[ci][2];
							} else if (!memcmp(first4, "HEAD", 4)) {
								curPal->table[ci].R = PALHEAD0[ci][0];
								curPal->table[ci].G = PALHEAD0[ci][1];
								curPal->table[ci].B = PALHEAD0[ci][2];
							} else if (!memcmp(first4, "CYBR", 4)) {
								curPal->table[ci].R = PALCYBR0[ci][0];
								curPal->table[ci].G = PALCYBR0[ci][1];
								curPal->table[ci].B = PALCYBR0[ci][2];
							} else if (!memcmp(first4, "RECT", 4)) {
								curPal->table[ci].R = PALRECT0[ci][0];
								curPal->table[ci].G = PALRECT0[ci][1];
								curPal->table[ci].B = PALRECT0[ci][2];
							}
						}

						RGBPalette *comPal = (RGBPalette *)malloc(sizeof(RGBPalette));
						comPal->size = 256;
						comPal->table = (RGBTriple *)malloc(256 * sizeof(RGBTriple));

						for (int ci=0;ci<256;ci++) {
							comPal->table[ci].R = D64MONSTER[ci][0];
							comPal->table[ci].G = D64MONSTER[ci][1];
							comPal->table[ci].B = D64MONSTER[ci][2];
						}

						RGBImage *curImg = fromDoom64Sprite(src, width, height, curPal);
						RGBImage *altImg = fromDoom64Sprite(src, width, height, altPal);
						RGBImage *altImg2 = fromDoom64Sprite(src, width, height, altPal2);

						PalettizedImage *palImg = //Palettize(curImg, comPal);//
						FloydSteinbergDither(curImg, comPal);

						PalettizedImage *altPalImg = //Palettize(curImg, comPal);//
						FloydSteinbergDither(altImg, comPal);
						
						PalettizedImage *altPalImg2 = //Palettize(curImg, comPal);//
						FloydSteinbergDither(altImg2, comPal);						

						allImages[i] = palImg;
						int wp2 = np2(width);
						if(wp2 < 8) wp2 = 8;
						int hp2 = np2(height);
						if(hp2 < 8) hp2 = 8;
						Resize(palImg, wp2, hp2);
						Resize(altPalImg, wp2, hp2);
						Resize(altPalImg2, wp2, hp2);
						//printf("\t\t\t\t\t%08x\n", palImg);

						allSprites[i] = (spriteN64_t *)malloc(sizeof(spriteN64_t) + (wp2*hp2));
						allSprites[i]->tiles = SwapShort(tiles);
						allSprites[i]->compressed = SwapShort(compressed);
						allSprites[i]->cmpsize = SwapShort(cmpsize);
						allSprites[i]->xoffs = SwapShort(xoffs);
						allSprites[i]->yoffs = SwapShort(yoffs);
						allSprites[i]->width = SwapShort(wp2);
						allSprites[i]->height = SwapShort(hp2);
						allSprites[i]->tileheight = SwapShort(tileheight);
						load_twid(allSprites[i]->data, palImg->pixels, wp2, hp2);

						char fullname[9];
						memset(fullname, 0, 9);
						memcpy(fullname, lumpinfo[i].name, 8);
						fullname[0] &= 0x7f;
						int altlumpnum = -1;
						int altlumpnum2 = -1;

						if (!memcmp(fullname, "SARG", 4)) {
							fullname[1] = 'P';
							fullname[2] = 'E';
							fullname[3] = 'C';
							altlumpnum = W_GetNumForName(fullname);
						} else if (!memcmp(fullname, "TROO", 4)) {
							fullname[0] = 'N';
							fullname[1] = 'I';
							fullname[2] = 'T';
							fullname[3] = 'E';
							altlumpnum = W_GetNumForName(fullname);
						} else if (!memcmp(fullname, "POSS", 4)) {
							fullname[0] = 'Z';
							fullname[1] = 'O';
							fullname[2] = 'M';
							fullname[3] = 'B';
							altlumpnum = W_GetNumForName(fullname);
						} else if (!memcmp(fullname, "BOSS", 4)) {
							fullname[1] = 'A';
							fullname[2] = 'R';
							fullname[3] = 'O';
							altlumpnum = W_GetNumForName(fullname);
						}  else if (!memcmp(fullname, "PLAY", 4)) {
							fullname[2] = 'Y';
							fullname[3] = '1';
							altlumpnum = W_GetNumForName(fullname);
							fullname[2] = 'Y';
							fullname[3] = '2';
							altlumpnum2 = W_GetNumForName(fullname);
						}
						if (altlumpnum != -1) {
						allSprites[altlumpnum] = (spriteN64_t *)malloc(sizeof(spriteN64_t) + (wp2*hp2));
						allSprites[altlumpnum]->tiles = SwapShort(tiles);
						allSprites[altlumpnum]->compressed = SwapShort(compressed);
						allSprites[altlumpnum]->cmpsize = SwapShort(cmpsize);
						allSprites[altlumpnum]->xoffs = SwapShort(xoffs);
						allSprites[altlumpnum]->yoffs = SwapShort(yoffs);
						allSprites[altlumpnum]->width = SwapShort(wp2);
						allSprites[altlumpnum]->height = SwapShort(hp2);
						allSprites[altlumpnum]->tileheight = SwapShort(tileheight);
						load_twid(allSprites[altlumpnum]->data, altPalImg->pixels, wp2, hp2);						
						}
						if (altlumpnum2 != -1) {
						allSprites[altlumpnum2] = (spriteN64_t *)malloc(sizeof(spriteN64_t) + (wp2*hp2));
						allSprites[altlumpnum2]->tiles = SwapShort(tiles);
						allSprites[altlumpnum2]->compressed = SwapShort(compressed);
						allSprites[altlumpnum2]->cmpsize = SwapShort(cmpsize);
						allSprites[altlumpnum2]->xoffs = SwapShort(xoffs);
						allSprites[altlumpnum2]->yoffs = SwapShort(yoffs);
						allSprites[altlumpnum2]->width = SwapShort(wp2);
						allSprites[altlumpnum2]->height = SwapShort(hp2);
						allSprites[altlumpnum2]->tileheight = SwapShort(tileheight);
						load_twid(allSprites[altlumpnum2]->data, altPalImg2->pixels, wp2, hp2);	
						}
						
						free(curImg->pixels); free(curImg);
						free(curPal->table); free(curPal);
						free(comPal->table); free(comPal);
					} else {
						//printf("\t\t\t\tnonenemy\n");
//non-enemy 256 color sprite %d\n", i);
						RGBPalette *curPal;
						if (!(memcmp(name,"SAWGA0",6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
									sawgpal.table[n].R = curPal->table[n].R;
									sawgpal.table[n].G = curPal->table[n].G;
									sawgpal.table[n].B = curPal->table[n].B;
							}
						} else if (!(memcmp(name, "PUNGA0", 6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
									pungpal.table[n].R = curPal->table[n].R;
									pungpal.table[n].G = curPal->table[n].G;
									pungpal.table[n].B = curPal->table[n].B;
							}
						} else if (!(memcmp(name, "PISGA0", 6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
									pisgpal.table[n].R = curPal->table[n].R;
									pisgpal.table[n].G = curPal->table[n].G;
									pisgpal.table[n].B = curPal->table[n].B;
							}
						} else if (!(memcmp(name, "SHT1A0", 6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
									sht1pal.table[n].R = curPal->table[n].R;
									sht1pal.table[n].G = curPal->table[n].G;
									sht1pal.table[n].B = curPal->table[n].B;
							}
						} else if (!(memcmp(name, "SHT2A0", 6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
								sht2pal.table[n].R = curPal->table[n].R;
								sht2pal.table[n].G = curPal->table[n].G;
								sht2pal.table[n].B = curPal->table[n].B;
							}
						} else if (!(memcmp(name, "CHGGA0", 6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
								chggpal.table[n].R = curPal->table[n].R;
								chggpal.table[n].G = curPal->table[n].G;
								chggpal.table[n].B = curPal->table[n].B;
							}
						} else if (!(memcmp(name, "ROCKA0", 6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
								rockpal.table[n].R = curPal->table[n].R;
								rockpal.table[n].G = curPal->table[n].G;
								rockpal.table[n].B = curPal->table[n].B;
							}
						} else if (!(memcmp(name, "PLASA0", 6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
								plaspal.table[n].R = curPal->table[n].R;
								plaspal.table[n].G = curPal->table[n].G;
								plaspal.table[n].B = curPal->table[n].B;
							}
						} else if (!(memcmp(name, "BFGGA0", 6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
								bfggpal.table[n].R = curPal->table[n].R;
								bfggpal.table[n].G = curPal->table[n].G;
								bfggpal.table[n].B = curPal->table[n].B;
							}
						} else if (!(memcmp(name, "LASRA0", 6))) {
							paldata = (void*)sprite + sizeof(spriteN64_t) + (cmpsize);
 							curPal = fromDoom64Palette((short *)paldata, 256);
							for(int n=0;n<256;n++) {
								lasrpal.table[n].R = curPal->table[n].R;
								lasrpal.table[n].G = curPal->table[n].G;
								lasrpal.table[n].B = curPal->table[n].B;
							}
						}

						if (cmpsize & 1) {
							if (!(memcmp(name,"SAWG",4))) {
								curPal = &sawgpal;
							} else if (!(memcmp(name, "PUNG", 4))) {
								curPal = &pungpal;
							} else if (!(memcmp(name, "PISG", 4))) {
								curPal = &pisgpal;
							} else if (!(memcmp(name, "SHT1", 4))) {
								curPal = &sht1pal;
							} else if (!(memcmp(name, "SHT2", 4))) {
								curPal = &sht2pal;
							} else if (!(memcmp(name, "CHGG", 4))) {
								curPal = &chggpal;
							} else if (!(memcmp(name, "ROCK", 4))) {
								curPal = &rockpal;
							} else if (!(memcmp(name, "PLAS", 4))) {
								curPal = &plaspal;
							} else if (!(memcmp(name, "BFGG", 4))) {
								curPal = &bfggpal;
							} else if (!(memcmp(name, "LASR", 4))) {
								curPal = &lasrpal;
							} /*else {
								curPal = fromDoom64Palette((short*)((void*)sprite + sizeof(spriteN64_t) + (width*height)), 256);
							}*/
						}

//						paldata = (void*)sprite + sizeof(spriteN64_t) + (width * height);
//						RGBPalette *curPal = fromDoom64Palette((short *)paldata, 256);
						RGBImage *curImg = fromDoom64Sprite(src, width, height, curPal);
						RGBPalette *comPal = (RGBPalette *)malloc(sizeof(RGBPalette));
						comPal->size = 256;
						comPal->table = (RGBTriple *)malloc(256 * sizeof(RGBTriple));
						for (int ci=0;ci<256;ci++) {
							comPal->table[ci].R = D64NONENEMY[ci][0];
							comPal->table[ci].G = D64NONENEMY[ci][1];
							comPal->table[ci].B = D64NONENEMY[ci][2];
						}
						PalettizedImage *palImg = //Palettize(curImg, comPal); //
						FloydSteinbergDither(curImg, comPal);
						allImages[i] = palImg;
						//printf("\t\t\t\t\t%08x\n", palImg);
						//free(palImg->pixels); free(palImg);
						free(curImg->pixels); free(curImg);
//						free(curPal->table); free(curPal);
						free(comPal->table); free(comPal);
        				}
				} else {
					//printf("\t\t\t16 color\n");
					width = (width + 15) & ~15;
					uint8_t *expandedimg = expand_4to8(src, width, height);
					unscramble(expandedimg, width, height, tileheight, compressed);
					paldata = (void*)sprite + sizeof(spriteN64_t) + cmpsize;
					RGBPalette *curPal = fromDoom64Palette((short *)paldata, 16);
					RGBImage *curImg = fromDoom64Sprite(expandedimg, width, height, curPal);

					RGBPalette *comPal = (RGBPalette *)malloc(sizeof(RGBPalette));
					comPal->size = 256;
					comPal->table = (RGBTriple *)malloc(256 * sizeof(RGBTriple));
					for (int ci=0;ci<256;ci++) {
						comPal->table[ci].R = D64NONENEMY[ci][0];
						comPal->table[ci].G = D64NONENEMY[ci][1];
						comPal->table[ci].B = D64NONENEMY[ci][2];
					}

					PalettizedImage *palImg;
					// 179 - 182 no dither
					if (i >= 179 && i <= 182) {
						palImg = Palettize(curImg, comPal);
					} else {
						palImg = FloydSteinbergDither(curImg, comPal);
					}

					//printf("\t\t\t\t\t%08x\n", palImg);
					allImages[i] = palImg;
//					free(palImg->pixels); free(palImg);
					free(curImg->pixels); free(curImg);
					free(curPal->table); free(curPal);
					free(comPal->table); free(comPal);
					free(expandedimg);
				}
				free(tmpdata);
			}
		}
	}

	uint8_t *ne_sheet = (uint8_t *)malloc(1024*1024);

	for (int i=0;i<388;i++) {
		int lumpnum = nonenemy_sheet[i][0];
		int x = nonenemy_sheet[i][1];
		int y = nonenemy_sheet[i][2];
		int w = nonenemy_sheet[i][3];
		int h = nonenemy_sheet[i][4];

		PalettizedImage *lumpImg = allImages[lumpnum];
		if (!lumpImg) {
			printf("missing image for lump %d\n", lumpnum);
			exit(-1);
		}

		for (int k=y;k<y+h;k++) {
			for (int j=x;j<x+w;j++) {
				ne_sheet[(k*1024) + j] = lumpImg->pixels[((k-y)*w) + (j-x)];
			}
		}
	}

	uint8_t *twid_sheet = (uint8_t *)malloc(1024*1024);
	load_twid(twid_sheet, ne_sheet, 1024, 1024);
	uint8_t junk[16];
	int sheet_fd = open("non_enemy.tex", O_RDWR | O_CREAT, 0666);
	write(sheet_fd, junk, 16);
	write(sheet_fd, twid_sheet, 1024*1024);
	close(sheet_fd);
	free(ne_sheet);
	free(twid_sheet);

	/*
	 * HERE BEGINS FIXWAD CODE adapted
	 */
	int fd = open("pow2.wad", O_RDWR | O_CREAT, 0666);

	for(int i=0;i<4;i++) {
		write(fd, &identifier[i], 1);
	}
	write(fd, &numlumps, 4);
	int lastofs = 4 + 4 + 4;

	for (int i=0;i<numlumps;i++) {
		if (((i < 349) || (i > 923)) ||
			((names[i][0] == 'P') && (names[i][1] == 'A') && (names[i][2] == 'L')) ) {

			int orig_size = lumpinfo[i].size;
			//printf("%d %d -> ", i, lastofs);

			if (lumpinfo[i].name[0] & 0x80) {
				orig_size = lumpinfo[i+1].filepos - lumpinfo[i].filepos;
			}

			int padded_size = (orig_size + 3) & ~3;
			lastofs = lastofs + padded_size;
			//printf("%d\n", lastofs);
		}
		else {
			uint8_t *outbuf;
			int outlen;
			int wp2 = SwapShort(allSprites[i]->width);
			int hp2 = SwapShort(allSprites[i]->height);
			outbuf = encode((void *)allSprites[i], sizeof(spriteN64_t) + (wp2*hp2), &outlen);
			free(outbuf);
			int orig_size = outlen;
			int padded_size = (orig_size + 3) & ~3;
			//printf("* %d %d -> ", i, lastofs);
			lastofs = lastofs + padded_size;
			//printf("%d\n", lastofs);
		}
	}
//printf("computed lastofs %d (%08x)\n", lastofs, lastofs);
	write(fd, &lastofs, 4);
	lastofs = 12;

	for (int i=0;i<numlumps;i++) {
		if (((i < 349) || (i > 923)) ||
			((names[i][0] == 'P') && (names[i][1] == 'A') && (names[i][2] == 'L'))) {

			int data_size;
			int orig_size = lumpinfo[i].size;

			data_size = orig_size;
			if (lumpinfo[i].name[0] & 0x80) {
				data_size = lumpinfo[i+1].filepos - lumpinfo[i].filepos;
			}

			memset(lumpdata, 0, LUMPDATASZ);
			memcpy(lumpdata, doom64wad + lumpinfo[i].filepos, data_size);

			data_size = (data_size+3)&~3;

			write(fd, lumpdata, data_size);

			lumpinfo[i].filepos = lastofs;
			lumpinfo[i].size = orig_size;
			lastofs = lastofs + data_size;
		} else {
			uint8_t *outbuf;
			int outlen;
			int wp2 = SwapShort(allSprites[i]->width);
			int hp2 = SwapShort(allSprites[i]->height);
			int fileLen;
			int origLen = sizeof(spriteN64_t) + (wp2*hp2);
			outbuf = encode((void *)allSprites[i], origLen, &outlen);
			fileLen = outlen;
			int orig_size = fileLen;
			int padded_size = (orig_size + 3) & ~3;

			memset(lumpdata, 0, LUMPDATASZ);
			memcpy(lumpdata, outbuf, orig_size);
			free(outbuf);
			
			write(fd, lumpdata, padded_size);
//printf("%d -> %d\n", origLen, outlen);
			lumpinfo[i].filepos = lastofs;
			lumpinfo[i].size = origLen;
			lastofs = lastofs + padded_size;
		}
	}

	for (int i=0;i<numlumps;i++) {
		write(fd, (void*)(&lumpinfo[i]), sizeof(lumpinfo_t));
	}
	close(fd);

#define NUMALTLUMPS 311
	char pwad[4] = {'P','W','A','D'};
	int alt_fd = open("alt.wad", O_RDWR | O_CREAT, 0666);
	for(int i=0;i<4;i++) {
		write(alt_fd, &pwad[i], 1);
	}
	int numaltlumps = NUMALTLUMPS;
	write(alt_fd, &numaltlumps, 4);
	lastofs = 4 + 4 + 4;
	lumpinfo_t *altlumpinfo = (lumpinfo_t *)malloc(numaltlumps * sizeof(lumpinfo_t));

	for (int i=0;i<numaltlumps;i++) {
		memset(altlumpinfo[i].name, 0, 8);
		memcpy(altlumpinfo[i].name, newlumps[i], 8);
		if (newlumps[i][0] != 'S' || (newlumps[i][0] == 'S' && newlumps[i][1] != '2')) {
			altlumpinfo[i].name[0] |= 0x80;
		}
		altlumpinfo[i].filepos = lastofs;
		if(newlumps[i][0] == 'S' && newlumps[i][1] == '2') {
			altlumpinfo[i].size = 0;
		} else {
			int altlumpnum = W_GetNumForName(newlumps[i]);

			uint8_t *outbuf;
			int outlen;
			int wp2 = SwapShort(allSprites[altlumpnum]->width);
			int hp2 = SwapShort(allSprites[altlumpnum]->height);
			int fileLen;
			int origLen = sizeof(spriteN64_t) + (wp2*hp2);
			outbuf = encode((void *)allSprites[altlumpnum], origLen, &outlen);
			free(outbuf);
			fileLen = outlen;
			int orig_size = fileLen;
			int padded_size = (orig_size + 3) & ~3;
			altlumpinfo[i].size = origLen;
			lastofs = lastofs + padded_size;
		}
	}

	write(alt_fd, &lastofs, 4);
	
	for (int i=0;i<numaltlumps;i++) {
		if (altlumpinfo[i].size) {
			int altlumpnum = W_GetNumForName(newlumps[i]);

			uint8_t *outbuf;
			int outlen;
			int wp2 = SwapShort(allSprites[altlumpnum]->width);
			int hp2 = SwapShort(allSprites[altlumpnum]->height);
			int fileLen;
			int origLen = sizeof(spriteN64_t) + (wp2*hp2);
			outbuf = encode((void *)allSprites[altlumpnum], origLen, &outlen);
			fileLen = outlen;
			int orig_size = fileLen;
			int padded_size = (orig_size + 3) & ~3;
			
			memset(lumpdata, 0, LUMPDATASZ);
			memcpy(lumpdata, outbuf, orig_size);
			free(outbuf);
			write(alt_fd, lumpdata, padded_size);
		}
	}
	
	for (int i=0;i<numaltlumps;i++) {
		write(alt_fd, (void*)(&altlumpinfo[i]), sizeof(lumpinfo_t));
	}	
	close(alt_fd);
	free(doom64wad);
	return 0;
}
