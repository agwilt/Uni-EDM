#define _POSIX_C_SOURCE 201112L

#include <stdlib.h>
#include <stdio.h>
#include "path.h"

#include "config.h"

struct link *path_prepend_link(int id, struct link *first)
{
	struct link *new_link = malloc(sizeof(struct link));
	new_link->id = id;
	new_link->next = first;
	return new_link;
}

struct link *path_add_link(int id, struct link *prev)
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
	// this enables full buffering to greatly speed up output with lots of newlines
	setvbuf(stdout, NULL, _IOFBF, 0);
	while (first != NULL) {
		printf("%d\n", first->id);
		first = first->next;
	}
}
