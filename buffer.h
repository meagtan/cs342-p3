/**
 * CS 342 Project 3
 * Ata Deniz Aydin
 * 21502637
 *
 * Thread-safe buffer for students
 */

#include <pthread.h>

#ifndef __BUFFER_H
#define __BUFFER_H

struct student {
	int sid;
	char firstname[64];
	char lastname[64];
	double cgpa;
};

// buffer shared between producer and consumer
struct buffer {
	struct student **buf;
	pthread_cond_t full, empty; // represents whether buffer NOT full or empty
	pthread_mutex_t mutex;
	int start, end, size;
	int id; // hack, stores # of producer associated with buffer
};

#define EMPTY(buf) ((buf).size == 0)
#define FULL(buf)  ((buf).size == bufsiz)

int bufsiz;

// initialize buffer and mutexes
void buffer_init(struct buffer *buf, int id);

// copy student into buffer, waiting until not full
void buffer_push(struct buffer *buf, struct student *st);

// copy student from buffer, waiting until not empty
struct student *buffer_pop(struct buffer *buf);

// free buffer
void buffer_free(struct buffer *buf);

#endif
