#include <pthread.h>

#include "heap.h"
#include "buffer.h" // contains maximum buffer size

// initial size of heap
#define MAXSTUDENTS 100

// shared parameters
int N;
char *input, *output;

// array of buffers, mutexes
struct buffer *bufs;
struct heap avail; // need to consume from available buffer with largest size, in order to keep buffers full the least
int finished = 0; // number of finished producers; monotone increasing, shouldn't need to be synchronized // not used

// producer, consumer
void *producer(void *args); // takes producerid as argument
void *consumer(void *args);

