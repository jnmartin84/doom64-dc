#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>     /* malloc, free, rand */
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

uint32_t np2(uint32_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}


short SwapShort(short dat)
{
	return ((((dat << 8) | (dat >> 8 & 0xff)) << 16) >> 16);
}

int main(int argc, char **argv) {
	char *fn = argv[1];
	unsigned short w = SwapShort((unsigned short)atoi(argv[2]));
	unsigned short h = SwapShort((unsigned short)atoi(argv[3]));
	short xofs = SwapShort((short)atoi(argv[4]));
	short yofs = SwapShort((short)atoi(argv[5]));
	unsigned short cmpsize = SwapShort((unsigned short)atoi(argv[6]));

	FILE *file;
	char *buffer;

	unsigned long fileLen;

	//Open file
	char rawname[256];
	sprintf(rawname, "%s.raw", fn);
	file = fopen(rawname, "rb");
	if (!file) {
		fprintf(stderr, "Unable to open file %s", fn);
		return -1;
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer=(char *)malloc(fileLen);
	if (!buffer) {
		fprintf(stderr, "Memory error!");
                                fclose(file);
		return -1;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

/*
typedef struct

{

        unsigned short  tiles;      // 0

        short           compressed; // 2

        unsigned short  cmpsize;    // 4

        short           xoffs;      // 6

        short           yoffs;      // 8

        unsigned short  width;      // 10

        unsigned short  height;     // 12

	unsigned short tileheight;

} spriteDC_t;
*/

// np2 w
// np2 h
	uint32_t wp2 = np2((uint32_t)SwapShort(w));
	uint32_t hp2 = np2((uint32_t)SwapShort(h));

printf("%d %d %d %d\n", SwapShort(w), wp2, SwapShort(h), hp2);
	uint32_t pw = (SwapShort(w) + 7) & ~7;
	uint8_t *outbuf = (uint8_t *)malloc(wp2*hp2);
	memset(outbuf,0,wp2*hp2);
	for (uint32_t sy = 0; sy < SwapShort(h); sy++) {
		memcpy(outbuf + (sy*wp2), buffer + (sy*pw), SwapShort(w));
	}

	FILE *twidfile;
	char twidname[256];
	sprintf(twidname, "%s.lmp", fn);
	twidfile = fopen(twidname, "wb+");
		if (!twidfile)
	{
		fprintf(stderr, "Unable to open file %s", twidname);
		return -1;
	}

	unsigned short tiles = SwapShort((unsigned short)1);
	short scompd = SwapShort((short)-1);

	fwrite(&tiles,sizeof(unsigned short),1,twidfile);
	fwrite(&scompd,sizeof(unsigned short),1,twidfile);
	fwrite(&cmpsize,sizeof(unsigned short),1,twidfile);
	fwrite(&xofs,sizeof(unsigned short),1,twidfile);
	fwrite(&yofs,sizeof(unsigned short),1,twidfile);
	fwrite(&w,sizeof(unsigned short),1,twidfile);
	fwrite(&h,sizeof(unsigned short),1,twidfile);
	fwrite(&tiles,sizeof(unsigned short),1,twidfile);

	fwrite(outbuf,wp2*hp2,1,twidfile);
	fclose(twidfile);
	free(buffer);
	free(outbuf);
	return 0;
}
