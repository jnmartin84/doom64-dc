#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "lumpinfo.h"

#define NUMLUMPS 311

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

int numlumps;
char pwad[4] = {'P','W','A','D'};
lumpinfo_t *lumpinfo;

#define LUMPDATASZ (256*1024)
char lumpdata[LUMPDATASZ];

char lumpfn[256];
char encfn[256];

int main(int argc, char *argv) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;

	fp = fopen("newlumpslist.txt", "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

// if line is S2_START , size is 0. filepos is 12 entry is uncompressed
// anything else not begin with S2_,
// open the file for that lump name
// read the size of compressed lump
// filepos of lump 1 is 12
// filepos of lump 2 is 12 + sizeof lump 1 file
// etc
// S2_END , size is 0, filepos is whatever last graphic lump filepos
// next up is the infotable
// filepos is actual filepos
// size is whatever the size would be when decompressed (need to know this too)
// name is new lumpname & 0x8fffffff

	numlumps = NUMLUMPS;//wadfileptr.numlumps;
	lumpinfo = (lumpinfo_t *)malloc(numlumps * sizeof(lumpinfo_t));

	int fd = open("alt.wad", O_RDWR | O_CREAT);// | O_BINARY);

	for(int i=0;i<4;i++) {
		write(fd, &pwad[i], 1);
	}
	write(fd, &numlumps, 4);

	int lastofs = 4 + 4 + 4;

	for (int i=0;i<numlumps;i++) {
		char nextlumpname[9];
		memset(nextlumpname,0,9);
		getline(&line, &len, fp);
		for(int j=0;j<9;j++) {
			if(line[j] != '\r' && line[j] != '\n') {
				nextlumpname[j] = line[j];
			} else {
				break;
			}
		}

		memset(lumpinfo[i].name, 0, 8);
		memcpy(lumpinfo[i].name, nextlumpname, 8);
		if (nextlumpname[0] != 'S' || (nextlumpname[0] == 'S' && nextlumpname[1] != '2')) {
			lumpinfo[i].name[0] |= 0x80;
		}
		printf("%s\n", nextlumpname);

		lumpinfo[i].filepos = lastofs;

		if(nextlumpname[0] == 'S' && nextlumpname[1] == '2') {
			lumpinfo[i].size = 0;
		} else {
			sprintf(lumpfn, "%s.twid", nextlumpname);
			sprintf(encfn, "%s.twid.enc", nextlumpname);
			int lfd = open(lumpfn, O_RDONLY);
			int efd = open(encfn, O_RDONLY);
			unsigned long llen = lseek(lfd, 0, SEEK_END);
			unsigned long elen = lseek(efd, 0, SEEK_END);
			close(lfd);
			close(efd);
			lumpinfo[i].size = llen;
			lastofs = lastofs + ((elen + 3) & ~3);
		}
	}

	write(fd, &lastofs, 4);

	fclose(fp);
	fp = fopen("newlumpslist.txt", "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	for (int i=0;i<numlumps;i++) {
		char nextlumpname[9];
		memset(nextlumpname,0,9);
		getline(&line, &len, fp);
		for(int j=0;j<9;j++) {
			if(line[j] != '\r' && line[j] != '\n') {
				nextlumpname[j] = line[j];
			} else {
				break;
			}
		}

		if(lumpinfo[i].size) {
			sprintf(encfn, "%s.twid.enc", nextlumpname);
			printf("reading %s \n", encfn);
			int efd = open(encfn, O_RDONLY);
			unsigned long elen = lseek(efd, 0, SEEK_END);
			unsigned long pad_elen = (elen + 3) & ~3;
			lseek(efd, 0, SEEK_SET);
			char *ebuf = malloc(elen);
			read(efd, ebuf, elen);
			close(efd);
			write(fd, ebuf, pad_elen);
			free(ebuf);
		}
	}

	for (int i=0;i<numlumps;i++) {
		write(fd, (void*)(&lumpinfo[i]), sizeof(lumpinfo_t));
	}

	close(fd);
	free(lumpinfo);
	return 0;
}
