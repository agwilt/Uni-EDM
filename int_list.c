#define _POSIX_C_SOURCE 201112L

#include <stdlib.h>
#include "int_list.h"

/* Intelligently add an int to a list */
void append_to_list(struct list *list, int x)
{
	if (list->max_len == 0) {
		list->array = malloc(sizeof(int));
		list->max_len = 1;
	} else if (list->len >= list->max_len) {
		list->array = realloc(list->array, sizeof(int)*2*list->len);
		list->max_len <<= 1;
	}
	list->array[list->len] = x;
	list->len++;
}

/* Free an entire array of lists, and the array itself */
void free_lists(struct list *lists, int n)
{
	for (int i=0; i<n; ++i)
		if (lists[i].max_len > 0)
			free(lists[i].array);
	free(lists);
}
