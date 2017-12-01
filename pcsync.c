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
	// remaining = N;

	long int i;
	pthread_t *thr = malloc((N+1) * sizeof(pthread_t)); // all threads, including consumer

	// initialize shared mutexes, condition variables
	heap_init(&avail, N);

	// create N buffers, create producers
	bufs = malloc(N * sizeof(struct buffer));
	for (i = 0; i < N; ++i) {
		buffer_init(bufs+i, i);
		pthread_create(thr+i, NULL, producer, &bufs[i].id);
	}

	// create consumer
	pthread_create(thr+i, NULL, consumer, NULL);

	// wait for all threads to finish // TODO is this correct?
	for (i = 0; i <= N; ++i)
		pthread_join(thr[i], NULL);

	// close buffers
	for (i = 0; i < N; ++i)
		buffer_free(bufs+i);
	free(bufs);

	// release mutexes etc. if necessary

	free(thr);
	heap_free(&avail);
	pthread_exit(NULL);
}

void *producer(void *args)
{
	int id = *((int *) args);
	struct student *st;
	int prodid; // producer id of each entry read

	// printf("producer %d entered\n", id);

	// read input file
	FILE *f = fopen(input, "r");
	if (!f) {
		fprintf(stderr, "Error: file %s does not exist\n", input);
		exit(1);
	}

	st = malloc(sizeof(struct student));
	while (!feof(f) && fscanf(f, "%d %d %s %s %lf", &prodid, &st->sid, st->firstname, st->lastname, &st->cgpa)) {
		if (prodid == id) {
			buffer_push(bufs+id, st);

			// signal that buffer not empty
			pthread_mutex_lock(&bufs[id].mutex);
			if (!EMPTY(bufs[id])) {
				heap_decrkey(&avail, -bufs[id].size, &bufs[id].id);
				// printf("added bufs[%ld] to avail with size %d, avail.size = %d\n", id, bufs[id].size, avail.size);
			}
			pthread_mutex_unlock(&bufs[id].mutex);

			// st already sent, allocate new block
			st = malloc(sizeof(struct student));
		}
		fscanf(f, " \n"); // skip line
	}
	free(st); // last block read was invalid
	buffer_push(bufs+id, NULL);

	// signal last entry
	pthread_mutex_lock(&bufs[id].mutex);
	if (!EMPTY(bufs[id]))
		heap_decrkey(&avail, -bufs[id].size, &bufs[id].id);
	pthread_mutex_unlock(&bufs[id].mutex);

	printf("producer %d finished\n", id);

	fclose(f);
	pthread_exit(NULL);
}

void *consumer(void *args)
{
	struct heap students;
	struct student *st;
	int remaining = N; // number of producers that haven't finished and been completely consumed

	heap_init(&students, MAXSTUDENTS);

	// read from each buffer
	// pthread_mutex_lock(&avail.mutex);
	while (remaining) {
		int id = *((int *) heap_pop(&avail));
		// printf("received id %d\n", id);

		// remove element from buffer; continue without processing if no element left (producer terminated)
		if (!(st = buffer_pop(bufs+id))) {
			remaining--;
			continue;
		}

		pthread_mutex_lock(&bufs[id].mutex);
		if (!EMPTY(bufs[id])) {
			// decrkey not push as producer might have pushed itself to avail beforehand
			heap_decrkey(&avail, -bufs[id].size, (int *) &bufs[id].id);
			// printf("added bufs[%d] to avail with size %d\n", id, bufs[id].size);
		}
		pthread_mutex_unlock(&bufs[id].mutex);

		heap_push(&students, st->sid, st);
	}

	printf("producers finished\n");

	// output students
	FILE *f = fopen(output, "w");
	if (!f) {
		fprintf(stderr, "Error: could not open file %s.\n", output);
		exit(1);
	}

	while (!heap_empty(&students)) {
		st = heap_pop(&students);
		fprintf(f, "%d %s %s %.2lf\n", st->sid, st->firstname, st->lastname, st->cgpa);
		free(st);
	}

	heap_free(&students); // should also free each entry in students
	fclose(f);
	pthread_exit(NULL);
}
