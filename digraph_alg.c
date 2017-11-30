#define _POSIX_C_SOURCE 201112L

#include "digraph.h"
#include "digraph_alg.h"

struct list {
	int *array;
	size_t len, max_len;
};

// return active vertex with max. phi, last in list L[i]
static int get_max_active_node(int **L, int max)
{
	for (;max >= 0; --max) {
		if (L[max].len > 0) return L[max].array[L[max].len - 1];
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
	// set lists L[i] of active vertices with phi(v) = i
	// At first: active verts are exactly s's neighbours
	int **L = calloc(2*G->n - 1, sizeof(struct list));
	L[0].array = malloc(G-V[s].d_plus * sizeof(int));
	L[0].len = L[0].max_len = G->V[s].d_plus;
	for (int i=0; i<G->V[s].d_plus; ++i)
		L[0][i] = G->E[G->V[s].to[i]].y;
	// set lists A[v] of allowed edges starting in v
	int **A = calloc(G->n, sizeof(struct list));

	int v;
	while (v = get_max_active_node(L, phi_max) != -1) {
		// find allowed edge
		if (A[v].len > 0) {	// found allowed edge; push(e)
			int e = A[v].array[A[v].len - 1];
			long val = ex[v]>G->E[e].weight ? G->E[e].weight : ex[v] ;
			if (v == G->E[e].x)	// edge in G
				f[e] += val;
			else			// edge in G_back, reduce along original edge
				f[e] -= val;
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
