#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "graph.h"
#include "euler.h"
#include "path.h"

bool graph_is_eulerian(struct graph *G)
{
	if (G->is_directed) {
		fprintf(stderr, "Error: can't be bothered to implement digraph euler-checking");
		exit(1);
	} else {
		for (int i = 0; i<(G->num_nodes); ++i) {
			if (G->nodes[i].num_n % 2)
				return false;
		}
		return true;
	}
}

// path: first link, path->next: end of new path
// return: original path->next
struct link *graph_euler_cycle(struct graph *G, struct link *path)
{
	// lazy exit if nothing to do
	if (G->nodes[path->id].num_n == 0) {
		return path->next;
	}

	struct link *current = path;
	struct link *last=path->next;
	int x = path->id;
	while (G->nodes[x].num_n > 0) {
		x = graph_remove_edge(G, x, 0); // remove edge (x,y), update x to y
		current = path_add_link(x, current); // and add new x to path
	}

	current = path;  // first in new loop
	while (current->next !=NULL) {
		current = graph_euler_cycle(G, current);
	}

	//current->next = last;
	return last;
}
