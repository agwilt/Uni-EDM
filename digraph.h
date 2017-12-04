#define _POSIX_C_SOURCE 201112L

#ifndef DIGRAPH_H
#define DIGRAPH_H

#include <stdbool.h>

#include "config.h"

struct vert {
	int id;
	int d_plus, d_minus;
	int _max_plus, _max_minus;
	int *to, *from;			// arrays of edge ids
};

// edge (x,y)
struct edge {
	int id;
	int x, y;
	long weight;
};

struct graph {
	int n, m;
	int _max_n, _max_m;
	struct edge *E;
	struct vert *V;
};


struct graph graph_from_file(char const *filename);
void graph_free(struct graph *G);

/* return new last node */
int graph_add_nodes(struct graph *G, int num);
/* Return value: id of new edge */
int graph_add_edge(struct graph *G, int x, int y, long weight);

void graph_print(struct graph *G);

#endif
