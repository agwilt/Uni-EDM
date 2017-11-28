#define _POSIX_C_SOURCE 201112L

#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

// return number of disjoint paths
int ford_fulkerson(struct graph *G, int s, int t, int *to, int *from, int *paths);
// return if path found
bool bfs_augm_path(struct graph *G, int s, int t, int *from, int *prev);

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

	struct graph G = graph_from_file(argv[1], true);
	int *to = malloc(sizeof(int) * G.num_nodes);
	int *from = malloc(sizeof(int) * G.num_nodes);
	int *paths = malloc(sizeof(int) * G.num_nodes);
	if (to==NULL || from==NULL || paths==NULL) {
		fprintf(stderr, "Error: Go download some more RAM!\n");
	}

	// just in case, check for edges (s,t) and throw them out/print them
	for (int i=0; i<G.nodes[s].num_n; ++i) {
		if (G.nodes[s].neighbours[i].id == t) {
			graph_remove_edge(&G, s, i);
			fprintf(stderr, "Warning: detected edge (%d,%d).\n", s, t);
		}
	}

	int n_paths = ford_fulkerson(&G, s, t, to, from, paths);

	// now print paths
	for (int i=0; i<n_paths; ++i) {
		for (int v=paths[i]; v!=t; v=to[v]) {
			printf("%d ", v);
		}
		printf("\b\n");
#ifdef DEBUG
		for (int i=0; i<G.num_nodes; ++i) {
			printf("\t\t\tto[%d] = %d\tfrom[%d] = %d\n", i, to[i], i, from[i]);
		}
#endif
	}

	free(to);
	free(from);
	free(paths);
	graph_free(&G);

	return 0;
}

bool bfs_augm_path(struct graph *G, int s, int t, int *from, int *prev)
{
	prev[s] = -1;
	char *visited = calloc(G->num_nodes, 1);
	bool exists_st_path = false;

	int *q_next, *q_back, *unexplored;
	q_next = q_back = unexplored = malloc(G->num_nodes * sizeof(int));

	visited[s] = true;

	// do s separately, just in case®
	// we know that t isn't a neighbour
	for (int i=0; i<(G->nodes[s].num_n); ++i) {
		int neighbour_id = G->nodes[s].neighbours[i].id;
		if (from[neighbour_id] == -1 && !visited[neighbour_id]) {
			visited[neighbour_id] = true;
			prev[neighbour_id] = s;
			*(q_back++) = neighbour_id;
		}
		// don't need to check for edges going into s, that's silly
	}

	while (q_next != q_back) {
		// we know cur_node isn't t, because we exit once t is reached
		int cur_node = *(q_next++);
		// TODO: look at special cases
		for (int i=0; i<(G->nodes[cur_node].num_n); ++i) {
			int neighbour_id = G->nodes[cur_node].neighbours[i].id;
			// check if t
			if (neighbour_id == t) {
				visited[neighbour_id] = true;
				prev[neighbour_id] = cur_node;
				goto end;
			}
			// only look at edges if no flow
			if (from[neighbour_id] == -1 && !visited[neighbour_id]) {
				visited[neighbour_id] = true;
				prev[neighbour_id] = cur_node;
				*(q_back++) = neighbour_id;
			}
		}
		// now check for possible back edge
		if (from[cur_node] != -1) {
			if (!visited[from[cur_node]]) {
				visited[from[cur_node]] = true;
				prev[from[cur_node]] = cur_node;
				*(q_back++) = from[cur_node];
			}
		}
	}

end:
	exists_st_path = visited[t];

	free(visited);
	free(unexplored);

	return exists_st_path;
}

// find internally disjoint paths w/ ford-fulkerson/edmonds-karp
// Flow format: remember from, to id for each node (since int. disjoint)
// Remember flow from s with special array paths
//
// to, from, paths should be int arrays of length at least G->num_nodes
// return value: number of paths (paths stored in paths)
int ford_fulkerson(struct graph *G, int s, int t, int *to, int *from, int *paths)
{
	if (!G->is_directed) {
		fprintf(stderr, "Error finding s-t-flow: graph is undirected!\n");
		exit(1);
	}

	// set flow to 0 at first
	for (int i=0; i<G->num_nodes; ++i) to[i] = -1;
	for (int i=0; i<G->num_nodes; ++i) from[i] = -1;
	// remember number of paths
	int n_paths = 0;

	// save individual paths: each node remembers previous node
	int *prev = malloc(sizeof(int) * G->num_nodes);

	// find max. flow
	while (bfs_augm_path(G, s, t, from, prev)) {
		// prev now contains s-t-path:
		// augment
		for (int cur_node=t; cur_node != s; cur_node = prev[cur_node]) {
			// assume (prev,cur) is normal edge not yet taken
			if (cur_node == t || from[cur_node] == -1) {		// (prev,cur) not yet taken by flow
				to[prev[cur_node]] = cur_node;
				if (prev[cur_node] == s) {	// if prev is s, add path
					paths[n_paths++] = cur_node;
				}
				from[cur_node] = prev[cur_node];
			} else if (to[cur_node] == prev[cur_node] && from[prev[cur_node]] == cur_node) {	// (cur,prev) was taken
				to[cur_node] = -1;
				from[prev[cur_node]] = -1;
			} else {
				printf("Halp, something bad!\n");
			}
		}
	}

	free(prev);
	return n_paths;
}
