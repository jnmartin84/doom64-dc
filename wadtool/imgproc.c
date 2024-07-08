#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// adapatation and extension of
// https://literateprograms.org/floyd-steinberg_dithering__c_.html

extern short SwapShort(short val);

#include "imgtypes.h"

/* must free returnvalue->table AND returnvalue */
RGBPalette *fromDoom64Palette(uint16_t *data, int32_t count) {
	uint16_t *palsrc;
	RGBPalette *retPal;
	uint16_t val;
	uint8_t r;
	uint8_t g;
	uint8_t b;

	retPal = (RGBPalette *)malloc(sizeof(RGBPalette));
	retPal->size = count;
	retPal->table = (RGBTriple *)malloc(count * sizeof(RGBTriple));

	palsrc = data;

	for(int32_t j = 0; j < count; j++) {
		val = *palsrc++;
		val = SwapShort(val);
		b = (val & 0x003E) << 2;
		g = (val & 0x07C0) >> 3;
		r = (val & 0xF800) >> 8;

		if ((j + r + g + b) == 0) {
			retPal->table[0].R = 255;
			retPal->table[0].G = 0;
			retPal->table[0].B = 255;
		} else {
			retPal->table[j].R = r;
			retPal->table[j].G = g;
			retPal->table[j].B = b;
		}
	}

	return retPal;
}

/* must free returnvalue->pixels AND returnvalue */
RGBImage *fromDoom64Sprite(uint8_t *data, int32_t w, int32_t h, RGBPalette *pal) {
	RGBImage *retImg;
	int32_t index;
	uint8_t pixel;

	retImg = (RGBImage *)malloc(sizeof(RGBImage));
	retImg->width = w;
	retImg->height = h;
	retImg->pixels = (RGBTriple *)malloc((w * h) * sizeof(RGBTriple));

	for (int32_t j=0;j<h;j++) {
		for (int32_t i=0; i<w; i++) {
			index = (j*w) + i;
			pixel = data[index];

			retImg->pixels[index].R = pal->table[pixel].R;
			retImg->pixels[index].G = pal->table[pixel].G;
			retImg->pixels[index].B = pal->table[pixel].B;
		}
	}

	return retImg;
}

// https://stackoverflow.com/a/34187992
static uint32_t usqrt4(uint32_t val) {
	uint32_t a, b;

	if (val < 2) return val;

	a = 1255;

	b = val / a;
	a = (a + b) >> 1;

	b = val / a;
	a = (a + b) >> 1;
	b = val / a;
	a = (a + b) >> 1;
	b = val / a;
	a = (a + b) >> 1;

	return a;
}

// m_fixed.c
static uint32_t D_abs(int32_t x)
{
    int32_t _s = x >> 31;
    return (uint32_t)((x ^ _s) - _s);
}

// https://stackoverflow.com/a/9085524
uint32_t ColorDistance(RGBTriple *c1, RGBTriple *c2)
{
	uint32_t rmean = ((int32_t)c1->R + (int32_t)c2->R) / 2;
	uint32_t r = D_abs((int32_t)c1->R - (int32_t)c2->R);
	uint32_t g = D_abs((int32_t)c1->G - (int32_t)c2->G);
	uint32_t b = D_abs((int32_t)c1->B - (int32_t)c2->B);
	return usqrt4(
		(((512 + rmean) * r * r) >> 8)
		+ (4 * g * g)
		+ (((767 - rmean) * b * b) >> 8) );
}

// this does a much better job than GIMP version of the art pipeline
unsigned char FindNearestColor(RGBTriple *color, RGBPalette *palette) {
	int i, bestIndex = 0;
	uint32_t distanceSquared, minDistanceSquared;
	minDistanceSquared = (1 << 31);
	for (i=0; i<palette->size; i++) {
		distanceSquared = ColorDistance(color, &(palette->table[i]));
		if (distanceSquared < minDistanceSquared) {
			minDistanceSquared = distanceSquared;
			bestIndex = i;
		}
	}
	return bestIndex;
}

#define plus_truncate_uchar(a, b) \
    if (((int)(a)) + (b) < 0) \
        (a) = 0; \
    else if (((int)(a)) + (b) > 255) \
        (a) = 255; \
    else \
        (a) += (b);

#define compute_disperse(channel) \
error = ((int)(currentPixel->channel)) - palette->table[index].channel; \
if (x + 1 < image->width) { \
	if (!((image->pixels[(x+1) + (y+0)*image->width].R == 0xff) \
	&& (image->pixels[(x+1) + (y+0)*image->width].G == 0x00)	\
	&& (image->pixels[(x+1) + (y+0)*image->width].B == 0xff))) { \
    plus_truncate_uchar(image->pixels[(x+1) + (y+0)*image->width].channel, (error*7) >> 4); \
	}	\
} \
if (y + 1 < image->height) { \
    if (x - 1 > 0) { \
	if (!((image->pixels[(x-1) + (y+1)*image->width].R == 0xff) \
	&& (image->pixels[(x-1) + (y+1)*image->width].G == 0x00)	\
	&& (image->pixels[(x-1) + (y+1)*image->width].B == 0xff))) { \
        plus_truncate_uchar(image->pixels[(x-1) + (y+1)*image->width].channel, (error*3) >> 4); \
	}	\
    } \
	if (!((image->pixels[(x+0) + (y+1)*image->width].R == 0xff) \
	&& (image->pixels[(x+0) + (y+1)*image->width].G == 0x00)	\
	&& (image->pixels[(x+0) + (y+1)*image->width].B == 0xff))) { \
    plus_truncate_uchar(image->pixels[(x+0) + (y+1)*image->width].channel, (error*5) >> 4); \
	}	\
    if (x + 1 < image->width) { \
	if (!((image->pixels[(x+1) + (y+1)*image->width].R == 0xff) \
	&& (image->pixels[(x+1) + (y+1)*image->width].G == 0x00)	\
	&& (image->pixels[(x+1) + (y+1)*image->width].B == 0xff))) { \
        plus_truncate_uchar(image->pixels[(x+1) + (y+1)*image->width].channel, (error*1) >> 4); \
	}	\
    } \
}

