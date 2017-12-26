#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

const int NEWLINE = 10;

int my_cnt(const char *file){
	int fd, filesize, num_blocks;
	char **page;
	struct stat status;
	fd = open(file, O_RDWR);
	if (fd < 0){
		fprintf(stderr, "%s: open failed %s\n", file, strerror(errno));
		exit(1);
	}
	long pg_size = sysconf(_SC_PAGESIZE);										//determine size of a page on current system
	int filestatus = fstat(fd, &status);
	filesize = status.st_size;
	num_blocks = ceil((double)filesize / pg_size);							//determine number of blocks required to hold the file
	int i, j, k, count;
	page = malloc (sizeof(char*) * num_blocks);								//allocate memory for num_blocks pages of size pg_size
	for(i = 0; i < num_blocks; i++)
		page[i] = malloc(sizeof(char) * pg_size);
	for (j = 0; j < num_blocks; j++)
		if (pread(fd, page[j], pg_size, pg_size * j) < 0){					//read the page and store it
			fprintf(stderr, "%s: open failed %s\n", strerror(errno));
			exit(1);
		}
	count = 0;
	for (k = 0; k < num_blocks; k++) {
		int m;		
		for (m = 0; m < pg_size && m != filesize; m++){						//read each character in every page and count the newlines
			if (page[k][m] == NEWLINE)
				count++;
		}
	}
	close(fd);
	return count;
}