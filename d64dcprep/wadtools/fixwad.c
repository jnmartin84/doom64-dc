#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "lumpinfo.h"

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
void *wad_inbuffer;
void *wad_outbuffer;
char pwad[4] = {'I','W','A','D'};
lumpinfo_t *lumpinfo;
#define LUMPDATASZ (256*1024)
char lumpdata[LUMPDATASZ];
int main(int argc, char *argv) {
	char *infile = "doom64.wad";

	int wad_fd = open(infile, O_RDONLY);
	read(wad_fd, (void*)&wadfileptr, sizeof(wadinfo_t));
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
	lseek(wad_fd, infotableofs, SEEK_SET);
	read(wad_fd, (void*)lumpinfo, numlumps * sizeof(lumpinfo_t));

	lseek(wad_fd, 0, SEEK_SET);

	int fd = open("out.wad", O_RDWR | O_CREAT);// | O_BINARY);

//	write(fd, "PWAD", 4);
	for(int i=0;i<4;i++) {
		write(fd, &pwad[i], 1);
	}
	write(fd, &numlumps, 4);
	int lastofs = 4 + 4 + 4;

	for (int i=0;i<numlumps;i++) {
//		if((i < 349) || (i > 923)) {
if ( ((i < 349) || (i > 923)) ||

((names[i][0] == 'P') && (names[i][1] == 'A') && (names[i][2] == 'L')) ) {
			int orig_size = lumpinfo[i].size;
			if (lumpinfo[i].name[0] & 0x80) {
				orig_size = lumpinfo[i+1].filepos - lumpinfo[i].filepos;
			}
			int padded_size = (orig_size + 3) & ~3;
			lastofs = lastofs + padded_size;
		}
		else {
			char *name = names[i];
			char fullname[256];
			sprintf(fullname, "%s.twid.enc", name);
			int lfd = open(fullname, O_RDONLY);
			unsigned long fileLen;
			//Get file length
			fileLen = lseek(lfd, 0, SEEK_END);
printf("%s\n", name);
//printf("%s lfd %d len %d\n", name, lfd, fileLen);
			//fileLen=ftell(lfd);
			lseek(lfd, 0, SEEK_SET);
			//char *lmpbuf = malloc(fileLen);
			//read(lfd, lmpbuf, fileLen);
			int orig_size = fileLen;
			int padded_size = (orig_size + 3) & ~3;
			close(lfd);
			lastofs = lastofs + padded_size;
		}
	}

write(fd, &lastofs, 4);
lastofs = 12;

	for (int i=0;i<numlumps;i++) {
//if (  ((lumpinfo[i].name[0] == 'P') && (lumpinfo[i].name[1] == 'A') && (lumpinfo[i].name[2] == 'L')) ||
//(i < 349) || (i > 923)) {
//if ( ((names[i][0] == 'P') && (names[i][1] == 'A') && (names[i][2] == 'L')) ||
//(i < 349) || (i > 923)) {
if ( ((i < 349) || (i > 923)) ||

((names[i][0] == 'P') && (names[i][1] == 'A') && (names[i][2] == 'L'))
) {
int data_size;

		int orig_size = lumpinfo[i].size;
		int padded_size = (orig_size + 3) & ~3;
		data_size = orig_size;
		if(lumpinfo[i].name[0] & 0x80) {
			data_size = lumpinfo[i+1].filepos - lumpinfo[i].filepos;
		}
		memset(lumpdata, 0, LUMPDATASZ);

printf("%d data size %d\n", i, data_size);

		lseek(wad_fd, lumpinfo[i].filepos, SEEK_SET);
		read(wad_fd, lumpdata, data_size);
		data_size = (data_size+3)&~3;
		write(fd, lumpdata, data_size);
		lumpinfo[i].filepos = lastofs;
		lumpinfo[i].size = orig_size;
		lastofs = lastofs + data_size;//(data_size + 3) & ~3;//(lumpinfo[i].filepos - lumpinfo[i-1].filepos);
}

else {
char *name = names[i];
char origname[256];
char fullname[256];
sprintf(origname, "%s.twid", name);
sprintf(fullname, "%s.twid.enc", name);
int ofd = open(origname, O_RDONLY);
//printf("open %s %d\n", origname, ofd);
int lfd = open(fullname, O_RDONLY);
//printf("open %s %d\n", fullname, lfd);
unsigned long fileLen;
        fileLen = lseek(lfd, 0, SEEK_END);
        lseek(lfd, 0, SEEK_SET);
unsigned long origLen;
        origLen = lseek(ofd, 0, SEEK_END);
//printf("%s %d\n", origname, origLen);
//printf("%s %d\n", fullname, fileLen);

close(ofd);
int orig_size = fileLen;
int padded_size = (orig_size + 3) & ~3;
printf("%d data size %d\n", i, padded_size);
		memset(lumpdata, 0, LUMPDATASZ);
		read(lfd, lumpdata, orig_size);
		write(fd, lumpdata, padded_size);
close(lfd);

		lumpinfo[i].filepos = lastofs;
		lumpinfo[i].size = origLen;
		lastofs = lastofs + padded_size;
}

	}



	for (int i=0;i<numlumps;i++) {
		write(fd, (void*)(&lumpinfo[i]), sizeof(lumpinfo_t));
	}
//flush(fd);
	close(fd);
	close(wad_fd);

	return 0;
}
