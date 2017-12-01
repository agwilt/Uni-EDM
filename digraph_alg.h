#define _POSIX_C_SOURCE 201112L

#ifndef DIGRAPH_ALG_H
#define DIGRAPH_ALG_H

#include "digraph.h"
#include "config.h"

long *digraph_max_flow(struct graph *G, int s, int t);

long digraph_flow_val(struct graph *G, int s, long *f);

#endif
