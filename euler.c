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

/*
// idea: copy old graph over to new one, thereby saving the whole thing
struct link *graph_euler_cycle(struct graph *G)
{
	struct link *first;
	struct link *current;
	struct link *start;

	node_id x = 0;

	start = current = first = path_prepend_link(x, NULL);

	for (start = current; start != NULL;) {
		x = current->id;

		while (G->nodes[x].num_n > 0) {
			graph_remove_edge(G, x, 0);
			x = G->nodes[x].neighbours[0].id; // x is dead. Long live the new x!
			current = path_add_link(x, current); // new x will forever go down in the annals of history!
		}

		// find new place to start
		current = start;
		while (current != NULL && G->nodes[current->id].num_n == 0) {
			current = current->next;
		}
		start = current;
	}

	return first;
}
*/

// last != NULL: append list *last to output
struct link *graph_euler_cycle(struct graph *G, struct link *path)
{
	printf("I was called!\n");
	struct link *current = path;
	struct link *last=path->next;
	node_id x = path->id;
	while (G->nodes[x].num_n > 0) {
		x = graph_remove_edge(G, x, 0); // remove edge (x,y), update x to y
		current = path_add_link(x, current); // and add new x to path
	}


	current = path;
	struct link *next;
	while (current->next !=NULL) {
		printf("Extra round!\n");
		next = current->next;
		current->next = graph_euler_cycle(G, current);
		current = next;
	}

	current->next = last;
	printf("Bye!\n");
	return path;
}
