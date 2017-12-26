## Author: Nathan Thomas

all: reading.o reading.so mapping.o mapping.so nlcnt.o

reading.o: reading.c
	gcc -c -fPIC reading.c -o reading.o
	
reading.so: reading.o
	gcc -shared -lm -o reading.so reading.o
	
mapping.o: mapping.c
	gcc -c -fPIC mapping.c -o mapping.o
	
mapping.so: mapping.o
	gcc -shared -lm -o mapping.so mapping.o
	
nlcnt.o: nlcnt.c
	gcc -o nlcnt.o nlcnt.c -ldl

clean:
	rm mapping.c~ reading.c~ nlcnt.c~ makefile~