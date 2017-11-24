#define _POSIX_C_SOURCE 201112L

#include <stdio.h>

static struct node **ford_fulkerson(struct graph *G, int s, int t);

int main(int argc, char *argv[])
{
}

// F-F only for graphs with weight 1 everywhere, so that flow can be saved in an
// efficient way
static struct node **ford_fulkerson(struct graph *G, int s, int t)
{
	if (!G->is_directed) {
		fprintf(stderr, "Error finding s-t-flow: graph is undirected!\n");
		exit(1);
	}

	// save flow in augm. graph
	struct graph Gf = graph_duplicate(G);

	// save individual paths
	int *prev = malloc(sizeof(int) * G->num_nodes);

	while (bfs_augm_path(G, s, t, to, from, prev)) {
		// prev now contains s-t-path
	}
}
