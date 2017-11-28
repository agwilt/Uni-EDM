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

	double **f = NULL;
	graph_edmonds_karp(&G, s, t, &f);
	int *start = calloc(G.num_nodes, sizeof(int));
	int v, num=0;
	start[t] = -1;

	// while value > 0, try to take apart flow, starting at start_edge
	while (true) {
		for (v=s; start[v]!=-1; v=start[v]) {
			while (start[v] < G.num_nodes && f[v][start[v]] == 0) ++(start[v]);
			if (start[v] == G.num_nodes) goto end;
			printf("%d ", start[v]);
			f[v][start[v]]--;
		}
		printf("\n");

		num++;
	}
end:
	printf("Anzahl: %d\n", num);

	free(f);

	return 0;
}
