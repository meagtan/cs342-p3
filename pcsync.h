/**
 * CS 342 Project 3
 * Ata Deniz Aydin
 * 21502637
 *
 * Shared variables for threads and other declarations
 */

#include <pthread.h>

#include "heap.h"
#include "buffer.h" // contains maximum buffer size

// initial size of heap of students
#define MAXSTUDENTS 100

// shared parameters
int N;
char *input, *output;

// array of buffers and minheap storing buffers ordered by size
// instead of looping through available buffers, more efficient to keep them in a heap,
//  pop largest buffer once it is available, process buffer and add it back to heap
// after adding to buffer, producer increases priority of buffer
// after removing from buffer, consumer decreases priority of buffer
// avail.mutex has lower priority than the mutex of each buffer,
//  should be held while buffer locked as the size of the buffer may change otherwise
struct buffer *bufs;
struct heap avail; // need to consume from available buffer with largest size, in order to keep buffers full the least

// producer, consumer runner functions
void *producer(void *args); // takes producerid as argument
void *consumer(void *args);

