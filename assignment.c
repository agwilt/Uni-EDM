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
	for (int v=0; v<G.n; ++v) {
		if (G.V[v].d_plus != G.n/2) err(1, "Bad graph\n");
	}

	// generate initial zul. pot
	int *pi = calloc(G.n, sizeof(int));
	for (int e=0; e<G.m; ++e)
		if (G.E[e].weight < pi[G.E[e].y])
			pi[G.E[e].y] = G.E[e].weight;
	char *f = calloc(G.m, sizeof(char));

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
}

int successive_shortest_paths(struct graph *G, int *pi, char *f)
{
	int val = 0;
	int *prev = malloc(G->m * sizeof(int)); // tree for dijkstra, saves edges
	char *ex = calloc(G->n, sizeof(char));
	while (val < G->n/2) {
		/* find x, y */
		int x;
		for (x = 0; x<G->n/2; ++x) {
			if (ex[x] == -1) break;
		}
		int y = dijkstra(G, pi, f, ex, x, prev);
		/* augment along x-y-path */
		for (int v=y; v!=x;) {
			int e = prev[v];
			if (f[e] == 0) {
				f[e] = 1;
				v = G->E[e].x;
			}
			else {
				f[e] = 0;
				v = G->E[e].y;
			}
		}
		/* update ex[] */
		ex[x]--;
		ex[y]++;
	}

	free(prev);
	free(ex);

	int cost = 0;
	for (int e=0; e<G->m; ++e) {
		cost += f[e];
	}

	return cost;
}

int dijkstra(struct graph *G, int *pi, char *f, char *ex, int x, int *prev)
{
	bool *visited = calloc(G->n, sizeof(bool));
	struct fib_node **node = malloc(G->n * sizeof(struct fib_node *));
	struct fib_heap heap = {.n=0, .b=NULL};

	int *old_pi = malloc(G->n * sizeof(int));
	for (int i=0; i<G->n; ++i)
		old_pi[i] = pi[i];

	visited[x] = true;
	node[x] = fib_heap_insert(&heap, (void*)(long) x, 0);

	int v;
	while ((v = (long) fib_heap_extract_min(&heap)) != 0) {
		/* return if valid y found */
		if (ex[v] == 0 && v >= G->n/2) {
			free(visited);
			free(node);
			free(old_pi);
			fib_heap_free(&heap);
			return v;
		}
		visited[v] = true;
		/* look at neighbours */
		for (int i=0; i<G->V[v].d_plus; ++i) {
			int e = G->V[v].to[i];
			if (f[e] == 0) {
				int w = G->E[e].y;
				if (! visited[w]) {
					visited[w] = true;
					node[w] = fib_heap_insert(&heap, (void*)(long) w, node[v]->key + G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] += node[w]->key;
					prev[w] = e;
				} else if (node[w]->key > node[v]->key + G->E[e].weight + old_pi[v] - old_pi[w]) {
					fib_heap_decrease_key(&heap, node[w], node[v]->key + G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] += node[w]->key;
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
					node[w] = fib_heap_insert(&heap, (void*)(long) w, node[v]->key - G->E[e].weight + pi[v] - pi[w]);
					pi[w] += node[w]->key;
					prev[w] = e;
				} else if (node[w]->key > node[v]->key - G->E[e].weight + old_pi[v] - old_pi[w]) {
					fib_heap_decrease_key(&heap, node[w], node[v]->key - G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] += node[w]->key;
					prev[w] = e;
				}
			}
		}
	}

	err(1, "Error: no possible solution.\n");
}
