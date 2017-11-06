#define _POSIX_C_SOURCE 201112L

#ifndef GRAPH_ALG_H
#define GRAPH_ALG_H

#include "graph.h"

#include "config.h"

// minimal spanning tree w/ Prim, fibonacci heaps (print edges to stdout)
double graph_mst(struct graph *G);

#endif
