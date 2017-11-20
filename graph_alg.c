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
	int *q_next, *q_back, *unexplored;
	q_next = q_back = unexplored = malloc(G->num_nodes * sizeof(int));
	// q_back is next place to put new item
	// q_next is next item to take

	*(q_back++) = s; // push r
	visited[s] = true;

	while (q_next != q_back) {
		int cur_node = *(q_next++);
		for (int i=0; i<(G->nodes[cur_node].num_n); ++i) {
			int neighbour_id = G->nodes[cur_node].neighbours[i].id;
			if (!visited[neighbour_id]) {
				visited[neighbour_id] = 1;
				prev[neighbour_id] = cur_node;
				*(q_back++) = neighbour_id;
				if (neighbour_id == t)
					goto end;
			}
		}
	}

end:

	free(visited);
	free(unexplored);

	return visited[t];
}

struct graph graph_bfs(struct graph *G, int r)
{
	struct graph T = {.num_nodes = 0, .max_nodes=0, .nodes = NULL, .is_directed=G->is_directed};
	graph_add_nodes(&T, G->num_nodes);
	// BFS stuff
	char *visited = calloc(G->num_nodes, 1);
	int *q_next, *q_back, *unexplored;
	q_next = q_back = unexplored = malloc(G->num_nodes * sizeof(int));
	// q_back is next place to put new item
	// q_next is next item to take

	*(q_back++) = r; // push r
	visited[r] = true;

	while (q_next != q_back) {
		int cur_node = *(q_next++);
		for (int i=0; i<(G->nodes[cur_node].num_n); ++i) {
			int neighbour_id = G->nodes[cur_node].neighbours[i].id;
			if (!visited[neighbour_id]) {
				visited[neighbour_id] = 1;
				graph_add_edge(&T, cur_node, neighbour_id, G->nodes[cur_node].neighbours[i].weight);
				*(q_back++) = neighbour_id;
			}
		}
	}

	free(visited);
	free(unexplored);

	return T;
}

#if 0
// Max. Flow stuff (only for weight 1.0 on all edges ...)
struct node **graph_ford_fulkerson(struct graph *G, int s, int t)
{
	if (!G->is_directed) {
		fprintf(stderr, "Error finding s-t-flow: graph is undirected!\n");
		exit(1);
	}
	// f remembers where each node is "flowing to", NULL if nowhere
	int *to = malloc(sizeof(int) * G->n);
	for (int i = 0; i<G->n; ++i) to[i] = -1;
	int *from = malloc(sizeof(int) * G->n);
	for (int i = 0; i<G->n; ++i) from[i] = -1;
	struct node **paths = calloc(sizeof(struct node *) * G->n);
	size_t paths_len = 0;

	// to save path: save previous node
	int *prev = malloc(sizeof(int) * G->n);
	for (int i = 0; i<G->n; ++i) prev[i] = -1;

	// find f-augm. path
	char *visited = calloc(G->num_nodes, 1);
	int *unexplored = malloc(G->num_nodes * sizeof(int));
	int *sp = unexplored;
	int cur_node, neighbour_id;
	visited[s] = true;
	*(sp++) = s;

	while (sp > unexplored) {
		cur_node = *(--sp);
		for (int i=0; i<(G->nodes[cur_node].num_n); ++i) {
			neighbour_id = G->nodes[cur_node].neighbours[i].id;
			if (from[neighbour_id] != cur_node && !visited[neighbour_id]) {
				visited[neighbour_id] = 1;
				prev[neighbour_id] = cur_node;
				*(sp++) = neighbour_id;
			}
		}
		if (to[cur_node] != -1 && !visited[to[cur_node]]) {
			visited[to[cur_node]] = 1;
			*(sp++) = to[cur_node];
			prev[to[cur_node]] = cur_node;
		}
	}
	if (!visited[t]) printf("Done!\n");

	// augment along path
	for (cur_node = t; cur_node != s; cur_node = prev[cur_node]) {
		if (from[cur_node] == -1) {
			to[prev[cur_node]] = cur_node;
			from[cur_node] = prev[cur_node];
		}
	}
	paths[paths_len++] = G->nodes+cur_node;
}
#endif

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
