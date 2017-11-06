#define _POSIX_C_SOURCE 201112L

#ifndef FIB_HEAP_H
#define FIB_HEAP_H

#include "graph.h"

#include "config.h"

/* DATA STRUCTURES
 *
 * store roots in array, double length when necessary
 */

struct fib_heap {
	int n;		// how long is b (max element is n-1)
	struct fib_node **b; // array of pointers to fib_nodes
};

struct fib_node {
	double key;
	void *val;

	int phi;
	int degree;
	struct fib_node *parent;
	struct fib_node *child;		// "prev-most" child
	struct fib_node *next;
	struct fib_node *prev;
};

extern struct fib_node *fib_heap_insert(struct fib_heap *heap, void *value, double key);

extern struct fib_node *fib_heap_extract_min(struct fib_heap *heap);

extern void fib_heap_decrease_key(struct fib_heap *heap, struct fib_node *v, double key);

extern void fib_print_heap(struct fib_heap *heap);

extern void fib_heap_free(struct fib_heap *heap);

#endif
