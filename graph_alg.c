#define _POSIX_C_SOURCE 201112L

#include <stdlib.h>
#include <stdio.h>
#include "graph.h"
#include "graph_alg.h"
#include "fib_heap.h"

// prints list of edges to stdout
double graph_mst(struct graph *G)
{
	if (G->is_directed) {
		fprintf(stderr, "Error generating MST: graph is directed!\n");
		exit(1);
	}

	struct edge {
		int from;
		int to;
	};

	// priority queue
	struct fib_heap heap = {.n = 0, .b = NULL};
	// nodes alredy in resulting tree
	char *visited = calloc(G->num_nodes, 1);
	// f_nodes[i] is the address of the fib_node corresponding to i (or null)
	struct fib_node **f_nodes = calloc(G->num_nodes, sizeof(struct fib_node **));
	// just to store both ends of edge
	struct edge *edges = malloc(G->num_nodes * sizeof(struct edge));

	double total_weight = 0;
	int v_id = 0;
	struct node *v;

	// go through neighbours
	while (1) {

		// look at current node
		v = G->nodes+v_id;
		for (int n = 0; n < v->num_n; ++n) {
			int n_id = v->neighbours[n].id;
			// skip if already visited
			if (visited[n_id]) continue;

			if (f_nodes[n_id] == NULL) {
				f_nodes[n_id] = fib_heap_insert(&heap, (void *) (edges+n_id), v->neighbours[n].weight);
				edges[n_id] = (struct edge) {.from = v_id, .to = n_id};
			} else if (v->neighbours[n].weight < f_nodes[n_id]->key) {
				fib_heap_decrease_key(&heap, f_nodes[n_id], v->neighbours[n].weight);
			}
		}

		// choose next node
		struct fib_node *f = fib_heap_extract_min(&heap);
		if (f == NULL) break;  // break if heap empty
		v_id = ((struct edge *) f->val)->from;
		printf("%d %d\n", v_id, ((struct edge *) f->val)->to);
		visited[v_id] = true; 
		total_weight += f->key;
		free(f);
	}

	free(visited);
	free(f_nodes);
	free(edges);

	return total_weight;
}
