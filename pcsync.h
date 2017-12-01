#include <pthread.h>

#include "heap.h"
#include "buffer.h" // contains maximum buffer size

// initial size of heap
#define MAXSTUDENTS 100

// shared parameters
int N;
char *input, *output;

// array of buffers etc.
// the mutexes of each buffer have to be locked before avail.mutex
struct buffer *bufs;
struct heap avail; // need to consume from available buffer with largest size, in order to keep buffers full the least
int remaining; // number of producers left to consume, initialized to N, controlled with avail.mutex

// producer, consumer
void *producer(void *args); // takes producerid as argument
void *consumer(void *args);

