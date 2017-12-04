/**
 * CS 342 Project 3
 * Ata Deniz Aydin
 * 21502637
 *
 * Implementation of buffer.h
 */

#include "buffer.h"

#include <stdio.h> // testing
#include <stdlib.h>

void buffer_init(struct buffer *b, int id)
{
	b->buf = malloc(bufsiz * sizeof(struct student *));
	b->start = b->end = b->size = 0;
	b->id = id;

	pthread_mutex_init(&b->mutex, NULL);
	pthread_cond_init(&b->empty, NULL);
	pthread_cond_init(&b->full, NULL);
}

void buffer_push(struct buffer *buf, struct student *st)
{
	pthread_mutex_lock(&buf->mutex);

	// wait until buffer not full
	while (FULL(*buf)) {
		// printf("buffer full\n");
		pthread_cond_wait(&buf->full, &buf->mutex);
	}

	// add st to buffer
	buf->buf[buf->end] = st; // struct assignment copies each member of struct
	buf->size++;
	buf->end++;
	buf->end %= bufsiz;

	// if (st)
	// 	printf("producer %d %s %s %lf\n", st->sid, st->firstname, st->lastname, st->cgpa);

	// signal that buffer not empty
	pthread_cond_signal(&buf->empty);
	pthread_mutex_unlock(&buf->mutex);
}

struct student *buffer_pop(struct buffer *buf)
{
	struct student *st;

	pthread_mutex_lock(&buf->mutex);

	// printf("acquired lock\n");
	// wait until buffer nonempty; should not wait if producer already concluded
	while (EMPTY(*buf)) {
		// printf("buffer empty\n");
		pthread_cond_wait(&buf->empty, &buf->mutex);
	}

	// remove element from buffer
	st = buf->buf[buf->start];
	buf->size--;
	buf->start++;
	buf->start %= bufsiz;

	// if (st)
	// 	printf("consumer %d %s %s %lf\n", st->sid, st->firstname, st->lastname, st->cgpa);

	// int complete = EMPTY(*buf) && buf->finished;

	pthread_cond_signal(&buf->full);
	pthread_mutex_unlock(&buf->mutex);

	return st;
}

void buffer_free(struct buffer *b)
{
	free(b->buf);
	pthread_cond_destroy(&b->full);
	pthread_cond_destroy(&b->empty);
	pthread_mutex_destroy(&b->mutex);
}
