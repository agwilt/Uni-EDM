#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "digraph.h"
#include "digraph_alg.h"

struct list {
	int *array;
	size_t len, max_len;
};

static inline void relabel(int v, struct graph *G, int s, int t, long *f, long *ex, struct list *L, struct list *A, int *phi, int *phi_max);
static inline void push(int v, int e, struct graph *G, int s, int t, long *f, long *ex, struct list *L, struct list *A, int *phi, int *phi_max);

/* Intelligently add an int to a list */
static void append_to_list(struct list *list, int x)
{
	if (list->max_len == 0) {
		list->array = malloc(sizeof(int));
		list->max_len = 1;
	} else if (list->len >= list->max_len) {
		list->array = realloc(list->array, sizeof(int)*2*list->len);
		list->max_len >>= 1;
	}
	list->array[list->len] = x;
	list->len++;
}

/* Free an entire array of lists, and the array itself */
static void free_lists(struct list *lists, int n)
{
	for (int i=0; i<n; ++i)
		if (lists[i].len > 0)
			free(lists[i].array);
	free(lists);
}

/* return active vertex with max. phi, last in list L[i] */
static int get_max_active_node(struct list *L, int *max, long *ex)
{
#ifdef DEBUG
	printf("get_max_active_node(L, %d, ex)\n", *max);
#endif
	for (;*max >= 0; --(*max)) {
		while (L[*max].len > 0) {
			int v = L[*max].array[L[*max].len - 1];
			printf("    -> Looking at %d with ex[%d]=%ld\n", v, v, ex[v]);
			/* WARNING: vertices in L aren't always active, so check if they really are */
			if (ex[v] > 0)
				return v;
			else
				L[*max].len--;
		}
	}
#ifdef DEBUG
	printf("Aww, no active nodes found.\n");
#endif
	return -1;
}

// TODO: possibly speed up again with is_allowed
/* return allowed edge starting (in G_f) in v, or -1 */
static int get_allowed_edge(struct list *A, int v, int *phi, struct graph *G, long *f, long *ex)
{
	while (A[v].len > 0) {
		int e = A[v].array[A[v].len - 1];
		if ((f[e] < G->E[e].weight && phi[v] == phi[G->E[e].y] + 1) || (f[e] > 0 && phi[G->E[e].y] == phi[v] + 1))
			return e;
		else
			A[v].len--;
	}
	return -1;
}

long *digraph_max_flow(struct graph *G, int s, int t)
{
	/*
	 * Overview of variables:
	 * 	f: flow, saved for every edge
	 * 	phi: distance marker
	 * 	phi_max: >= max{phi(v) | v active}
	 * 	ex: inflow - outflow. v is active, if ex(v)>0 and v!=t
	 * 	L[i], 0<=i<2n-1: list of possibly active vertices with phi(v)=i
	 * 	A[v], 0<=v<n: saves list of (possibly) allowed edges (in G_f) starting in v.
	 */

	// set initial preflow
	long *f = calloc(G->m, sizeof(long));
	for (int i=0; i<G->V[s].d_plus; ++i) {
		f[G->V[s].to[i]] = G->E[G->V[s].to[i]].weight;
	}
	// set phi
	int phi_max = 0;
	int *phi = calloc(G->n, sizeof(int));
	phi[s] = G->n;
	// set Überschuss
	long *ex = calloc(G->n, sizeof(long));
	for (int i=0; i<G->V[s].d_plus; ++i) {
		ex[G->E[G->V[s].to[i]].y] += f[G->V[s].to[i]];
		ex[s] -= f[G->V[s].to[i]];
	}
	// set lists L[i] of possible active vertices with phi(v) = i
	// At first: active vertices are exactly s's neighbours
	struct list *L = calloc(2*G->n, sizeof(struct list));
	if (G->V[s].d_plus > 0) {
		L[0].array = malloc(G->V[s].d_plus * sizeof(int));
		L[0].len = L[0].max_len = G->V[s].d_plus;
		for (int i=0; i<G->V[s].d_plus; ++i) {
			L[0].array[i] = G->E[G->V[s].to[i]].y;
		}
	}
	// set lists A[v] of possibly allowed edges starting in v
	struct list *A = calloc(G->n, sizeof(struct list));

	/*
	 * Initialisation now completed, start algorithm
	 */

	int v, e;
	while ((v = get_max_active_node(L, &phi_max, ex)) != -1) {
		if ((e = get_allowed_edge(A, v, phi, G, f, ex)) != -1)
			push(v, e, G, s, t, f, ex, L, A, phi, &phi_max);
		else
			relabel(v, G, s, t, f, ex, L, A, phi, &phi_max);
	}

	free(phi);
	free(ex);
	free_lists(L, 2*G->n);
	free_lists(A, G->n);

	return f;
}

