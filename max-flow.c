#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <stdlib.h>

#include "digraph.h"
#include "push-relabel.h"
#include "int_list.h"
#include "config.h"

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

	struct graph G = graph_from_file(argv[1]);

	/* pack G */
	/* silly idea: make dummy edge 0, fill everything with 0s */
	int **edges = malloc(G.n * sizeof(int *));
	for (int i=0; i<G.n; ++i) {
		edges[i] = malloc(G.n * sizeof(int));
		for (int j=0; j<G.n; ++j)
			edges[i][j] = -1;
	}

	struct graph G_compr = {.n=0, .m=0, ._max_n=0, ._max_m=0, .E=NULL, .V=NULL};
	graph_add_nodes(&G_compr, G.n);
	/* build new graph */
	for (int e=0; e<G.m; ++e) {
		int x = G.E[e].x;
		int y = G.E[e].y;
		if (edges[x][y] == -1) {	/* New edge */
			edges[x][y] = graph_add_edge(&G_compr, x, y, G.E[e].weight);
		} else {
			G_compr.E[edges[x][y]].weight += G.E[e].weight;
		}
	}

	/* compute flow in simple graph, output val */
	long *f_compr = digraph_max_flow(&G_compr, s, t);

	setvbuf(stdout, NULL, _IOFBF, 0);
	printf("%ld\n", digraph_flow_val(&G_compr, s, f_compr));

	/* Now, unpack again */
	long *f = calloc(G.m, sizeof(long));
	for (int e=0; e<G.m; ++e) {
		int x = G_compr.E[e].x;
		int y = G_compr.E[e].y;
		/* set to -1 if weight == 0 */
		if (edges[x][y] != -1) {
			if (G_compr.E[edges[x][y]].weight > G.E[e].weight) {
				f[e] = G.E[e].weight;
				G_compr.E[edges[x][y]].weight -= f[e];
			} else {
				f[e] = G_compr.E[edges[x][y]].weight;
				edges[x][y] = -1;
			}
		}
	}
	free(f_compr);
	for (int i=0; i<G.n; ++i)
		free(edges[i]);

	digraph_flow_print(f, G.m);

	/* free stuff */
	free(f);
	graph_free(&G);
	graph_free(&G_compr);

	return 0;
}
