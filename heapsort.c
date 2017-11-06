#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include "fib_heap.h"

#include "config.h"

int main()
{
	struct fib_heap heap = {.n = 0, .b = NULL};

	int i;
	int n = 0;

	while (scanf("%d", &i)) {
		fib_heap_insert(&heap, NULL, i);
		++n;
	}

	while (n > 0) {
		struct fib_node *node = fib_heap_extract_min(&heap);
		printf("%lf\n", node->key);
		--n;
	}

	return 0;
}
