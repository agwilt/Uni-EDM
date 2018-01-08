#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "digraph.h"
#include "config.h"
#include "fib_heap.h"

/* return cost if a solution exists */
int successive_shortest_paths(struct graph *G, int *pi, char *f);

/* dijkstra from x, stop at first valid y and return it */
int dijkstra(struct graph *G, int *pi, char *f, char *ex, int x, int *prev);

int main(int argc, char *argv[])
{
	if (argc <= 1)
		err(1, "Usage: %s filename\n", argv[0]);

	struct graph G = graph_from_file(argv[1]);

	/* check if graph valid */
	/*
	for (int v=0; v<G.n/2; ++v) {
		if (G.V[v].d_plus != G.n/2) err(1, "Error: Bad graph: d_plus(%d)=%d\n", v, G.V[v].d_plus);
	}
	for (int v=G.n/2; v<G.n; ++v) {
		if (G.V[v].d_minus != G.n/2) err(1, "Error: Bad graph: d_minus(%d)=%d\n", v, G.V[v].d_minus);
	}
	*/

	/* generate initial zul. pot */
	int *pi = calloc(G.n, sizeof(int));
	for (int e=0; e<G.m; ++e)
		if (G.E[e].weight < pi[G.E[e].y])
			pi[G.E[e].y] = G.E[e].weight;
	char *f = calloc(G.m, sizeof(char));

#ifdef DEBUG
	if (G.n < 100) {
		graph_print(&G);
	} else {
		printf("n = %d, m = %d\n", G.n, G.m);
	}
#endif

	int cost = successive_shortest_paths(&G, pi, f);

	/* output */
	printf("%d\n", cost);
	for (int e=0; e<G.m; ++e) {
		if (f[e]) {
			if (G.E[e].x < G.n/2)
				printf("%d %d\n", G.E[e].x, G.E[e].y);
			else
				printf("%d %d\n", G.E[e].y, G.E[e].x);
		}
	}

	graph_free(&G);
	free(pi);
	free(f);

	return 0;
}

int successive_shortest_paths(struct graph *G, int *pi, char *f)
{
#ifdef DEBUG
	printf("successive_shortest_paths(G, pi=%p, f=%p)\n", (void*) pi, (void*) f);
#endif
	int val = 0;
	int *prev = malloc(G->m * sizeof(int)); // tree for dijkstra, saves edges
	char *ex = calloc(G->n, sizeof(char));
	while (val < G->n/2) {
		/* find x, y */
		int x;
		for (x = 0; x<G->n/2; ++x) {
			if (ex[x] == 0) break;
		}
		if (x == G->n/2) {
			err(1, "Error: Graph is broken.\n");
		}
		int y = dijkstra(G, pi, f, ex, x, prev);

#ifdef DEBUG
		printf("Will now augment from %d to %d\n", x, y);
#endif
		/* augment along x-y-path */
		for (int v=y; v!=x;) {
			int e = prev[v];
			if (f[e] == 0) {
				printf("v = %d, e = (%d,%d), f[e] = %d\n", v, G->E[e].x, G->E[e].y, f[e]);
				f[e] = 1;
				if (v == G->E[e].x) printf("Wah!\n");
				v = G->E[e].x;
			} else {
				printf("v = %d, e = (%d,%d), f[e] = %d\n", v, G->E[e].x, G->E[e].y, f[e]);
				f[e] = 0;
				if (v == G->E[e].y) printf("Wah!\n");
				v = G->E[e].y;
			}
		}
		/* update ex[] */
		ex[x]--;
		ex[y]++;
		val++;
	}

	free(prev);
	free(ex);

	int cost = 0;
	for (int e=0; e<G->m; ++e) {
		cost += f[e]*G->E[e].weight;
	}

	return cost;
}

int dijkstra(struct graph *G, int *pi, char *f, char *ex, int x, int *prev)
{
#ifdef DEBUG
	printf("dijkstra(G, pi=%p, f=%p, ex=%p, x=%d, prev=%p)\n", (void*) pi, (void*) f, (void*) ex, x, (void*) prev);
#endif
	char *visited = calloc(G->n, sizeof(char));
	struct fib_node **node = calloc(G->n, sizeof(struct fib_node *));
	struct fib_heap heap = {.n=0, .b=NULL};

	int *old_pi = malloc(G->n * sizeof(int));
	for (int i=0; i<G->n; ++i)
		old_pi[i] = pi[i];

	visited[x] = true;
	node[x] = fib_heap_insert(&heap, G->V+x, 0);

	struct fib_node *node_addr;
	int return_v = -1;
	while ((node_addr = fib_heap_extract_min(&heap)) != 0) {
		int v = ((struct vert*) node_addr->val)->id;
#ifdef DEBUG
		printf("Dijkstra Iteration, starting from v=%d!\n", v);
#endif
		/* return if valid y found */
		if (return_v == -1 && ex[v] == 0 && v >= G->n/2) {
			return_v = v;
		}
		/* look at neighbours */
#ifdef DEBUG
		printf("\tnow scanning neighbours ...\n");
#endif
		for (int i=0; i<G->V[v].d_plus; ++i) {
			int e = G->V[v].to[i];
#ifdef DEBUG
			printf("\tLooking at e=%d\n", e);
#endif
			if (f[e] == 0) {
				int w = G->E[e].y;
				if (G->E[e].weight + old_pi[v] - old_pi[w] < 0) printf("ERROR: Invalied zul. Pot.\n");
				if (! visited[w]) {
#ifdef DEBUG
					printf("\t\tfound new neighbour %d from %d in G\n", w, v);
#endif
					visited[w] = true;
					node[w] = fib_heap_insert(&heap, G->V+w, node[v]->key + G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] = old_pi[w] + node[w]->key;
					prev[w] = e;
				} else if (node[w]->key > node[v]->key + G->E[e].weight + old_pi[v] - old_pi[w]) {
#ifdef DEBUG
					printf("\t\tupdated edge (%d,%d) in G\n", v, w);
#endif
					fib_heap_decrease_key(&heap, node[w], node[v]->key + G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] = old_pi[w] + node[w]->key;
					prev[w] = e;
				}
			}
		}
		for (int i=0; i<G->V[v].d_minus; ++i) {
			int e = G->V[v].from[i];
#ifdef DEBUG
			printf("\tLooking at e=%d\n", e);
#endif
			if (f[e] == 1) {
				int w = G->E[e].x;
				if (G->E[e].weight + old_pi[v] - old_pi[w] < 0) printf("ERROR: Invalied zul. Pot.\n");
				if (! visited[w]) {
#ifdef DEBUG
					printf("\t\tfound new neighbour %d from %d in G_f\n", w, v);
#endif
					visited[w] = true;
					node[w] = fib_heap_insert(&heap, G->V+w, node[v]->key - G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] = old_pi[w] + node[w]->key;
					prev[w] = e;
				} else if (node[w]->key > node[v]->key - G->E[e].weight + old_pi[v] - old_pi[w]) {
#ifdef DEBUG
					printf("\t\tupdated edge (%d,%d) in G_f\n", v, w);
#endif
					fib_heap_decrease_key(&heap, node[w], node[v]->key - G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] = old_pi[w] + node[w]->key;
					prev[w] = e;
				}
			}
		}
	}

	free(visited);
	free(old_pi);
	if (heap.b) free(heap.b);
	for (int i=0; i<G->n; ++i) {
		if (node[i]) free(node[i]);
	}
	free(node);

	if (return_v == -1)
		err(1, "Error: no possible solution.\n");
	else
		return return_v;
}
