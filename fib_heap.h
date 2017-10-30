#ifndef FIB_HEAP_H
#define FIB_HEAP_H

#include "graph.h"

/* DATA STRUCTURES
 *
 * store roots in array, double length when necessary
 */

struct fib_heap {
	int n;		// number of elements (i.e. elements up to (n-1) are valid)
	int max;	// space (i.e. last element possible is (max-1)
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
};

extern struct fib_node *fib_heap_insert(struct fib_heap *heap, void *value, double key);

extern struct fib_node *fib_heap_extract_min(struct fib_heap *heap);

extern void fib_heap_decrease_key(struct fib_heap *heap, struct fib_node *v, double key);

#endif
