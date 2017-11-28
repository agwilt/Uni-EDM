#define _POSIX_C_SOURCE 201112L

#include <stdlib.h>
#include <stdio.h>
#include "graph.h"
#include "graph_alg.h"
#include "fib_heap.h"

#include "config.h"

static double bfs_augm_path(struct graph *G, struct graph *G_back, double **f, int s, int t, int *prev);

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

static double bfs_augm_path(struct graph *G, struct graph *G_back, double **f, int s, int t, int *prev)
{
	prev[s] = -1;
	char *visited = calloc(G->num_nodes, 1);
	double *residual = calloc(G->num_nodes, sizeof(double));

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
			// only consider node if edge isn't saturated
			if (!visited[neighbour_id] && f[cur_node][neighbour_id] < G->nodes[cur_node].neighbours[i].weight) {
				visited[neighbour_id] = true;
				residual[neighbour_id] = G->nodes[cur_nodes].neighbours[i].weight - f[cur_node][neighbour_id];
				prev[neighbour_id] = cur_node;
				*(q_back++) = neighbour_id;
				if (neighbour_id == t)
					goto end;
			}
		}
		for (int i=0; i<(G_back->nodes[cur_node].num_n); ++i) {
			int neighbour_id = G_back->nodes[cur_node].neighbours[i].id;
			// only consider edge if flow is positive
			if (!visited[neighbour_id] && f[neighbour_id][cur_node] > 0) {
				visited[neighbour_id] = true;
				residual[neighbour_id] = f[neighbour_id][cur_node];
				prev[neighbour_id] = cur_node;
				*(q_back++) = neighbour_id;
				if (neighbour_id == t)
					goto end;
			}
		}
	}

end:
	// now calculate val
	double val;
	if (visited[t]) {
		val = HUGE_VAL;
		for (int v=t; v!=s; v=prev[v])
			if (residual[prev[v]][v] < val)
				val = residual[prev[v]][v];
	} else
		val = 0;

	free(visited);
	free(unexplored);
	free(residual);

	return val;
}

double graph_edmonds_karp(struct *G, int s, int t, double **f)
{
	if (!G->is_directed) {
		fprintf(stderr, "Error: Graph is undirected!\n");
		exit(1);
	}

	// set initial flow to 0
	struct graph f = {.num_nodes = 0, .max_nodes = 0, .nodes = NULL, .is_directed=true};
	graph_add_nodes(&f, G->num_nodes);
	for (int cur_node=0; current<G->num_nodes; cur_node++) {
		for (int i=0; i<G->nodes[cur_node].num_n; ++i) {
			graph_add_edge(&f, cur_node, G->nodes[cur_node].neighbours[i].id, 0.0);
		}
	}

	// flow saved as 2-dim array, intialized to 0
	if (f == NULL) {
		f = malloc(G->num_nodes * sizeof(double *));
		for (int i=0; i<G->num_nodes; ++i) {
			f[i] = calloc(G->num_nodes * sizeof(double));
		}
	}

	// new graph: G_back, where all the edges are the wrong way round
	struct graph G_back = {.num_nodes=0, .max_nodes=0, .nodes=NULL, .is_directed=true};
	graph_add_nodes(&G_back, G->num_nodes);
	for (int cur_node=0; current<G->num_nodes; cur_node++) {
		for (int i=0; i<G->nodes[cur_node].num_n; ++i) {
			graph_add_edge(&f, G->nodes[cur_node].neighbours[i].id, cur_node, G->nodes[cur_node].neighbours[i].weight);
		}
	}

	// save s-t-paths in prev
	int *prev = malloc(G->num_nodes * sizeof(int));

	double val;
	double total_val = 0;
	while (val = bfs_augm_path(G, G_back, f, s, t, prev)) {
		// prev is now s-t-path, val is value by which to augment
		for (int v=t; v!=s; v=prev[v]) {
			if (f[v][prev[v]] >= val)
				f[v][prev[v]] -= val;
			else
				f[prev[v]][v] += val;
		}
		total_val += val;
	}
	// now we have a max. s-t-flow

	return total_val;
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
