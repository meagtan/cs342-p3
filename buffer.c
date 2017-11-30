#include "buffer.h"

#include <stdio.h> // testing
#include <stdlib.h>

void buffer_init(struct buffer *b)
{
	b->buf = malloc(bufsiz * sizeof(struct student));
	pthread_mutex_init(&b->mutex, NULL);
	pthread_cond_init(&b->empty, NULL);
	pthread_cond_init(&b->full, NULL);
	b->start = b->end = b->size = 0;
	b->finished = 0;
}

void buffer_push(struct buffer *buf, struct student *st)
{
	pthread_mutex_lock(&buf->mutex);

	// wait until buffer not full
	while (FULL(*buf)) {
		printf("buffer full\n");
		pthread_cond_wait(&buf->full, &buf->mutex);
	}

	// add st to buffer
	buf->buf[buf->end] = *st; // struct assignment copies each member of struct
	buf->size++;
	buf->end++;
	buf->end %= bufsiz;

	printf("producer %d %s %s %lf\n", st->sid, st->firstname, st->lastname, st->cgpa);

	// signal that buffer not empty
	// heap_decrkey(&avail, -bufs[id].size, (void *) id);
	pthread_cond_signal(&buf->empty);
	pthread_mutex_unlock(&buf->mutex);
}

int buffer_pop(struct buffer *buf, struct student *st)
{
	pthread_mutex_lock(&buf->mutex);

	// printf("acquired lock\n");
	// wait until buffer nonempty; should not wait if producer already concluded
	while (EMPTY(*buf) && !buf->finished) {
		printf("buffer empty\n");
		pthread_cond_wait(&buf->empty, &buf->mutex);
	}
	if (buf->finished)
		return 0;

	// remove element from buffer
	*st = buf->buf[buf->start];
	buf->size--;
	buf->start++;
	buf->start %= bufsiz;

	printf("consumer %d %s %s %lf\n", st->sid, st->firstname, st->lastname, st->cgpa);

	pthread_cond_signal(&buf->full);
	pthread_mutex_unlock(&buf->mutex);

	return 1;
}

void buffer_free(struct buffer *b)
{
	free(b->buf);
	pthread_cond_destroy(&b->full);
	pthread_cond_destroy(&b->empty);
	pthread_mutex_destroy(&b->mutex);
}
