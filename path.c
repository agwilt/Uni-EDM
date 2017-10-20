#include <stdlib.h>
#include <stdio.h>
#include "path.h"

struct link *path_prepend_link(node_id id, struct link *first)
{
	struct link *new_link = malloc(sizeof(struct link));
	new_link->id = id;
	new_link->next = first;
	return new_link;
}

struct link *path_add_link(node_id id, struct link *prev)
{
	struct link *new_link = malloc(sizeof(struct link));
	new_link->id = id;
	if (prev != NULL) {
		new_link->next = prev->next;
		prev->next = new_link;
	} else {
		new_link->next = NULL;
	}
	return new_link;
}

void path_print(struct link *first)
{
	setvbuf(stdout, NULL, _IOFBF, 0);
	while (first != NULL) {
		printf("%ld\n", first->id);
		first = first->next;
	}
}
