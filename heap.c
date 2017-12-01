#include <stdlib.h>

#include "heap.h"

#define PARENT(i) (((i)-1)>>1)
#define LEFTCHILD(i) (2*(i)+1)

void heap_init(struct heap *h, int maxsize)
{
	h->keys = malloc(maxsize * sizeof(int));
	h->vals = malloc(maxsize * sizeof(void *));

	h->size = 0;
	h->maxsize = maxsize;

	pthread_mutex_init(&h->mutex, NULL);
	pthread_cond_init(&h->empty, NULL);
}

int heap_empty(struct heap *h)
{
	return h->size == 0;
}

void heap_push(struct heap *h, int key, void *val)
{
	pthread_mutex_lock(&h->mutex);

	if (h->size == h->maxsize) {
		h->maxsize <<= 1;
		h->keys = realloc(h->keys, h->maxsize * sizeof(int));
		h->vals = realloc(h->vals, h->maxsize * sizeof(void *));
	}

	int i;
	for (i = h->size++; i && key < h->keys[PARENT(i)]; i = PARENT(i)) {
		h->keys[i] = h->keys[PARENT(i)];
		h->vals[i] = h->vals[PARENT(i)];
	}
	h->keys[i] = key;
	h->vals[i] = val;

	pthread_cond_signal(&h->empty);
	pthread_mutex_unlock(&h->mutex);
}

void heap_decrkey(struct heap *h, int key, void *val)
{
	int i;

	pthread_mutex_lock(&h->mutex);

	// search for val in heap
	for (i = 0; i < h->size && h->vals[i] != val; ++i);

	// add val to heap if not in heap
	if (i == h->size) {
		//printf("%d not found\n", *((int *) val));
		if (i == h->maxsize) {
			h->maxsize <<= 1;
			h->keys = realloc(h->keys, h->maxsize * sizeof(int));
			h->vals = realloc(h->vals, h->maxsize * sizeof(void *));
		}
		h->keys[i] = key; // unnecessary
		h->vals[i] = val;
		h->size++;
	}

	// sift up
	for (; i && key < h->keys[PARENT(i)]; i = PARENT(i)) {
		h->keys[i] = h->keys[PARENT(i)];
		h->vals[i] = h->vals[PARENT(i)];
	}
	h->keys[i] = key;
	h->vals[i] = val;

	pthread_cond_signal(&h->empty);
	pthread_mutex_unlock(&h->mutex);
}

// is this thread safe?
void *heap_min(struct heap *h)
{
	return heap_empty(h) ? NULL : h->vals[0];
}

void *heap_pop(struct heap *h)
{
	pthread_mutex_lock(&h->mutex);

	while (heap_empty(h))
		pthread_cond_wait(&h->empty, &h->mutex);

	void *val = h->vals[0];
	h->size--;

	int i, j;
	for (i = 0, j = LEFTCHILD(i); j < h->size; i = j, j = LEFTCHILD(i)) {
		if (j+1 != h->size && h->keys[j+1] < h->keys[j])
			j = j+1;

		if (h->keys[h->size] < h->keys[j])
			break;
		h->keys[i] = h->keys[j];
		h->vals[i] = h->vals[j];
	}

	h->keys[i] = h->keys[h->size];
	h->vals[i] = h->vals[h->size];

	pthread_mutex_unlock(&h->mutex);
	return val;
}

void heap_free(struct heap *h)
{
	free(h->keys);
	free(h->vals);

	pthread_cond_destroy(&h->empty);
	pthread_mutex_destroy(&h->mutex);
}
