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

// idea: copy old graph over to new one, thereby saving the whole thing
struct link *graph_euler_cycle(struct graph *G)
{
	struct link *first;
	struct link *current;
	struct link *start;

	node_id x = 0;

	start = current = first = path_prepend_link(x, NULL);

gotos_are_Fun:

	while (G->nodes[x].num_n > 0) {
		node_remove_edge(G->nodes+x, 0);
		x = G->nodes[x].neighbours[0].id; // x is dead. Long live the new x!
		current = path_add_link(x, current); // new x will forever go down in the annals of history!
	}

	// find new place to start
	current = start;
	while (G->nodes[current->id].num_n == 0) {
		current = current->next;
		if (current == NULL) return first;
	}
	x = current->id;
	start = current;

	goto gotos_are_Fun;
}
