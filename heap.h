// Thread-safe minheap with integer keys and void * values
// Used to sort buffers by size and students by ID

#include <pthread.h>

struct heap {
	int *keys;
	void **vals;
	int size, maxsize; // maxsize doubled every time size reaches maxsize

	pthread_mutex_t mutex;
	pthread_cond_t empty;
};

void heap_init(struct heap *, int maxsize);

int heap_empty(struct heap *);

void heap_push(struct heap *, int key, void *val);

// searches for val in heap, if found decreases its key to key, else pushes it to heap
void heap_decrkey(struct heap *, int key, void *val);

void *heap_min(struct heap *);

void *heap_pop(struct heap *);
