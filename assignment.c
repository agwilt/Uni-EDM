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
	for (int v=0; v<G.n/2; ++v) {
		if (G.V[v].d_plus != G.n/2) err(1, "Error: Bad graph: d_plus(%d)=%d\n", v, G.V[v].d_plus);
	}
	for (int v=G.n/2; v<G.n; ++v) {
		if (G.V[v].d_minus != G.n/2) err(1, "Error: Bad graph: d_minus(%d)=%d\n", v, G.V[v].d_minus);
	}

	/* generate initial zul. pot */
	int *pi = calloc(G.n, sizeof(int));
	for (int e=0; e<G.m; ++e)
		if (G.E[e].weight < pi[G.E[e].y])
			pi[G.E[e].y] = G.E[e].weight;
	char *f = calloc(G.m, sizeof(char));

//	graph_print(&G);

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
//	printf("successive_shortest_paths(G, pi=%p, f=%p)\n", (void*) pi, (void*) f);
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

//		printf("Will now augment from %d to %d\n", x, y);
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
//	printf("dijkstra(G, pi=%p, f=%p, ex=%p, x=%d, prev=%p)\n", (void*) pi, (void*) f, (void*) ex, x, (void*) prev);
	char *visited = calloc(G->n, sizeof(char));
	struct fib_node **node = malloc(G->n * sizeof(struct fib_node *));
	struct fib_heap heap = {.n=0, .b=NULL};

	int *old_pi = malloc(G->n * sizeof(int));
	for (int i=0; i<G->n; ++i)
		old_pi[i] = pi[i];

	visited[x] = true;
	node[x] = fib_heap_insert(&heap, G->V+x, 0);

	struct fib_node *node_addr;
	while ((node_addr = fib_heap_extract_min(&heap)) != 0) {
		int v = ((struct vert*) node_addr->val)->id;
		free(node_addr);
//		printf("Dijkstra Iteration, starting from v=%d!\n", v);
		/* return if valid y found */
		if (ex[v] == 0 && v >= G->n/2) {
			free(visited);
			free(node);
			free(old_pi);
			fib_heap_free(&heap);
			return v;
		}
		/* look at neighbours */
//		printf("\tnow scanning neighbours ...\n");
		for (int i=0; i<G->V[v].d_plus; ++i) {
			int e = G->V[v].to[i];
//			printf("\tLooking at e=%d\n", e);
			if (f[e] == 0) {
				int w = G->E[e].y;
				if (! visited[w]) {
//					printf("\t\tfound new neighbour %d from %d in G\n", w, v);
					visited[w] = true;
					node[w] = fib_heap_insert(&heap, G->V+w, node[v]->key + G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] += node[w]->key;
					prev[w] = e;
				} else if (node[w]->key > node[v]->key + G->E[e].weight + old_pi[v] - old_pi[w]) {
//					printf("\t\tupdated edge (%d,%d) in G\n", v, w);
					fib_heap_decrease_key(&heap, node[w], node[v]->key + G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] += node[w]->key;
					prev[w] = e;
				}
			}
		}
		for (int i=0; i<G->V[v].d_minus; ++i) {
			int e = G->V[v].from[i];
//			printf("\tLooking at e=%d\n", e);
			if (f[e] == 1) {
				int w = G->E[e].x;
				if (! visited[w]) {
//					printf("\t\tfound new neighbour %d from %d in G_f\n", w, v);
					visited[w] = true;
					node[w] = fib_heap_insert(&heap, G->V+w, node[v]->key - G->E[e].weight + pi[v] - pi[w]);
					pi[w] += node[w]->key;
					prev[w] = e;
				} else if (node[w]->key > node[v]->key - G->E[e].weight + old_pi[v] - old_pi[w]) {
//					printf("\t\tupdated edge (%d,%d) in G_f\n", v, w);
					fib_heap_decrease_key(&heap, node[w], node[v]->key - G->E[e].weight + old_pi[v] - old_pi[w]);
					pi[w] += node[w]->key;
					prev[w] = e;
				}
			}
		}
	}

	err(1, "Error: no possible solution.\n");
}
