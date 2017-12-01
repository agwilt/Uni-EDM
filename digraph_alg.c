#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <stdlib.h>

#include "digraph.h"
#include "digraph_alg.h"

struct list {
	int *array;
	size_t len, max_len;
};

/* Intelligently add an int to a list */
static void append_to_list(struct list *list, int x)
{
	if (list->len >= list->max_len) {
		list->array = realloc(list->array, sizeof(int)*(2*list->len+1));
		list->max_len >>= 1;
	}
	list->array[list->len++] = x;
}

/* Free an entire array of lists, and the array itsself */
static void free_lists(struct list *lists, size_t n)
{
	for (int i=0; i<n; ++i)
		free(lists[i].array);
	free(lists);
}

/* return active vertex with max. phi, last in list L[i] */
static int get_max_active_node(struct list *L, bool *is_active, int *max)
{
	for (;*max >= 0; --(*max)) {
		while (L[*max].len > 0) {
			int v = L[*max].array[L[*max].len - 1];
			/* WARNING: vertices in L aren't always active, so check if they really are */
			if (is_active[v])
				return v;
			else
				L[*max].len--;
		}
	}
	return -1;
}

/* return allowed edge starting (in G_f) in v, or -1 */
static int get_allowed_edge(struct list *A_v, bool *is_allowed)
{
	while (A_v->len > 0) {
		int e = A_v->array[A_v->len - 1];
		if (is_allowed[e])
			return e;
		else
			A_v->len--;
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
	 * 	is_active: saves if a vertex is active, to prevent O(d_plus+d_minus) checking time
	 * 	L[i], 0<=i<2n-1: list of possibly active vertices with phi(v)=i
	 * 	is_allowed: saves if an edge is allowed: phi(v) = phi(w) + 1
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
		ex[G->V[s].to[i]] += f[G->V[s].to[i]];
		ex[s] -= f[G->V[s].to[i]];
	}
	// set lists L[i] of possible active vertices with phi(v) = i
	// At first: active verts are exactly s's neighbours
	bool *is_active = calloc(G->n, sizeof(bool));
	struct list *L = calloc(2*G->n - 1, sizeof(struct list));
	L[0].array = malloc(G->V[s].d_plus * sizeof(int));
	L[0].len = L[0].max_len = G->V[s].d_plus;
	for (int i=0; i<G->V[s].d_plus; ++i) {
		L[0].array[i] = G->E[G->V[s].to[i]].y;
		is_active[G->E[G->V[s].to[i]].y] = true;
	}
	// set lists A[v] of possibly allowed edges starting in v
	bool *is_allowed = calloc(G->m, sizeof(bool));
	struct list *A = calloc(G->n, sizeof(struct list));

	/*
	 * Initialisation now completed, start algorithm
	 */

	int v, e;
	while ((v = get_max_active_node(L, is_active, &phi_max)) != -1) {
		if ((e = get_allowed_edge(A+v, is_allowed)) != -1)
			push(v, e, G, f, ex, L, is_active, A, is_allowed, phi, phi_max);
		else
			relabel(v, G, f, ex, L, is_active, A, is_allowed, phi, phi_max);
	}

	free(phi);
	free(ex);
	free(is_active);
	free(is_allowed);
	free_lists(L, 2*G->n);
	free_lists(A, G->n);

	return f;
}

inline void relabel(int v, struct graph *G, long *f, long *ex, struct list *L, bool *is_active, struct list *A, bool *is_allowed, int *phi, int phi_max)
{
	phi[v] = 1+phi[G->E[G->V[v].to[0]].y];
	/* find minimum phi among neighbours */
	for (int i=0; i<G->V[v].d_plus; ++i) {
		if (phi[G->V[v].to[i]] < phi[i])
			phi[i] = phi[G->V[v].to[i]];
	}
}

inline void push(int v, int e, struct graph *G, long *f, long *ex, struct list *L, bool *is_active, struct list *A, bool *is_allowed, int *phi, int phi_max)
{
	/* find delta to augment by */
	long delta;
	/* e's capacity is "to small", e saturated, v still active */
	if (ev[v] > G->E[e].weight) {
		delta = G->E[e].weight;
		ex[v] -= delta;		/* still >0 */
		/* ex[v] is now 0, e possibly not saturated, v deactivated */
	} else {
		delta = ex[v];
		is_active[v] = false;
		ex[v] = 0;
		L[phi_max].len--;	/* by choice of v, phi(v) = phi_max */
	}
	/* Have a look if e is still allowed
	 * If e is saturated, it's not allowed anymore
	 * If it isn't, it's still allowed
	 */
	if (delta == G->E[e].weight) {
		is_allowed[e] = false;
		A[v].len--;
	}

	int w;	/* the other vertex */
	if (v == G->E[e].x) {	/* edge in G */
		f[e] += delta;
		w = G->E[e].y;
	} else {		/* edge in G_back, reduce along original edge */
		f[e] -= delta;
		w = G->E[e].x;
	}

	/* w is now definitely active. Add to L if newly active */
	if (!is_active[w] && w != t) {
		is_active[w] = true;
		append_to_list(L[phi[w]], w);
	}
}
