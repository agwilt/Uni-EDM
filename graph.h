#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#pragma once

#define MAX_NEIGH 1

typedef long node_id;

// function for generating graph from file
long fast_atol(char **str);

struct node {
	node_id num_n;
	node_id max_n;
	struct neighbour *neighbours;
};

struct neighbour {
	double weight;
	node_id id;
};

struct graph {
	node_id num_nodes;
	node_id max_nodes;
	struct node *nodes;
	bool is_directed;
};

// adds (directed) neighbour to node, doubles max_n if necessary
void node_add_neighbour(struct node *x, node_id y, double weight);

// removes a neighbour from x->neighbours, and moves the last one to its place.
node_id node_remove_edge(struct node *x, node_id offset); // WARNING: moves around edges
node_id graph_remove_edge(struct graph *G, node_id x, node_id offset);

struct graph graph_from_file(char const *filename, bool is_directed);

// free all the nodes' RAM, as well as G->nodes
void graph_free(struct graph *G);

node_id graph_add_nodes(struct graph *G, node_id n);
void graph_add_edge(struct graph *G, node_id start, node_id end, double weight);

void graph_print(struct graph *G);
// return number of Zshgskomp
int graph_zus_komp(struct graph *G);
