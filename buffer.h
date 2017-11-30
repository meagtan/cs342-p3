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
	struct student *buf;
	pthread_cond_t full, empty; // represents whether buffer NOT full or empty
	pthread_mutex_t mutex;
	int start, end, size;
	int finished; // whether producer has finished adding students
};

#define EMPTY(buf) ((buf).size == 0)
#define FULL(buf)  ((buf).size == bufsiz)

int bufsiz;

// initialize buffer and mutexes
void buffer_init(struct buffer *buf);

// copy student into buffer
void buffer_push(struct buffer *buf, struct student *st);

// copy student from buffer
// return value: whether producer will continue or not
int buffer_pop(struct buffer *buf, struct student *st);

// free buffer
void buffer_free(struct buffer *buf);

#endif