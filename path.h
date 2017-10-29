#ifndef PATH_H
#define PATH_H

#include "graph.h"

struct link {
	int id;
	struct link *next;
};

// make new list: path_prepend_link(id, NULL) or path_add_link(id, NULL)

struct link *path_prepend_link(int id, struct link *first);
struct link *path_add_link(int id, struct link *prev);
void path_print(struct link *first);

#endif
