#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

#include "digraph.h"
#include "config.h"
#include "fib_heap.h"

/* return cost if a solution exists */
long successive_shortest_paths(struct graph *G, long *pi, char *f, int *ex);

/* dijkstra from x, stop at first valid y and return it */
int dijkstra(struct graph *G, long *pi, char *f, int *ex, int x, int *prev);

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

	/* Set initial f */
	char *f = calloc(G.m, sizeof(char));
	int *ex = calloc(G.n, sizeof(int));
	for (int e=0; e<G.m; ++e) {
		if (G.E[e].weight < 0) {
			f[e] = 1;
			ex[G.E[e].x]--;
			ex[G.E[e].y]++;
		}
	}

	/* generate initial zul. pot */
	long *pi = calloc(G.n, sizeof(long));

	long cost = successive_shortest_paths(&G, pi, f, ex);

	/* output */
	printf("%ld\n", cost);
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

long successive_shortest_paths(struct graph *G, long *pi, char *f, int *ex)
{
	int val = 0;
	int *prev = malloc(G->m * sizeof(int)); // tree for dijkstra, saves edges
	while (true) {
		/* find x, y */
		int x;
		for (x = 0; x<G->n/2; ++x)
			if (ex[x] >= 0) break;
		if (x == G->n/2) {
			for (x = G->n/2; x<G->n; ++x) {
				if (ex[x] >= 2) break;
			}
			if (x == G->n)
				break;
		}
		int y = dijkstra(G, pi, f, ex, x, prev);

		/* augment along x-y-path */
		for (int v=y; v!=x;) {
			int e = prev[v];
			if (f[e] == 0) {
				f[e] = 1;
				v = G->E[e].x;
			} else {
				f[e] = 0;
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

	long cost = 0;
	for (int e=0; e<G->m; ++e) {
		cost += f[e]*G->E[e].weight;
	}

	return cost;
}

int dijkstra(struct graph *G, long *pi, char *f, int *ex, int x, int *prev)
{
	char *visited = calloc(G->n, sizeof(char));
	char *main_visited = calloc(G->n, sizeof(char));
	struct fib_node **node = calloc(G->n, sizeof(struct fib_node *));
	struct fib_heap heap = {.n=0, .b=NULL};

	visited[x] = true;
	node[x] = fib_heap_insert(&heap, G->V+x, 0);

	struct fib_node *node_addr;
	while ((node_addr = fib_heap_extract_min(&heap)) != 0) {
		int v = ((struct vert*) node_addr->val)->id;
		main_visited[v] = true;
		/* return if valid y found */
		if ((v<G->n/2 && ex[v]<=-2) || (v>=G->n/2 && ex[v]<=0)) {
			/* set new pi */
			for (int i=0; i<G->n; ++i) {
				if (main_visited[i])
					pi[i] += node[i]->key;
				else
					pi[i] += node[v]->key;
			}

			free(visited);
			free(main_visited);
			if (heap.b) free(heap.b);
			for (int i=0; i<G->n; ++i) {
				if (node[i]) free(node[i]);
			}
			free(node);

			return v;
		}

		/* look at neighbours */
		for (int i=0; i<G->V[v].d_plus; ++i) {
			int e = G->V[v].to[i];
			if (f[e] == 0) {
				int w = G->E[e].y;
				if (! visited[w]) {
					visited[w] = true;
					node[w] = fib_heap_insert(&heap, G->V+w, node_addr->key + G->E[e].weight + pi[v] - pi[w]);
					prev[w] = e;
				} else if (node[w]->key > node_addr->key + G->E[e].weight + pi[v] - pi[w]) {
					fib_heap_decrease_key(&heap, node[w], node_addr->key + G->E[e].weight + pi[v] - pi[w]);
					prev[w] = e;
				}
			}
		}
		for (int i=0; i<G->V[v].d_minus; ++i) {
			int e = G->V[v].from[i];
			if (f[e] == 1) {
				int w = G->E[e].x;
				if (! visited[w]) {
					visited[w] = true;
					node[w] = fib_heap_insert(&heap, G->V+w, node_addr->key - G->E[e].weight + pi[v] - pi[w]);
					prev[w] = e;
				} else if (node[w]->key > node_addr->key - G->E[e].weight + pi[v] - pi[w]) {
					fib_heap_decrease_key(&heap, node[w], node_addr->key - G->E[e].weight + pi[v] - pi[w]);
					prev[w] = e;
				}
			}
		}
	}

	err(1, "Error: no possible solution.\n");
}
