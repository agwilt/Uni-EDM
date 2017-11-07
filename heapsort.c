#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <stdlib.h>
#include "fib_heap.h"

#include "config.h"

int main()
{
	struct fib_heap heap = {.n = 0, .b = NULL};

	double key;
	int n = 0;

	char *line = NULL;
	size_t len = 0;

	while (getline(&line, &len, stdin)) {
		if (line[0] == '\0') break;
		//i = strtod(line, NULL);
		if (sscanf(line, "%lf", &key)) {
			fib_heap_insert(&heap, NULL, key);
			++n;
		}

		free(line);
		line = NULL;
		len = 0;
	}

	while (n > 0) {
		struct fib_node *node = fib_heap_extract_min(&heap);
		printf("%lf\n", node->key);
		--n;
	}

	return 0;
}
