#define _GNU_SOURCE
#include "graph.h"
#include <stdbool.h>
#pragma once

bool graph_is_eulerian(struct graph *G);
// warning: destroys graph!
struct link *graph_euler_cycle(struct graph *G);
