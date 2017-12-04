#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "digraph.h"
#include "push-relabel.h"
#include "int_list.h"

static inline void relabel(int v, struct graph *G, long *f, struct list *L, struct list *A, int *phi, int *phi_max);
static inline void push(int v, int e, struct graph *G, int t, long *f, long *ex, struct list *L, struct list *A, int *phi, int *phi_max);

static int get_max_active_node(struct list *L, int *max, long *ex);
static int get_allowed_edge(struct list *A, int v, int *phi, struct graph *G, long *f);

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
		L[0].max_len = G->V[s].d_plus;
		for (int i=0; i<G->V[s].d_plus; ++i) {
			if (G->E[G->V[s].to[i]].y != t) {
				append_to_list(L, G->E[G->V[s].to[i]].y);
			}
		}
	}
	// set lists A[v] of possibly allowed edges starting in v
	struct list *A = calloc(G->n, sizeof(struct list));

	/*
	 * Initialisation now completed, start algorithm
	 */

	int v, e;
	while ((v = get_max_active_node(L, &phi_max, ex)) != -1) {
		if ((e = get_allowed_edge(A, v, phi, G, f)) != -1)
			push(v, e, G, t, f, ex, L, A, phi, &phi_max);
		else
			relabel(v, G, f, L, A, phi, &phi_max);
	}

	free(phi);
	free(ex);
	free_lists(L, 2*G->n);
	free_lists(A, G->n);

	return f;
}

static inline void relabel(int v, struct graph *G, long *f, struct list *L, struct list *A, int *phi, int *phi_max)
{
	/* First, remove v from L since phi is definitely going to increase */
	/* By choice of v, v is last in L[phi[v]] for the old phi */
	L[phi[v]].len--;

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

	/* Possibly update phi_max, add v to new different L */
	if (phi[v] > *phi_max)	*phi_max = phi[v];
	append_to_list(L+phi[v], v);

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
}

static inline void push(int v, int e, struct graph *G, int t, long *f, long *ex, struct list *L, struct list *A, int *phi, int *phi_max)
{
	/* find delta to augment by */
	long delta;

	/* Set ex */
	if (ex[v] > G->E[e].weight) {
		delta = G->E[e].weight;
		ex[v] -= delta;		/* still >0 */
	} else {	/* v is deactivated */
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

	/* Set flow */
	int w;	/* the other vertex */
	if (v == G->E[e].x) {	/* edge in G */
		f[e] += delta;
		w = G->E[e].y;
	} else {		/* edge in G_back, reduce along original edge */
		f[e] -= delta;
		w = G->E[e].x;
	}
	if (phi[w] > *phi_max) *phi_max = phi[w];

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
		if (f[i] > 0)
			printf("%d %ld\n", i, f[i]);
	}
}

/* return active vertex with max. phi, last in list L[i] */
static int get_max_active_node(struct list *L, int *max, long *ex)
{
	for (;*max >= 0; --(*max)) {
		while (L[*max].len > 0) {
			int v = L[*max].array[L[*max].len - 1];
			/* WARNING: vertices in L aren't always active, so check if they really are */
			if (ex[v] > 0)
				return v;
			else
				L[*max].len--;
		}
	}
	return -1;
}

// TODO: possibly speed up again with is_allowed
/* return allowed edge starting (in G_f) in v, or -1 */
static int get_allowed_edge(struct list *A, int v, int *phi, struct graph *G, long *f)
{
	while (A[v].len > 0) {
		int e = A[v].array[A[v].len - 1];
		if ((f[e] < G->E[e].weight && phi[v] == phi[G->E[e].y] + 1) || (f[e] > 0 && phi[v] == phi[G->E[e].x] + 1))
			return e;
		else
			A[v].len--;
	}
	return -1;
}
