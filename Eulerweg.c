#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "euler.h"
#include "path.h"

int main(int argc, char *argv[])
{
	if (argc<=1) return 1;

	struct graph G = graph_from_file(argv[1], false);

	int zus_komp = graph_zus_komp(&G);

	if (zus_komp > 1) {
		printf("Graph is disconnected, with %d components. Also not very Eulerian.\n", zus_komp);
		return 0;
	} else if (graph_is_eulerian(&G)) {
		struct link *path = path_prepend_link(0, NULL);
		graph_euler_cycle(&G, path);
		path_print(path);
	} else {
		printf("Graph is connected but not Eulerian.\n");
	}

	return 0;
}
