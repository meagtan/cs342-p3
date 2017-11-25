all: pcsync.c heap.c
	gcc -o pcsync pcsync.c heap.c -lpthread
