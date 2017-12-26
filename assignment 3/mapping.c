#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const int NEWLINE = 10;

int my_cnt(const char *file){
	int fd, filesize;
	char* mapping;
	struct stat status;
	fd = open(file, O_RDWR);
	if (fd < 0){
		fprintf(stderr, "%s: open failed %s\n", file, strerror(errno));
		exit(1);
	}
	int filestatus = fstat(fd, &status);
	filesize = status.st_size;																	//get the number of characters in the file
	mapping = (char *)mmap(0, filesize, PROT_READ, MAP_PRIVATE, fd, 0);			//map the file to an array in memory
	if (mapping == MAP_FAILED){
		fprintf(stderr, "%s: mmap failed %s\n", file, strerror(errno));
		exit(1);
	}
		
	int i, count;
	count = 0;
	for (i = 0; i < filesize; i++){															//read the array checking for newline characters
		if (mapping[i] == NEWLINE)
			count++;
	}
	close(fd);
	return count;
}