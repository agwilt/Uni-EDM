#define _POSIX_C_SOURCE 201112L

#ifndef PATH_H
#define PATH_H

#include "graph.h"

#include "config.h"

struct link {
	int id;
	struct link *next;
};

// make new list: path_prepend_link(id, NULL) or path_add_link(id, NULL)

struct link *path_prepend_link(int id, struct link *first);
struct link *path_add_link(int id, struct link *prev);
void path_print(struct link *first);
void path_free(struct link *first);

#endif
