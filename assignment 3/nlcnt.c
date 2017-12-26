#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

const char* mapping = "mapping";
const char* reading = "reading";
int (*linecount)(const char*);

int lsubstring(const char* str1, const char* str2){
	int i;	
	for (i = 0; str2[i] != 0; i++){
		if (str2[i] != str1[i])
			return -1;
	}	
	return 0;
}

int main(int argc, char* argv[]){
	if ( argc != 3){
		fprintf(stderr, "Wrong number of arguments!\n");
		exit(1);	
	}
	void *handle;
	if (lsubstring(mapping, argv[1]) == 0){
		handle = dlopen("./mapping.so", RTLD_LAZY);
		if (!handle) {
			fprintf(stderr, "%s\n", dlerror());
			exit(EXIT_FAILURE);
		}
	} else if (lsubstring(reading, argv[1]) == 0) {
		handle = dlopen("./reading.so", RTLD_LAZY);
		if (!handle) {
			fprintf(stderr, "%s\n", dlerror());
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "Argument 1 of nlcnt is invalid\n");	
		exit(1);
	} 
	linecount = dlsym(handle, "my_cnt");	
	printf("The number of newlines in %s are %d\n", argv[2], linecount(argv[2]));
	dlclose(handle);
	exit(1);
}