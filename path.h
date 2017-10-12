#define _GNU_SOURCE
#include "graph.h"
#pragma once

struct link {
	node_id id;
	struct link *next;
};

// make new list: path_prepend_link(id, NULL) or path_add_link(id, NULL)

struct link *path_prepend_link(node_id id, struct link *first);
struct link *path_add_link(node_id id, struct link *prev);
void path_print(struct link *first);
