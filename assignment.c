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

#ifdef DEBUG
	if (G.n < 100) {
		graph_print(&G);
	} else {
		printf("n = %d, m = %d\n", G.n, G.m);
	}
#endif

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
#ifdef DEBUG
	printf("successive_shortest_paths(G, pi=%p, f=%p)\n", (void*) pi, (void*) f);
#endif
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

#ifdef DEBUG
		printf("Will now augment from %d to %d\n", x, y);
#endif
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
#ifdef DEBUG
	printf("dijkstra(G, pi=%p, f=%p, ex=%p, x=%d, prev=%p)\n", (void*) pi, (void*) f, (void*) ex, x, (void*) prev);
	printf("Reduced costs are:\n");
#endif
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
#ifdef DEBUG
		printf("Dijkstra Iteration, starting from v=%d!\n", v);
#endif
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
#ifdef DEBUG
		printf("\tnow scanning neighbours ...\n");
#endif
		for (int i=0; i<G->V[v].d_plus; ++i) {
			int e = G->V[v].to[i];
#ifdef DEBUG
			printf("\tLooking forward at e=%d\n", e);
#endif
			if (f[e] == 0) {
				int w = G->E[e].y;
				if (G->E[e].weight + pi[v] - pi[w] < 0) err(1, "ERROR: Reduced costs in G c_pi((%d,%d))=%ld\n", v, w, G->E[e].weight + pi[v] - pi[w]);
				if (! visited[w]) {
#ifdef DEBUG
					printf("\t\tfound new neighbour %d from %d in G\n", w, v);
#endif
					visited[w] = true;
					node[w] = fib_heap_insert(&heap, G->V+w, node_addr->key + G->E[e].weight + pi[v] - pi[w]);
					prev[w] = e;
				} else if (node[w]->key > node_addr->key + G->E[e].weight + pi[v] - pi[w]) {
#ifdef DEBUG
					printf("\t\tupdated edge (%d,%d) in G\n", v, w);
#endif
					fib_heap_decrease_key(&heap, node[w], node_addr->key + G->E[e].weight + pi[v] - pi[w]);
					prev[w] = e;
				}
			}
		}
		for (int i=0; i<G->V[v].d_minus; ++i) {
			int e = G->V[v].from[i];
#ifdef DEBUG
			printf("\tLooking back at e=%d\n", e);
#endif
			if (f[e] == 1) {
				int w = G->E[e].x;
				if (-G->E[e].weight + pi[v] - pi[w] < 0) err(1, "ERROR: Reduced costs in G_back c_pi((%d,%d))=%ld\n", v, w, -G->E[e].weight + pi[v] - pi[w]);
				if (! visited[w]) {
#ifdef DEBUG
					printf("\t\tfound new neighbour %d from %d in G_f\n", w, v);
#endif
					visited[w] = true;
					node[w] = fib_heap_insert(&heap, G->V+w, node_addr->key - G->E[e].weight + pi[v] - pi[w]);
					prev[w] = e;
				} else if (node[w]->key > node_addr->key - G->E[e].weight + pi[v] - pi[w]) {
#ifdef DEBUG
					printf("\t\tupdated edge (%d,%d) in G_f\n", v, w);
#endif
					fib_heap_decrease_key(&heap, node[w], node_addr->key - G->E[e].weight + pi[v] - pi[w]);
					prev[w] = e;
				}
			}
		}
	}

	err(1, "Error: no possible solution.\n");
}
