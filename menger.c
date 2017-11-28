#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "graph_alg.h"

int main(int argc, char *argv[])
{
	int s = 0;
	int t = 1;

	if (argc<=1) {
		printf("Usage: %s filename [s t]\nDefault: s=0, t=1.\n", argv[0]);
		return 1;
	} else if (argc>=4) {
		s = atoi(argv[2]);
		t = atoi(argv[3]);
		if (s == t) {
			fprintf(stderr, "Error: s = t.\n");
			return 1;
		}
	}

	struct graph G = graph_from_file(argv[1], true);

	// just in case, check for edges (s,t) and throw them out/print them
	for (int i=0; i<G.nodes[s].num_n; ++i) {
		if (G.nodes[s].neighbours[i].id == t) {
			graph_remove_edge(&G, s, i);
			fprintf(stderr, "Warning: detected edge (%d,%d).\n", s, t);
		}
	}

	double **flow = NULL;
	double val = graph_edmonds_karp(&G, s, t, f);
	int *next = malloc(sizeof(int) * G->num_nodes);
	next[s] = next[t] = -1;

	struct {int x; int y} start_edge = {0,0};

	// while value > 0, try to take apart flow, starting at start_edge
	while (val > 0) {
		// find next start_edge
		while (f[start_edge.x][start_edge.y] == 0) {
			if (y == G->num_nodes-1) {
				y = 0;
				++x;
			} else ++y;
		}
		int v = start_edge.y;

		if (found path)
			val--;
	}

	return 0;
}
