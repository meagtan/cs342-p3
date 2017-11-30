#include <pthread.h>
#include "heap.h"

struct student {
	int sid;
	char firstname[64];
	char lastname[64];
	double cgpa;
};

// buffer shared between producer and consumer
struct buffer {
	struct student *buf;
	pthread_cond_t full, empty; // represents whether buffer NOT full or empty
	pthread_mutex_t mutex;
	int start, end, size;
};

#define EMPTY(buf) ((buf).size == 0)
#define FULL(buf)  ((buf).size == bufsiz)

// initial size of heap
#define MAXSTUDENTS 32

// shared parameters
int N, bufsiz;
char *input, *output;

// array of buffers, mutexes
struct buffer *bufs;
struct heap avail; // need to consume from available buffer with largest size, in order to keep buffers full the least
int finished = 0; // number of finished producers; monotone increasing, shouldn't need to be synchronized

// producer, consumer
void *producer(void *args); // takes producerid as argument
void *consumer(void *args);

// initialize buffer and mutexes
void buffer_init(struct buffer *buf);

// free buffer
void buffer_free(struct buffer *buf);
