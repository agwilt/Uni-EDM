#define _GNU_SOURCE

#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX_NEIGH 1

struct node {
	int num_n;
	int max_n;
	struct neighbour *neighbours;
};

struct neighbour {
	double weight;
	int id;
};

struct graph {
	int num_nodes;
	int max_nodes;
	struct node *nodes;
	bool is_directed;
};

int graph_remove_edge(struct graph *G, int x, int offset);

struct graph graph_from_file(char const *filename, bool is_directed);

// free all the nodes' RAM, as well as G->nodes
void graph_free(struct graph *G);

int graph_add_nodes(struct graph *G, int n);
void graph_add_edge(struct graph *G, int start, int end, double weight);

void graph_print(struct graph *G);

/*
 * ALGORITHMS:
 */


// return number of Zshgskomp
int graph_zus_komp(struct graph *G);

// minimal spanning tree w/ Prim, fibonacci heaps (print edges to stdout)
void graph_mst(struct graph *G);

#endif
