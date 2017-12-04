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
	struct list **edges = malloc(G.n * sizeof(struct list *));
	bool *layed_edge = calloc(G.n, G.n * sizeof(bool));
	long *weights = calloc(G.n, G.n * sizeof(long));
	for (int i=0; i<G.n; ++i)
		edges[i] = calloc(G.n, sizeof(struct list));
	/* populate edges */
	for (int e=0; e<G.m; ++e) {
		int x = G.E[e].x;
		int y = G.E[e].y;
		append_to_list(edges[x]+y, e);
		weights[G.n*x + y] += G.E[e].weight;
	}
	/* build new graph */
	struct graph G_compr = {.n=0, .m=0, ._max_n=0, ._max_m=0, .E=NULL, .V=NULL};
	graph_add_nodes(&G_compr, G.n);
	for (int e=0; e<G.m; ++e) {
		int x = G.E[e].x;
		int y = G.E[e].y;
		if (!layed_edge[G.n*x + y] && edges[x][y].len > 0) {
			layed_edge[G.n*x + y] = true;
			graph_add_edge(&G_compr, x, y, weights[G.n*x + y]);
		}
	}
	/* Done packing */

	long *f_compr = digraph_max_flow(&G_compr, s, t);

	setvbuf(stdout, NULL, _IOFBF, 0);
	printf("%ld\n", digraph_flow_val(&G_compr, s, f_compr));

	/* Now, unpack again */
	long *f = calloc(G.m, sizeof(long));
	for (int e=0; e<G_compr.m; ++e) {
		int x = G_compr.E[e].x;
		int y = G_compr.E[e].y;
		for (size_t i=0; i<edges[x][y].len && weights[G.n*x+y]>0; ++i) {
			if (weights[G.n*x + y] >= G.E[edges[x][y].array[i]].weight) {
				f[edges[x][y].array[i]] += G.E[edges[x][y].array[i]].weight;
				weights[G.n*x + y] -= G.E[edges[x][y].array[i]].weight;
			}
		}
	}

	digraph_flow_print(f, G.m);

	/* free stuff */
	free(layed_edge);
	free(weights);
	for (int i=0; i<G.n; ++i)
		free_lists(edges[i], G.n);
	free(f);
	free(f_compr);
	graph_free(&G);
	graph_free(&G_compr);

	return 0;
}