static inline void relabel(int v, struct graph *G, int s, int t, long *f, long *ex, struct list *L, struct list *A, int *phi, int *phi_max)
{
#ifdef DEBUG
	printf("RELABEL(%d)!\n", v);
#endif
	phi[v] = INT_MAX;
	int e;
	/* find minimum phi among neighbours in G_f */
	/* first check d_plus(v) */
	for (int i=0; i<G->V[v].d_plus; ++i) {
		e = G->V[v].to[i];
		if (f[e] < G->E[e].weight && phi[G->E[e].y]+1 < phi[v])
			phi[v] = phi[G->E[e].y] + 1;
	}
	/* then, d_minus(v) */
	for (int i=0; i<G->V[v].d_minus; ++i) {
		e = G->V[v].from[i];
		if (f[e] > 0 && phi[G->E[e].x]+1 < phi[v])
			phi[v] = phi[G->E[e].x] + 1;
	}
	if (phi[v] > *phi_max) *phi_max = phi[v];
	/* Now check how many edges are newly allowed */
	for (int i=0; i<G->V[v].d_plus; ++i) {
		e = G->V[v].to[i];
		if (f[e] < G->E[e].weight && phi[G->E[e].y]+1 == phi[v])
			append_to_list(A+v, e);	/* Just append, since there weren't any allowed edges beforehand. */
	}
	/* then, d_minus(v) */
	for (int i=0; i<G->V[v].d_minus; ++i) {
		e = G->V[v].from[i];
		if (f[e] > 0 && phi[G->E[e].x]+1 == phi[v])
			append_to_list(A+v, e);	/* Just append, since there weren't any allowed edges beforehand. */
	}
	
	if (phi[v] == INT_MAX) {
		printf("Oh dear, that's not supposed to happen. I think.\n");
	}
}

static inline void push(int v, int e, struct graph *G, int s, int t, long *f, long *ex, struct list *L, struct list *A, int *phi, int *phi_max)
{
#ifdef DEBUG
	printf("PUSH(%d)!\n", e);
#endif
	/* find delta to augment by */
	long delta;
	/* e's capacity is "to small", v still active */
	if (ex[v] > G->E[e].weight) {
		delta = G->E[e].weight;
		ex[v] -= delta;		/* still >0 */
	/* ex[v] is now 0, v deactivated */
	} else {
		delta = ex[v];
		ex[v] = 0;
		L[*phi_max].len--;	/* by choice of v, phi(v) = phi_max */
	}
	/* Have a look if e is still allowed
	 * If e is saturated, it's not allowed anymore
	 * If it isn't, it's still allowed
	 */
	if (delta == G->E[e].weight) {
		A[v].len--;
	}

	int w;	/* the other vertex */
	if (v == G->E[e].x) {	/* edge in G */
		f[e] += delta;
		w = G->E[e].y;
	} else {		/* edge in G_back, reduce along original edge */
		f[e] -= delta;
		w = G->E[e].x;
		/* Possible update phi_max. If e=(v,w), no need since phi(v)>phi(w). */
		if (phi[w] > *phi_max) *phi_max = phi[w];
	}

	/* w is now definitely active. Add to L if newly active */
	if (ex[w]==0 && w != t)
		append_to_list(L+phi[w], w);
	ex[w] += delta;
}

long digraph_flow_val(struct graph *G, int s, long *f)
{
	long val = 0;
	for (int i=0; i<G->V[s].d_plus; ++i) {
		val += f[G->V[s].to[i]];
	}
	return val;
}

void digraph_flow_print(long *f, int m)
{
	for (int i=0; i<m; ++i) {
		printf("%d %ld\n", i, f[i]);
	}
}
