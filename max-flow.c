#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <stdlib.h>

#include "digraph.h"
#include "config.h"

int main(int argc, char *argv[])
{
	int s = 0;
	int t = 1;

	if (argc<=1) {
		printf("Usage: %s filename [s t]\nDefault: s=0, t=1.\n", argv[0]);
		return 1;
	} else if (argc>=4) {
		s = atoi(argv[2]);
		t = atoi(argv[3]);
		if (s == t) {
			fprintf(stderr, "Error: s = t.\n");
			return 1;
		}
	}

	struct graph G = graph_from_file(argv[1]);

	return 0;
}
