#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>     /* malloc, free, rand */
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
/* Linear/iterative twiddling algorithm from Marcus' tatest */
#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
                     ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )

#define MIN(a, b) ( (a)<(b)? (a):(b) )

void load_twid(void *dst, void *src, uint32_t w, uint32_t h) {
    uint32_t x, y, yout, min, mask, bpp;

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

int main(int argc, char **argv) {
	char *fn = argv[1];
	int w = atoi(argv[2]);
	int h = atoi(argv[3]);
//	int xofs	= atoi(argv[4]);
//	int yofs	= atoi(argv[5]);
	FILE *file;
	char *buffer;
	char *twid;
	unsigned long fileLen;
	char fullname[256];
	sprintf(fullname, "%s.lmp", fn);
	//Open file
	file = fopen(fullname, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", fullname);
		return -1;
	}
	//Get file length
	fileLen=(w*h) +16;
	printf("filelen %lud\n", fileLen);
	//Allocate memory
	buffer=(char *)malloc(fileLen);
	twid=(char *)malloc(fileLen-16);
	if (!buffer || !twid)
	{
		fprintf(stderr, "Memory error!");
                                fclose(file);
		return -1;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

	load_twid(twid,buffer+16,w,h);

#if 1
int orig_nzc = 0;
for(int i=16;i<fileLen;i++) {
orig_nzc += buffer[i];
}

	//Do what ever with buffer

int twid_nzc = 0;
for(int i=0;i<fileLen-16;i++) {
twid_nzc += twid[i];
}

printf("orig %d twid %d\n", orig_nzc, twid_nzc);
#endif

	FILE *twidfile;
	char twidname[256];
	sprintf(twidname, "%s.twid", fn);
	twidfile = fopen(twidname, "wb+");
		if (!twidfile)
	{
		fprintf(stderr, "Unable to open file %s", twidname);
		return -1;
	}
/*
typedef struct {

        unsigned short tile_w;

        unsigned short tile_h;

        unsigned short xoffs;

        unsigned short yoffs;

        char data[0]; // actual size determined by (tile_w * tile_h)

} spriteDC_tile_t;
*/
//unsigned short tw = (unsigned short)w;
//unsigned short th = (unsigned short)h;
//unsigned short tx = (unsigned short)xofs;
//unsigned short ty = (unsigned short)yofs;

//fwrite(&tw,sizeof(unsigned short),1,twidfile);
//fwrite(&th,sizeof(unsigned short),1,twidfile);
//fwrite(&tx,sizeof(unsigned short),1,twidfile);
//fwrite(&ty,sizeof(unsigned short),1,twidfile);
fwrite(buffer,16,1,twidfile);
	fwrite(twid,fileLen-16,1,twidfile);
	fclose(twidfile);
	free(buffer);
free(twid);
return 0;
}
