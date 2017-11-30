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

	long int i;
	pthread_t *thr = malloc((N+1) * sizeof(pthread_t)); // all threads, including consumer

	// initialize shared mutexes, condition variables
	// heap_init(avail, N);

	// create N buffers, create producers
	bufs = malloc(N * sizeof(struct buffer));
	for (i = 0; i < N; ++i) {
		buffer_init(bufs+i);
		pthread_create(thr+i, NULL, producer, (void *) i);
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
	// heap_free(avail, N);
	pthread_exit(NULL);
}

void *producer(void *args)
{
	long int id = (long int) args;
	struct student st;
	int prodid; // producer id of each entry read

	// read input file
	FILE *f = fopen(input, "r");
	if (!f) {
		fprintf(stderr, "Error: file %s does not exist\n", input);
		exit(1);
	}

	while (!feof(f) && fscanf(f, "%d %d %s %s %lf", &prodid, &st.sid, st.firstname, st.lastname, &st.cgpa)) {
		if (prodid == id) {
			// pthread_mutex_lock(&avail.mutex);
			buffer_push(bufs+id, &st);
//			printf("producer %d %d %s %s %lf\n", prodid, st.sid, st.firstname, st.lastname, st.cgpa);
/*
			pthread_mutex_lock(&bufs[id].mutex);

			// wait until buffer not full
			while (FULL(bufs[id])) {
				printf("buffer full\n");
				pthread_cond_wait(&bufs[id].full, &bufs[id].mutex);
			}

			// add st to buffer
			bufs[id].buf[bufs[id].end] = st; // struct assignment copies each member of struct
			bufs[id].size++;
			bufs[id].end++;
			bufs[id].end %= bufsiz;

			// signal that buffer not empty
			// heap_decrkey(&avail, -bufs[id].size, (void *) id);
			pthread_cond_signal(&bufs[id].empty);
			pthread_mutex_unlock(&bufs[id].mutex);
*/
			// pthread_mutex_unlock(&avail.mutex);
		}
		fscanf(f, " \n"); // skip line
	}

	pthread_mutex_lock(&bufs[id].mutex);
	bufs[id].finished = 1;
	finished++;
	pthread_cond_signal(&bufs[id].empty);
	pthread_mutex_unlock(&bufs[id].mutex);

	printf("producer finished\n");

	fclose(f);
	pthread_exit(NULL);
}

void *consumer(void *args)
{
	struct heap students;
	struct student *st;
	int consumed = 0; // number of producers finished and completely consumed

	heap_init(&students, MAXSTUDENTS);

	// read from each buffer
	while (consumed != N) {
		// wait for nonempty buffer
		// maybe incorporate this under heap
		// while (heap_empty(&avail))
		// 	pthread_cond_wait(&avail.empty, &avail.mutex);
		// int id = (int) heap_pop(&avail);
		int id = 0;

/*		// consume from bufs[id]
		pthread_mutex_lock(&bufs[id].mutex);

		// wait until buffer nonempty
		while (!bufs[id].finished && EMPTY(bufs[id])) {
			printf("buffer empty\n");
			pthread_cond_wait(&bufs[id].empty, &bufs[id].mutex);
		}
		if (bufs[id].finished && EMPTY(bufs[id])) {
			consumed++;
			continue;
		}
*/
		st = malloc(sizeof(struct student));

		// remove element from buffer; continue without processing if no element left (producer terminated)
		if (!buffer_pop(bufs+id, st)) {
			free(st);
			consumed++;
			continue;
		}

		// printf("consumer %d %d %s %s %lf\n", id, st->sid, st->firstname, st->lastname, st->cgpa);

		heap_push(&students, st->sid, st);


/*		pthread_cond_signal(&bufs[id].full);
		pthread_mutex_unlock(&bufs[id].mutex);
*/
		// pthread_mutex_unlock(&avail.mutex);
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
		fprintf(f, "%d %s %s %lf\n", st->sid, st->firstname, st->lastname, st->cgpa);
		free(st);
	}

	heap_free(&students); // should also free each entry in students
	fclose(f);
	pthread_exit(NULL);
}
