#define _POSIX_C_SOURCE 201112L

#ifndef PUSH_RELABEL_H
#define PUSH_RELABEL_H

#include "digraph.h"
#include "config.h"

long *digraph_max_flow(struct graph *G, int s, int t);

long digraph_flow_val(struct graph *G, int s, long *f);

void digraph_flow_print(long *f, int m);

#endif
