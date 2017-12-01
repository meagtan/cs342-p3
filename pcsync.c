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
	remaining = N;

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

	printf("producer %d entered\n", id);

	// read input file
	FILE *f = fopen(input, "r");
	if (!f) {
		fprintf(stderr, "Error: file %s does not exist\n", input);
		exit(1);
	}

	st = malloc(sizeof(struct student));
	while (!feof(f) && fscanf(f, "%d %d %s %s %lf", &prodid, &st->sid, st->firstname, st->lastname, &st->cgpa)) {
		if (prodid == id) {
			// pthread_mutex_lock(&avail.mutex);
			buffer_push(bufs+id, st);
//			printf("producer %d %d %s %s %lf\n", prodid, st.sid, st.firstname, st.lastname, st.cgpa);

			// signal that buffer not empty
			pthread_mutex_lock(&bufs[id].mutex);
			if (!EMPTY(bufs[id])) {
				heap_decrkey(&avail, -bufs[id].size, &bufs[id].id);
				// pthread_cond_signal(&avail.empty);
				// printf("added bufs[%ld] to avail with size %d, avail.size = %d\n", id, bufs[id].size, avail.size);
			}
			pthread_mutex_unlock(&bufs[id].mutex);

			st = malloc(sizeof(struct student));

			// pthread_mutex_unlock(&avail.mutex);
		}
		fscanf(f, " \n"); // skip line
/*		pthread_mutex_lock(&avail.mutex);
		//printf("waiting for nonempty id\n");
		for (int i = 0; i < avail.size; ++i)
			printf("(%d,%d) ", *((int *) avail.vals[i]), avail.keys[i]);
		printf("avail.size = %d\n", avail.size);
		pthread_mutex_unlock(&avail.mutex);
*/
	}
	free(st);
	buffer_push(bufs+id, NULL);
	/*
	pthread_mutex_lock(&bufs[id].mutex);
	bufs[id].finished = 1;
	if (bufs[id].size == 0) {
		pthread_mutex_lock(&avail.mutex);
		remaining--;
		pthread_mutex_unlock(&avail.mutex);
	}
	pthread_cond_signal(&bufs[id].empty);
	pthread_mutex_unlock(&bufs[id].mutex);
	*/

	printf("producer %d finished\n", id);

	fclose(f);
	pthread_exit(NULL);
}

void *consumer(void *args)
{
	struct heap students;
	struct student *st;
	// int remaining = N; // number of producers that haven't finished and been completely consumed

	heap_init(&students, MAXSTUDENTS);

	// read from each buffer
	// pthread_mutex_lock(&avail.mutex);
	while (remaining) {
		// pthread_mutex_unlock(&avail.mutex);
		// wait for nonempty buffer
		// maybe incorporate this under heap
/*		pthread_mutex_lock(&avail.mutex);
		for (int i = 0; i < avail.size; ++i)
			printf("(%d,%d) ", *((int *) avail.vals[i]), avail.keys[i]);
		printf("avail.size = %d\n", avail.size);
		pthread_mutex_unlock(&avail.mutex);
*/
		int id = *((int *) heap_pop(&avail));
		printf("received id %d\n", id);
		// int id = 0;

		// st = malloc(sizeof(struct student));

		// remove element from buffer; continue without processing if no element left (producer terminated)
		if (!(st = buffer_pop(bufs+id))) {
			//free(st);
			remaining--;
			continue;
		}

		printf("popped element\n");
		pthread_mutex_lock(&bufs[id].mutex);
		if (!EMPTY(bufs[id])) {
			// not push as producer might have pushed itself to avail beforehand
			heap_decrkey(&avail, -bufs[id].size, (int *) &bufs[id].id);
			printf("added bufs[%d] to avail with size %d\n", id, bufs[id].size);
		}
		pthread_mutex_unlock(&bufs[id].mutex);
		//printf("consumer %d %d %s %s %lf\n", id, st->sid, st->firstname, st->lastname, st->cgpa);

		heap_push(&students, st->sid, st);

		// pthread_mutex_lock(&avail.mutex);
	}
	// pthread_mutex_unlock(&avail.mutex);

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
