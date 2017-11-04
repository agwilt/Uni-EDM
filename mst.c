#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include "graph.h"
#include "graph_alg.h"

int main(int argc, char *argv[])
{
	if (argc<=1) return 1;
	struct graph G = graph_from_file(argv[1], false);
	printf("%lf\n", graph_mst(&G));

	return 0;
}