/* must free returnvalue->pixels and returnvalue */
PalettizedImage *FloydSteinbergDither(RGBImage *image, RGBPalette *palette) {
	int x, y;
	PalettizedImage *retImg = (PalettizedImage *)malloc(sizeof(PalettizedImage *));
	retImg->width = image->width;
	retImg->height = image->height;
	retImg->pixels = malloc(image->width * image->height);
	memset(retImg->pixels, 0, image->width * image->height);

	for(y = 0; y < image->height; y++) {
		for(x = 0; x < image->width; x++) {
			RGBTriple *currentPixel = &(image->pixels[(y * image->width) + x]);
			unsigned char index = FindNearestColor(currentPixel, palette);
			if (index) {
				int error;
				retImg->pixels[(y * image->width) + x] = index;
				compute_disperse(R);
				compute_disperse(G);
				compute_disperse(B);
			}
		}
	}
	return retImg;
}


/* must free returnvalue->pixels and returnvalue */
PalettizedImage *Palettize(RGBImage *image, RGBPalette *palette) {
	int x, y;
	PalettizedImage *retImg = (PalettizedImage *)malloc(sizeof(PalettizedImage *));
	retImg->width = image->width;
	retImg->height = image->height;
	retImg->pixels = malloc(image->width * image->height);
	memset(retImg->pixels, 0, image->width * image->height);
	for(y = 0; y < image->height; y++) {
		for(x = 0; x < image->width; x++) {
			RGBTriple *currentPixel = &(image->pixels[(y * image->width) + x]);
			unsigned char index = FindNearestColor(currentPixel, palette);
			if (index) {
				retImg->pixels[(y * image->width) + x] = index;
			}
		}
	}
	return retImg;
}

void Resize(PalettizedImage *image, int wp2, int hp2) {
	int worig = image->width;
	int horig = image->height;

	uint8_t *newpixels = malloc(wp2 * hp2);
	memset(newpixels, 0, wp2 * hp2);


	for (int h=0;h<horig;h++) {
		for (int w=0;w<worig;w++) {
			newpixels[(h * wp2) + w] =
			image->pixels[(h * worig) + w];
		}
	}


	uint8_t *oldpixels = image->pixels;
	image->pixels = newpixels;
	free(oldpixels);
}

// from Doom64EX wadgen
// https://github.com/svkaiser/Doom64EX/blob/a5a8ccb87db062d4aacfbda09ac1404d49b5e973/src/engine/wadgen/sprite.cc#L179
/* must free return value */
uint8_t *expand_4to8(uint8_t *src, int width, int height) {
	int tmp, i;
	uint8_t *buffer = malloc(width*height);

	// Decompress the sprite by taking one byte and turning it into two values
	for (tmp = 0, i = 0; i < (width * height) / 2; i++) {
		buffer[tmp++] = (src[i] >> 4);
		buffer[tmp++] = (src[i] & 0xf);
	}

	return buffer;
}


// from Doom64EX wadgen
// https://github.com/svkaiser/Doom64EX/blob/a5a8ccb87db062d4aacfbda09ac1404d49b5e973/src/engine/wadgen/sprite.cc#L218
/* modifies img in place; no returnvalue, nothing to free */
void unscramble(uint8_t *img, int width, int height, int tileheight, int compressed) {
	uint8_t *buffer;
	int tmp,h,w,id,inv,pos;
	tmp = 0;
	h=0;w=0;id=0;inv=0;pos=0;
	buffer = malloc(width*height);
	for (h = 0; h < height; h++, id++) {
		// Reset the check for flipped rows if its beginning on a new tile
		if (id == tileheight) {
			id = 0;
			inv = 0;
		}
		// This will handle the row flipping issue found on most multi tiled sprites..
		if (inv) {
			if (compressed == -1) {
				for (w = 0; w < width; w += 8) {
					for (pos = 4; pos < 8; pos++) {
						buffer[tmp++] =
						img[(h*width) + w + pos];
					}
					for (pos = 0; pos < 4; pos++) {
						buffer[tmp++] =
						img[(h*width) + w + pos];
					}
				}
			} else {
				for (w = 0; w < width; w += 16) {
					for (pos = 8; pos < 16; pos++) {
						buffer[tmp++] =
						img[(h * width) + w + pos];
					}
					for (pos = 0; pos < 8; pos++) {
						buffer[tmp++] =
						img[(h * width) + w + pos];
					}
				}
			}
		} else {		// Copy the sprite rows normally
			for (w = 0; w < width; w++) {
				buffer[tmp++] = img[(h * width) + w];
			}
		}
		inv ^= 1;
	}

	memcpy(img, buffer, width*height);
	free(buffer);
}
