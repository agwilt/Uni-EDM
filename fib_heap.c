#include <math.h>
#include <stdlib.h>
#include <string.h>

// DEBUG
#include <stdio.h>
//#define DEBUG

#include "fib_heap.h"


// private declarations
static void fib_heap_plant(struct fib_heap *heap, struct fib_node *v);
static void fib_node_add_neighbour(struct fib_node *start, struct fib_node *end);

// functions


extern struct fib_node *fib_heap_insert(struct fib_heap *heap, void *val, double key)
{
	struct fib_node *node = malloc(sizeof(struct fib_node));
	node->key = key;
	node->val = val;
	node->phi = 0;
	node->degree = 0;
	node->parent = NULL;

	fib_heap_plant(heap, node);

#ifdef DEBUG
	printf("fib_heap_insert(%p, %p, %lf):\n", (void *) heap, (void *) val, key);
	fib_print_heap(heap);
	printf("Added node with key %lf at point %p\n", key, (void *) node);
#endif

	return node;
}

// WARNING: returns NULL if no node found, will break if b has garbage
extern struct fib_node *fib_heap_extract_min(struct fib_heap *heap)
{
	struct fib_node *r = NULL;
	double key = INFINITY;
	for (int i = 0; i<heap->n; ++i) {
		// don't need to check if root, since min. key must be root
		if (heap->b[i] != NULL && heap->b[i]->key < key) {
			r = heap->b[i];
			key = heap->b[i]->key;
		}
	}
#ifdef DEBUG
	if (r == NULL) printf("Warning: fib_heap_extract_min(%p) returned NULL\n", (void *) heap);
#endif

	// deal with kids
	if (r == NULL) return r;

	// remove from b FIRST, to avoid interference with kids
	heap->b[r->degree] = NULL;

	struct fib_node *next;
	for (struct fib_node *child = r->child; child != NULL; child=next) {
		child->parent = NULL;
		next = child->next;
		child->next = NULL;
		fib_heap_plant(heap, child);
	}
#ifdef DEBUG
	printf("node (key %lf) had degree %d and %d children\n", r->key, r->degree, i);
#endif

#ifdef DEBUG
	printf("fib_heap_extract_min(%p):\n", (void *) heap);
	fib_print_heap(heap);
#endif

	return r;
}

extern void fib_heap_decrease_key(struct fib_heap *heap, struct fib_node *v, double key)
{
	v->key = key;
	// first, find longest path
	struct fib_node *r = v->parent;
	while (r->parent != NULL && r->phi == 1) r = r->parent;
	// r is now root of path where all inner vertices z have phi(z) = 1

	struct fib_node *parent;
	v->phi = 1-v->phi; // so that v->phi isn't switched

	// TODO: merge these two loops if everything works

	while (v != r) {
		parent = v->parent;
		v->phi = 1-v->phi;
		parent->degree--;
		parent->child = v->next;
		v->parent = NULL;
		v->next = NULL;

		fib_heap_plant(heap, v);

		v = parent; // go on with parent
	}

	// now at root. Only plant if root is in fact a root (else flipping phi fixes everything)
	r->phi = 1 - r->phi;
	if (r->parent == NULL)
		fib_heap_plant(heap, r);

	return;
}

// private functions

static void fib_heap_plant(struct fib_heap *heap, struct fib_node *v)
{
	// grow b (and set things to NULL) if necessary
	if (v->degree >= heap->n) {
		heap->b = realloc(heap->b, (v->degree+1) * sizeof(struct fib_node*));
		// set to NULL
		for (; heap->n <= v->degree; heap->n++) {
			heap->b[heap->n] = NULL;
		}
	}

	struct fib_node *r = heap->b[v->degree];

	heap->b[v->degree] = NULL; // to avoid having rubbish lying around (old, rusty pointers being very, very dangerous)
	if (r != v && r != NULL && r->degree == v->degree && r->parent == NULL) {
		if (r->key <= v->key) {
			fib_node_add_neighbour(r, v);
			fib_heap_plant(heap, r);
		} else {
			fib_node_add_neighbour(v, r);
			fib_heap_plant(heap, v);
		}
	} else {
		heap->b[v->degree] = v;
	}

	return;
}

static void fib_node_add_neighbour(struct fib_node *start, struct fib_node *end)
{
#ifdef DEBUG
	if (end->parent != NULL) {
		fprintf(stderr, "Error: You no longer have an arborescence.\n");
		exit(1);
	}
#endif
	start->degree++;
	end->parent = start;
	end->next = start->child;
	start->child = end;

	return;
}





// DEBUG STUFF

static void fib_print_tabs(int n)
{
	if (n>0) printf("    ");
	for (int i = 1; i<n; ++i)
		printf("|   ");
}

static void fib_print_nodes(struct fib_node *v, int r)
{
	fib_print_tabs(r); printf("+------------------------------+\n");
	fib_print_tabs(r); printf("| address:      %p |\n", (void *) v);
	if (v) {
		fib_print_tabs(r); printf("| key:          %-14lf |\n", v->key);
		fib_print_tabs(r); printf("| value:        %-14p |\n", v->val);
		fib_print_tabs(r); printf("| phi:          %-14d |\n", v->phi);
		fib_print_tabs(r); printf("| degree:       %-14d |\n", v->degree);
		fib_print_tabs(r); printf("| parent:       %-14p |\n", (void *) v->parent);
		fib_print_tabs(r); printf("| child:        %-14p |\n", (void *) v->child);
		fib_print_tabs(r); printf("| next:         %-14p |\n", (void *) v->next);
		struct fib_node *child = v->child;
		while (child != NULL) {
			fib_print_nodes(child, r+1);
			child = child->next;
		}
	}
	fib_print_tabs(r); printf("+------------------------------+\n");
}

void fib_print_heap(struct fib_heap *heap)
{
	printf("    n = %d\n", heap->n);
	printf("    ---------------\n");
	printf("    Contents of b:\n");

	for (int i = 0; i<heap->n; ++i) {
		printf("    +===============+\n");
		printf("    | b[%d] = %p\n", i, (void *) heap->b[i]);
		fib_print_nodes(heap->b[i], 2);
	}
}
