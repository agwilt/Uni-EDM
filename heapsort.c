#include <stdio.h>
#include "fib_heap.h"

int main()
{
	struct fib_heap heap = {.n = 0, .max = 0, .b = NULL};

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
