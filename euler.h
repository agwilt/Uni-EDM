#define _GNU_SOURCE
#include "graph.h"
#include <stdbool.h>
#pragma once

bool graph_is_eulerian(struct graph *G);
// warning: destroys graph!
// path: first link, path->next: end of new path
struct link *graph_euler_cycle(struct graph *G, struct link *path);
