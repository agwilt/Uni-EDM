#define _POSIX_C_SOURCE 201112L

#include <stdlib.h>
#include <stdio.h>
#include "graph.h"
#include "graph_alg.h"
#include "fib_heap.h"

#include "config.h"

bool graph_bfs_path(struct graph *G, int s, int t, int *prev)
{
	prev[s] = -1;
	char *visited = calloc(G->num_nodes, 1);
	bool exists_st_path = false;

	int *q_next, *q_back, *unexplored;
	q_next = q_back = unexplored = malloc(G->num_nodes * sizeof(int));
	// q_back is next place to put new item
	// q_next is next item to take

	*(q_back++) = s; // push s
	visited[s] = true;

	while (q_next != q_back) {
		int cur_node = *(q_next++);
		for (int i=0; i<(G->nodes[cur_node].num_n); ++i) {
			int neighbour_id = G->nodes[cur_node].neighbours[i].id;
			if (!visited[neighbour_id]) {
				visited[neighbour_id] = true;
				prev[neighbour_id] = cur_node;
				*(q_back++) = neighbour_id;
				if (neighbour_id == t)
					goto end;
			}
		}
	}

end:
	exists_st_path = visited[t];

	free(visited);
	free(unexplored);

	return exists_st_path;
}

// prints list of edges to stdout
double graph_mst(struct graph *G)
{
	if (G->is_directed) {
		fprintf(stderr, "Error generating MST: graph is directed!\n");
		exit(1);
	}

	// save cheap way of getting to .to
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
	// just to store both ends of edge. edges[w] is the edge {v,w} that could connact w to the tree
	struct edge *edges = malloc(G->num_nodes * sizeof(struct edge));

	double total_weight = 0;
	struct node *v; // pointer to current node
	struct fib_node *f;

	int v_id = 0;	// id of current node
	visited[0] = true;

	setvbuf(stdout, NULL, _IOFBF, 0);

	// go through neighbours
	while (1) {

		// look at current node
		v = G->nodes+v_id;

		for (int n = 0; n < v->num_n; ++n) {
			int n_id = v->neighbours[n].id;
			// if neighbour is unvisited (not already in tree), see if edge is interesting
			if (! visited[n_id]) {
				if (f_nodes[n_id] == NULL) {						// if not already in heap
					f_nodes[n_id] = fib_heap_insert(&heap, (void *) (edges+n_id), v->neighbours[n].weight);
					edges[n_id] = (struct edge) {.from = v_id, .to = n_id};			// remember how we got to n_id
				} else if (v->neighbours[n].weight < f_nodes[n_id]->key) {		// else update if needed
					fib_heap_decrease_key(&heap, f_nodes[n_id], v->neighbours[n].weight);
					edges[n_id] = (struct edge) {.from = v_id, .to = n_id};			// remember how we got to n_id
				}
			}
		}

		visited[v_id] = true; 

		// choose next node
		f = fib_heap_extract_min(&heap);
#ifdef DEBUG
		printf("Extracted %p from heap.\n", (void *) f);
		printf("Heap now looks like:\n");
		fib_print_heap(&heap);
		printf("Edges:\n");
		for (int i= 0; i<G->num_nodes; ++i) {
			printf("%d connected via %d\n", edges[i].to, edges[i].from);
		}
#endif
		if (f == NULL) break;  // break if heap empty

		v_id = ((struct edge *) f->val)->to;
		printf("%d %d\n", ((struct edge *) f->val)->from, v_id);

		total_weight += f->key;
		// we can now free f, since visited[v_id] will be true after next round --> fib_node[v_id] won't be accessed
		free(f);
	}

	fib_heap_free(&heap);
	free(visited);
	free(f_nodes);
	free(edges);

	return total_weight;
}
