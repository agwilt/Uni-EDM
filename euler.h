#define _POSIX_C_SOURCE 201112L

#ifndef EULER_H
#define EULER_H

#include "graph.h"
#include <stdbool.h>

// check if each node has even number of neighbours
bool graph_is_eulerian(struct graph *G);

// warning: destroys graph by removing edges!
struct link *graph_euler_cycle(struct graph *G, struct link *path);

#endif
