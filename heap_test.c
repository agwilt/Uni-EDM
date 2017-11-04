#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <stdlib.h>
#include "fib_heap.h"

int main()
{
	struct fib_heap heap = {.n = 0, .b = NULL};
	char *input = NULL;
	size_t len = 0;
	struct fib_node *v;
	double key;

	while (1) {
		printf("> ");
		getline(&input, &len, stdin);
		switch (input[0]) {
			case 'h':
				printf("h .. Help\nq .. Quit\ne .. Extract Min\ni key .. Insert\np .. Print\nd address key .. Decrease Key\n");
				break;
			case 'q':
				return 0;
			case 'e':
				v = fib_heap_extract_min(&heap);
				if (v) printf("Key: %lf\n", v->key);
				else printf("Error: Empty heap.\n");
				break;
			case 'i':
				fib_heap_insert(&heap, NULL, strtod(input+2, NULL));
				break;
			case 'p':
				fib_print_heap(&heap);
				break;
			case 'd':
				sscanf(input+2, "%p %lf", (void **) &v, &key);
				fib_heap_decrease_key(&heap, v, key);
				break;
			default:
				printf("Error: Command not found.\n");
		}
		free(input);
		input=NULL;
		len=0;
	}

	return 0;
}
