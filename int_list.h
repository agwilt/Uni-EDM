#define _POSIX_C_SOURCE 201112L

#ifndef INT_LIST_H
#define INT_LIST_H

struct list {
	int *array;
	size_t len, max_len;
};

void append_to_list(struct list *list, int x);
void free_lists(struct list *lists, int n);

#endif
