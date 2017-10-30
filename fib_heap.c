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
#ifdef DEBUG
	printf("fib_heap_insert(%p, %p, %lf):\n", heap, val, key);
	print_heap(heap);
	printf("Adding node with key %lf at point %p\n", key, node);
#endif
	struct fib_node *node = malloc(sizeof(struct fib_node));
	node->key = key;
	node->val = val;
	node->phi = 0;
	node->degree = 0;
	node->parent = NULL;

	fib_heap_plant(heap, node);

	return node;
}

// WARNING: returns NULL if no node found, will break if b has garbage
extern struct fib_node *fib_heap_extract_min(struct fib_heap *heap)
{
#ifdef DEBUG
	printf("fib_heap_extract_min(%p):\n", heap);
	print_heap(heap);
#endif

	struct fib_node *r = NULL;
	double key = INFINITY;
	for (int i = 0; i<heap->n; ++i) {
		// don't need to check if root, since min. key must be root
		if (heap->b[i] != NULL && heap->b[i]->key < key) {
			r = heap->b[i];
			key = heap->b[i]->key;
		}
	}

	// deal with kids
	struct fib_node *next;
	for (struct fib_node *child = r->child; child != NULL;) {
		child->parent = NULL;
		fib_heap_plant(heap, child);
		// got to next child
		next = child->next;
		child->next = NULL;
		child = next;
	}

	// dirty, has to do for now
	for (int i=0; i<heap->n; ++i) {
#ifdef DEBUG
		printf("removing %p: checking b[%d] = %p\n", r, i, heap->b[i]);
#endif
		if (heap->b[i] == r)
			heap->b[i] = NULL;
	}

	return r;
}

extern void fib_heap_decrease_key(struct fib_heap *heap, struct fib_node *v, double key)
{
	// special case: v is a root already
	if (v->parent == NULL) {
		v->key = key;
	}
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
	heap->n = ((heap->n)>(v->degree))?heap->n:(v->degree+1); // heap.n = max(heap.n, degree(v)+1)
	// grow b (and set things to NULL) if necessary
	if (heap->n > heap->max) {
		heap->b = realloc(heap->b, heap->n * sizeof(struct fib_node *));
		memset(heap->b+heap->max, 0, sizeof(struct fib_node *)*(heap->n - heap->max));
		heap->max = heap->n;
	}

	struct fib_node *r = heap->b[v->degree];
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
	start->degree++;
	end->parent = start;
	end->next = start->child;
	start->child = end;

	return;
}





// DEBUG STUFF


void print_node(struct fib_node *v)
{
	if (v) {
		printf("key:          %lf\n", v->key);
		printf("value:        %p\n", v->val);
		printf("phi:          %d\n", v->phi);
		printf("degree:       %d\n", v->degree);
		printf("parent:       %p\n", v->parent);
		printf("child:        %p\n", v->child);
		printf("next:         %p\n", v->next);
	}
}

void print_heap(struct fib_heap *heap)
{
	printf("max, n = %d, %d\n", heap->max, heap->n);
	printf("---------------\n");
	printf("Contents of b:\n");

	for (int i = 0; i<heap->n; ++i) {
		printf("+===============+\n");
		printf("| b[%d] = %p\n", i, heap->b[i]);
		print_node(heap->b[i]);
		printf("L_______________\n");
	}
}
