#include <pthread.h>

#include "heap.h"
#include "buffer.h" // contains maximum buffer size

// initial size of heap
#define MAXSTUDENTS 100

// shared parameters
int N;
char *input, *output;

// array of buffers etc.
// avail.mutex has lower priority than the mutex of each buffer
struct buffer *bufs;
struct heap avail; // need to consume from available buffer with largest size, in order to keep buffers full the least

// producer, consumer
void *producer(void *args); // takes producerid as argument
void *consumer(void *args);

