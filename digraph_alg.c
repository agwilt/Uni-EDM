#define _POSIX_C_SOURCE 201112L

#include "digraph.h"
#include "digraph_alg.h"

struct list {
	int *array;
	size_t len, max_len;
};

static void append_to_list(struct list *list, int x)
{
	if (list->len >= list->max_len) {
		list->array = realloc(list->array, 2*list->len);
		list->max_len >>= 1;
	}
	list->array[list->len++] = x;
}

// return active vertex with max. phi, last in list L[i]
static int get_max_active_node(struct list *L, bool *is_active, int *max)
{
	for (;*max >= 0; --(*max)) {
		while (L[*max].len > 0) {
			int v = L[*max].array[L[*max].len - 1];
			// WARNING: vertices in L aren't always active, so check if they really are
			if (is_active[v])
				return v;
			else
				L[*max].len--;
		}
	}
	return -1;
}

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
	// set initial preflow
	long *f = calloc(G->m, sizeof(long));
	for (int i=0; i<G->V[s].d_plus; ++i) {
		f[G->V[s].to[i]] = G->E[G->V[s].to[i]].weight;
	}
	// set phi
	int phi_max = 0;
	int *phi = calloc(G->n, sizeof(int));
	phi[s] = n;
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
	L[0].array = malloc(G-V[s].d_plus * sizeof(int));
	L[0].len = L[0].max_len = G->V[s].d_plus;
	for (int i=0; i<G->V[s].d_plus; ++i) {
		L[0][i] = G->E[G->V[s].to[i]].y;
		is_active[G->E[G->V[s].to[i]].y] = true;
	}
	// set lists A[v] of possibly allowed edges starting in v
	bool *is_allowed = calloc(G->m, sizeof(bool));
	struct list *A = calloc(G->n, sizeof(struct list));

	int v, e;
	while (v = get_max_active_node(L, &phi_max) != -1) {
		// find allowed edge
		if (e = get_allowed_edge(A+v, is_allowed) != -1) {	// found allowed edge; push(e)
			long val = ex[v]>G->E[e].weight ? G->E[e].weight : ex[v] ;
			if (v == G->E[e].x) {	// edge in G
				f[e] += val;
				if (val == G->E[e].weight) {	// v not active anymore
					is_active[v] = false;
					L[phi_max].num--;
				}
			} else {		// edge in G_back, reduce along original edge
				f[e] -= val;
			}



		} else {		// no alowed edges; relabel(v)
			phi[v] = 1+phi[G->V[v].to[0].y];
			// find minimum phi among neighbours
			for (int i=0; i<G->V[v].d_plus; ++i) {
				if (phi[G->V[v].to[i]] < phi[i])
					phi[i] = phi[G->V[v].to[i]];
			}
		}


	}
}
