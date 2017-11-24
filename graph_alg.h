#define _POSIX_C_SOURCE 201112L

#ifndef GRAPH_ALG_H
#define GRAPH_ALG_H

#include "graph.h"

#include "config.h"

// try to find "shortest" s-t-path with BFS; say if one exists at all
bool graph_bfs_path(struct graph *G, int s, int t, int *prev);

// minimal spanning tree w/ Prim, fibonacci heaps (print edges to stdout)
double graph_mst(struct graph *G);

#if 0
// s-t-flow w/ Ford-Fulkerson for directed graphs
double *graph_ford_fulkerson(struct graph *G, int s, int t);
#endif

#endif
