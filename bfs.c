#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "graph_alg.h"

#include "config.h"

void print_path(int *prev, int t)
{
	if (prev[t] != -1) print_path(prev, prev[t]);
	printf("%d ", t);
}

int main(int argc, char *argv[])
{
	if (argc<=3) {
		printf("Usage: %s graph_file start end\n", argv[0]);
		return 1;
	}
	struct graph G = graph_from_file(argv[1], true);
	int *prev = malloc(sizeof(int) * G.num_nodes);
	int s = atoi(argv[2]);
	int t = atoi(argv[3]);

#ifdef DEBUG
	printf("Graph:\n");
	graph_print(&G);
#endif

	if (s<0 || t<0 || s>G.num_nodes || t>G.num_nodes || s==t) {
		printf("Error: invalid s, t!\n");
		return 1;
	}

	if (graph_bfs_path(&G, s, t, prev)) {
		print_path(prev, t);
	} else {
		printf("Sorry, no s-t-path.\n");
	}

	return 0;
}
