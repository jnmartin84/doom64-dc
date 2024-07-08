#ifndef __IMGTYPES_H
#define __IMGTYPES_H

#include <stdint.h>

typedef struct {
	uint8_t R;
	uint8_t G;
	uint8_t B;
} RGBTriple;

typedef struct {
	int32_t size;
	RGBTriple *table;
} RGBPalette;

typedef struct {
	int32_t width;
	int32_t height;
	RGBTriple *pixels;
} RGBImage;

typedef struct {
	int32_t width;
	int32_t height;
	uint8_t *pixels;
} PalettizedImage;

#endif // __IMGTYPES_H
