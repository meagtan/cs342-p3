#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "pcsync.h"

int main(int argc, char *argv[])
{
	// retrieve arguments

	if (argc != 5) {
		fprintf(stderr, "Format: pcsync <N> <buffersize> <infilename> <outputfilename>\n");
		return 1;
	}

	N      = atoi(argv[1]);
	bufsiz = atoi(argv[2]);
	input  = argv[3];
	output = argv[4];

	int i;
	pthread_t *thr = malloc((N+1) * sizeof(pthread_t)); // all threads, including consumer

	// initialize shared mutexes, condition variables
	heap_init(avail, N);

	// create N buffers, create producers
	bufs = malloc(N * sizeof(struct buffer));
	for (i = 0; i < N; ++i) {
		buffer_init(bufs[i]);
		pthread_create(thr+i, NULL, producer, (void *) i);
	}

	// create consumer
	pthread_create(thr+i, NULL, consumer, NULL);

	// wait for all threads to finish // TODO is this correct?
	for (i = 0; i <= N; ++i)
		pthread_join(thr+i);

	// close buffers
	for (i = 0; i < N; ++i)
		buffer_free(bufs[i]);
	free(bufs);
	// release mutexes etc. if necessary
}

void *producer(void *args)
{
	int id = (int) args;
	struct student st;
	int prodid; // producer id of each entry read

	// read input file
	FILE *f = fopen(input, "r");
	if (!f) {
		fprintf(stderr, "Error: file %s does not exist\n", input);
		exit(1);
	}

	while (fscanf(input, " %d %d %s %s %f", prodid, st.sid, st.firstname, st.lastname, st.cpga)) {
		if (prodid == id) {
			pthread_mutex_lock(&avail.mutex);
			pthread_mutex_lock(&bufs[id].mutex);

			// wait until buffer not full
			while (FULL(bufs[id]))
				pthread_cond_wait(&bufs[id].full, &bufs[id].mutex);

			// add st to buffer
			bufs[id].buf[bufs[id].end] = st; // struct assignment copies each member of struct
			bufs[id].size++;
			bufs[id].end++;
			bufs[id] %= bufsiz;

			// signal that buffer not empty
			heap_decrkey(&avail, -bufs[id].size, (void *) id);
			pthread_cond_signal(&bufs[id].empty, &bufs[id].mutex);
			pthread_mutex_unlock(&bufs[id].mutex);
			pthread_mutex_unlock(&avail.mutex);
		}
	}

	finished++;

	fclose(f);
	pthread_exit(NULL);
}

void *consumer(void *args)
{
	// store rbtree or something to order entries
	struct heap students;
	struct student *st;

	heap_init(&students, MAXSTUDENTS);

	// read from each buffer
	while (finished != N) {
		pthread_mutex_lock(&avail.mutex);

		// wait for nonempty buffer
		// maybe incorporate this under heap
		while (heap_empty(&avail))
			pthread_cond_wait(&avail.empty, &avail.mutex);
		int id = (int) heap_pop(&avail);

		// consume from bufs[id]
		pthread_mutex_lock(&bufs[id].mutex);

		// wait until buffer nonempty, just in case
		while (EMPTY(bufs[id])
			pthread_cond_wait(&bufs[id].empty, &bufs[id].mutex);

		// remove element from buffer
		st = malloc(sizeof(struct student));
		*st = bufs[id].buf[bufs[id].start];
		bufs[id].size--;
		bufs[id].start++;
		bufs[id].start %= bufsiz;

		heap_push(&students, st->studentid, st);

		pthread_cond_signal(&bufs[id].full, &bufs[id].mutex);
		pthread_mutex_unlock(&bufs[id].mutex);

		pthread_mutex_unlock(&avail.mutex);
	}

	// output students
	FILE *f = fopen(output, "w");
	if (!f) {
		fprintf(stderr, "Error: could not open file %s.\n", output);
		exit(1);
	}

	while (!heap_empty(&students)) {
		st = heap_pop(&students);
		fprintf(f, "%d %s %s %f\n", st->studentid, st->firstname, st->lastname, st->cgpa);
		free(st);
	}

	heap_free(&students);
	fclose(f);
	pthread_exit(NULL);
}