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

static inline void relabel(int v, struct graph *G, long *f, struct list *L, struct list *A, int *phi, int *phi_max);
static inline void push(int v, int e, struct graph *G, int t, long *f, long *ex, struct list *L, struct list *A, int *phi, int *phi_max);

#ifdef DEBUG
void status(struct graph *G, int t, long *f, long *ex, int *phi, int phi_max, struct list *L, struct list *A);
#endif

/* Intelligently add an int to a list */
static void append_to_list(struct list *list, int x)
{
#ifdef DEBUG
	printf("append_to_list(list=%p, %d);\n", (void *) list, x);
#endif
	if (list->max_len == 0) {
#ifdef DEBUG
		if (list->array != NULL) printf("weird ...\n");
#endif
		list->array = malloc(sizeof(int));
		list->max_len = 1;
	} else if (list->len >= list->max_len) {
		list->array = realloc(list->array, sizeof(int)*2*list->len);
		list->max_len <<= 1;
	}
	list->array[list->len] = x;
	list->len++;
}

/* Free an entire array of lists, and the array itself */
static void free_lists(struct list *lists, int n)
{
	for (int i=0; i<n; ++i)
		if (lists[i].max_len > 0)
			free(lists[i].array);
	free(lists);
}

/* return active vertex with max. phi, last in list L[i] */
#ifdef DEBUG
static int get_max_active_node(struct list *L, int *max, long *ex, int s, int t)
#else
static int get_max_active_node(struct list *L, int *max, long *ex)
#endif
{
#ifdef DEBUG
	printf("get_max_active_node(L=%p, max=%d, ex=%p, s=%d, t=%d)\n", (void *) L, *max, (void *) ex, s, t);
//	*max = 2*G->n - 1;
#endif
	for (;*max >= 0; --(*max)) {
#ifdef DEBUG
		printf("Looking at L[%d] ..\n", *max);
#endif
		while (L[*max].len > 0) {
			int v = L[*max].array[L[*max].len - 1];
#ifdef DEBUG
			printf("    -> Looking at %d with ex[%d]=%ld\n", v, v, ex[v]);
			if (v == t || v == s) {
				printf("Oh dear. Found \"active node\" %d.\n", v);
				exit(1);
			}
#endif
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
static int get_allowed_edge(struct list *A, int v, int *phi, struct graph *G, long *f)
{
#ifdef DEBUG
	printf("get_allowed_edge(A=%p, v=%d, phi=%p, G, f);\n", (void *) A, v, (void *) phi);
#endif
	while (A[v].len > 0) {
		int e = A[v].array[A[v].len - 1];
		if ((f[e] < G->E[e].weight && phi[v] == phi[G->E[e].y] + 1) || (f[e] > 0 && phi[v] == phi[G->E[e].x] + 1))
			return e;
		else
			A[v].len--;
	}
#ifdef DEBUG
	printf("Aww, no allowed edges found.\n");
#endif
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
#ifdef DEBUG
	status(G, t, f, ex, phi, phi_max, L, A);
	while ((v = get_max_active_node(L, &phi_max, ex, s, t)) != -1) {
#else
	while ((v = get_max_active_node(L, &phi_max, ex)) != -1) {
#endif
		if ((e = get_allowed_edge(A, v, phi, G, f)) != -1)
			push(v, e, G, t, f, ex, L, A, phi, &phi_max);
		else
			relabel(v, G, f, L, A, phi, &phi_max);
#ifdef DEBUG
		printf("Done Loop!\n");
		status(G, t, f, ex, phi, phi_max, L, A);
#endif
	}

	free(phi);
	free(ex);
	free_lists(L, 2*G->n);
	free_lists(A, G->n);

	return f;
}

static inline void relabel(int v, struct graph *G, long *f, struct list *L, struct list *A, int *phi, int *phi_max)
{
#ifdef DEBUG
	printf("RELABEL(v=%d, G, f, L=%p, A=%p, phi=%p, phi_max=%d)!\n", v, (void *) L, (void *) A, (void *) phi, *phi_max);
	if (*phi_max != phi[v]) printf("Oh dear, relabeling v with bad phi(v)!\n");
	if (L[phi[v]].array[L[phi[v]].len - 1] != v) printf("Oh dear, relabeling bad v!\n");
#endif
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

#ifdef DEBUG
	printf("\tphi_max is now %d.\n", *phi_max);
	if (phi[v] == INT_MAX) {
		printf("Oh dear, that's not supposed to happen. I think.\n");
	}
#endif
}

static inline void push(int v, int e, struct graph *G, int t, long *f, long *ex, struct list *L, struct list *A, int *phi, int *phi_max)
{
#ifdef DEBUG
	printf("PUSH(v=%d, e=%d, G, t=%d, f=%p, ex=%p, L=%p, A=%p, phi=%p, phi_max=%d)!\n", v, e, t, (void *) f, (void *) ex, (void *) L, (void *) A, (void *) phi, *phi_max);
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
#ifdef DEBUG
		if (L[*phi_max].array[L[*phi_max].len - 1] != v) printf("Oh dear, pushing from bad v!\n");
#endif
		L[*phi_max].len--;	/* by choice of v, phi(v) = phi_max */
	}
	/* Have a look if e is still allowed
	 * If e is saturated, it's not allowed anymore
	 * If it isn't, it's still allowed
	 */
	if (delta == G->E[e].weight) {
#ifdef DEBUG
		if (A[v].array[A[v].len - 1] != e) printf("Oh dear, pushing bad e!\n");
#endif
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

#ifdef DEBUG
void status(struct graph *G, int t, long *f, long *ex, int *phi, int phi_max, struct list *L, struct list *A)
{
	printf("STATUS: checking ...\n");
	bool everything_fine = true;
	/* Sanity check */
	/* ex_f */
	for (int v=0; v<G->n; ++v) {
		long c_ex = 0;
		for (int i=0; i<G->V[v].d_minus; ++i)
			c_ex += f[G->V[v].from[i]];
		for (int i=0; i<G->V[v].d_plus; ++i)
			c_ex -= f[G->V[v].to[i]];
		if (c_ex != ex[v]) {
			printf("Warning: ex[%d] = %ld, but should be %ld!\n", v, ex[v], c_ex);
			everything_fine = false;
		}
	}
	/* phi_max */
	for (int v=0; v<G->n; ++v) {
		if (v!=t && phi[v] > phi_max && ex[v]>0) {
			printf("Warning: found active node %d with phi(%d) = %d > %d = phi_max\n", v, v, phi[v], phi_max);
			everything_fine = false;
		}
	}
	/* check if active nodes in L */
	for (int v=0; v<G->n; ++v) {
		if (v != t && ex[v] > 0) {
			bool in_L = false;
			for (size_t i=0; i<L[phi[v]].len; ++i) {
				if (L[phi[v]].array[i] == v) in_L = true;
			}
			if (!in_L) {
				printf("Warning: found active node %d with phi(%d) = %d, not in L[%d].\n", v, v, phi[v], phi[v]);
				everything_fine = false;
			}
		}
	}
	/* check if allowed edges in A */
	for (int v=0; v<G->n; ++v) {
		for (int i=0; i < G->V[v].d_plus; ++i) {
			int e = G->V[v].to[i];
			if (f[e] < G->E[e].weight && phi[v] == phi[G->E[e].y] + 1) {	/* e allowed edge, in G */
				bool in_A = false;
				for (size_t j=0; j<A[v].len; ++j) {
					if (A[v].array[j] == e) in_A = true;
				}
				if (!in_A) {
					everything_fine = false;
					printf("Warning: found allowed edge %d = (%d,%d) A[%d].\n", e, v, G->E[e].y, v);
				}
			}
		}
		for (int i=0; i < G->V[v].d_minus; ++i) {
			int e = G->V[v].from[i];
			if (f[e] > 0 && phi[v] == phi[G->E[e].x] + 1) {	/* e allowed edge, in G_back */
				bool in_A = false;
				for (size_t j=0; j<A[v].len; ++j) {
					if (A[v].array[j] == e) in_A = true;
				}
				if (!in_A) {
					printf("Warning: found allowed edge %d = (%d,%d) A[%d].\n", e, v, G->E[e].x, v);
					everything_fine = false;
				}
			}
		}
	
	}
	if (everything_fine) printf("STATUS: Your variables are in order.\n");
	else printf("STATUS: You seem to have a problem.\n");

	if (G->n > 1000) return;
	
	/* Print out variables */
	printf("+++++++++++++++++++\n");
	printf("| f = {");
	for (int i=0; i<G->m; ++i)	printf("%ld, ", f[i]);
	printf("}\n");
	printf("| ex = {");
	for (int i=0; i<G->n; ++i)	printf("%ld, ", ex[i]);
	printf("}\n");
	printf("| phi = {");
	for (int i=0; i<G->n; ++i)	printf("%d, ", phi[i]);
	printf("}\n");
	printf("| phi_max = %d\n", phi_max);
	
	printf("| L:\n");
	for (int i=0; i<2*G->n; ++i) {
		printf("| \tL[%d] = {", i);
		for (size_t j=0; j<L[i].len; ++j) {
			printf("%d, ", L[i].array[j]);
		}
		printf("}\n");
	}
	printf("| A:\n");
	for (int i=0; i<G->n; ++i) {
		printf("| \tA[%d] = {", i);
		for (size_t j=0; j<A[i].len; ++j) {
			printf("%d, ", A[i].array[j]);
		}
		printf("}\n");
	}
	printf("+++++++++++++++++++\n");
}
#endif
