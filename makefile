all: pcsync.c heap.c buffer.c
	gcc -o pcsync pcsync.c heap.c buffer.c -lpthread
