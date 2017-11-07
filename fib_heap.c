#define _POSIX_C_SOURCE 201112L

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "fib_heap.h"
#include "config.h"

static void fib_heap_plant(struct fib_heap *heap, struct fib_node *v);
static void fib_node_add_neighbour(struct fib_node *start, struct fib_node *end);
static void fib_free_nodes(struct fib_node *v);
static void fib_deal_with_parents(struct fib_heap *heap, struct fib_node *v, struct fib_node *r);



extern struct fib_node *fib_heap_insert(struct fib_heap *heap, void *val, double key)
{
	struct fib_node *node = malloc(sizeof(struct fib_node));
	node->key = key;
	node->val = val;
	node->phi = 0;
	node->degree = 0;
	node->child = NULL;
	node->parent = NULL;
	node->next = NULL;
	node->prev = NULL;

	fib_heap_plant(heap, node);

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

	if (r == NULL) return r;
	// now deal with kids

	// remove from b FIRST, to avoid interference with kids
	heap->b[r->degree] = NULL;

	struct fib_node *next;
	for (struct fib_node *child = r->child; child != NULL; child=next) {
		next = child->next;
		if (next) next->prev = NULL;
		child->parent = NULL;
		child->next = NULL;
		child->prev = NULL;
		fib_heap_plant(heap, child);
	}

	return r;
}

// with this recursion, the entire path is detached before any planting happens, which hopefully keeps the heap clean ... 
static void fib_deal_with_parents(struct fib_heap *heap, struct fib_node *v, struct fib_node *r)
{
	if (v == r) {
		v->phi = 1 - v->phi;
		if (v->parent == NULL) {
			heap->b[v->degree+1] = NULL; // v doesn't belong here anymore
			fib_heap_plant(heap, v);
		}
		return;
	}
	struct fib_node *parent = v->parent;
	parent->degree--;
	if (parent->child == v) parent->child = v->next;
	else v->prev->next = v->next;
	if (v->next) v->next->prev = v->prev;
	v->parent = NULL;
	v->prev = NULL;
	v->next = NULL;

	fib_deal_with_parents(heap, parent, r);

	fib_heap_plant(heap, v);
}

extern void fib_heap_decrease_key(struct fib_heap *heap, struct fib_node *v, double key)
{
	if (v->key <= key) {
		fprintf(stderr, "Error: silly decrease key %lf → %lf\n", v->key, key);
		exit(1);
	}

	v->key = key;
	// in case v is a root or no reorganisation needed: just exit quickly
	if (v->parent == NULL)
		return;


	// first, find longest path and switch (phi)s
	struct fib_node *r = v->parent;
	while (r->parent != NULL && r->phi == 1) {
		r->phi = 1-v->phi;
		r = r->parent;
	}
	// r!=v is now root of path where all inner vertices z have phi(z) = 1
	// either phi(r) == 0 or r is root

	fib_deal_with_parents(heap, v, r);

	return;
}

// private functions

static void fib_heap_plant(struct fib_heap *heap, struct fib_node *v)
{
	// detach v, just in case
	v->parent = NULL;
	v->next = NULL;
	v->prev = NULL;

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
	if (r != NULL && r != v && r->parent == NULL) {
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
	end->prev = NULL;
	if (start->child)
		start->child->prev = end;
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
		fib_print_tabs(r); printf("| prev:         %-14p |\n", (void *) v->prev);
		struct fib_node *child = v->child;
		while (child != NULL) {
			fib_print_nodes(child, r+1);
			child = child->next;
		}
	}
	fib_print_tabs(r); printf("+------------------------------+\n");
}

extern void fib_print_heap(struct fib_heap *heap)
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

static void fib_free_nodes(struct fib_node *v)
{
	if (v) {
		struct fib_node *next;
		v = v->child;
		while (v != NULL) {
			next = v->next;
			fib_free_nodes(v);
			free(v);
			v = next;
		}
	}
}

extern void fib_heap_free(struct fib_heap *heap)
{
	if (heap->b) {
		for (int i = 0; i<heap->n; ++i) {
			if (heap->b[i]) {
				fib_free_nodes(heap->b[i]);
				free(heap->b[i]);
			}
		}
		free(heap->b);
	}
}
