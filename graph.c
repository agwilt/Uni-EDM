#define _POSIX_C_SOURCE 201112L

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "graph.h"

// function for generating graph from file
static int fast_atoi(char **str);

// adds (directed) neighbour to node, doubles max_n if necessary
static void node_add_neighbour(struct node *x, int y, double weight);
// removes a neighbour from x->neighbours, and moves the last one to its place.
static int node_remove_edge(struct node *x, int offset); // WARNING: moves around edges

int fast_atoi(char **str)
{
	int val = 0;
	while ( !(**str == ' ' || **str == '\n' || **str == '\0')) {
		val = val*10 + (*(*str)++ - '0');
	}
	return val;
}

int graph_add_nodes(struct graph *G, int n)
{
	if (n <= 0) return -1;
	if (G->max_nodes == 0) {
		if ((G->nodes = malloc(n*sizeof(struct node))) == NULL) exit(1);
		G->max_nodes = n;
	}
	while (G->num_nodes + n > G->max_nodes) {
		// don't check for NULL, things are looking bad and program will crash anyway if this fails
		G->nodes = realloc(G->nodes, 2*G->max_nodes*sizeof(struct node));
		G->max_nodes <<= 1; // this might go wrong
	}

	while (n --> 0) {
		G->nodes[G->num_nodes] = (struct node) {
			.num_n = 0,
			.max_n = 0,
			.neighbours = NULL};
		G->num_nodes++;
	}
	return G->num_nodes-1;
}

int node_remove_edge(struct node *x, int offset) // n: offset in array of neighbours
{
	int y = x->neighbours[offset].id;
	x->neighbours[offset] = x->neighbours[--(x->num_n)];
	return y;
}

int graph_remove_edge(struct graph *G, int x, int offset)
{
	int y = node_remove_edge(G->nodes+x, offset);
	if (! G->is_directed) {
		offset = 0;
		while (G->nodes[y].neighbours[offset].id != x) ++offset;
		node_remove_edge(G->nodes+y, offset);
	}
	return y;
}

void node_add_neighbour(struct node *x, int y, double weight)
{
	if (x->max_n == 0) {
		x->neighbours = malloc(MAX_NEIGH*sizeof(struct neighbour));
		x->max_n = MAX_NEIGH;
	} else if (x->num_n == x->max_n) {
		x->neighbours = realloc(x->neighbours, 2*x->max_n*sizeof(struct neighbour));
		x->max_n <<= 1; // this might go wrong
	}
	x->neighbours[x->num_n++] = (struct neighbour) {.weight=weight, .id=y};
}

void graph_add_edge(struct graph *G, int start, int end, double weight)
{
	node_add_neighbour(G->nodes+start, end, weight);

	if (!(G->is_directed))
		node_add_neighbour(G->nodes+end, start, weight);
}

void graph_print(struct graph *G)
{
	char ldelim, rdelim;

	if (G->is_directed) {
		printf("Digraph ");
		ldelim = '(';
		rdelim = ')';
	} else {
		printf("Undirected graph ");
		ldelim = '{';
		rdelim = '}';
	}

	printf("with %d vertices.\n", G->num_nodes);
	bool no_edges = true;
	for (int i=0; i<(G->num_nodes); ++i) {
		for (int j=0; j<(G->nodes[i].num_n); ++j) {
			if (no_edges) {
				puts("The edges are:");
				no_edges = false;
			}
			printf("%c%d,%d%c, weight %lf\n", ldelim, i, G->nodes[i].neighbours[j].id, rdelim, G->nodes[i].neighbours[j].weight);
		}
	}
	if (no_edges)
		puts("There are no edges.");
}

struct graph graph_from_file(char const *filename, bool is_directed)
{
	FILE *fp = fopen(filename, "r");
	int num = 0; // number of vertices
	int head, tail;

	char *line = NULL; // line from getline
	char *p; // for parsing line
	size_t len = 0;
	double weight;

	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open file: %s\n", filename);
		exit(1);
	}

	// Get number of vertices
	if (fscanf(fp, "%d\n", &num) != 1) {
		fprintf(stderr, "Error: invalid file format.\n");
		goto error;
	}
	struct graph G = {.num_nodes = 0, .max_nodes = 0, .is_directed=is_directed};
	graph_add_nodes(&G, num);

	// Get edges
	while (getline(&line, &len, fp) != -1) {
		// format: %d %d [%lf]
		p = line;

		// read end points
		head = fast_atoi(&p);
		++p;
		tail = fast_atoi(&p);

		// set weight
		if (*p != ' ' || sscanf(p, " %lf\n", &weight) == 0) {
			weight = 1.0;
		}

		if (tail != head) {
			graph_add_edge(&G, head, tail, weight);
		} else {
			fprintf(stderr, "Error: Invalid file format: Loops not allowed.\n");
			goto error;
		}
	}

	free(line);
	fclose(fp);

	return G;

error:
	fclose(fp);
	exit(1);
}

void graph_free(struct graph *G)
{
	for (int i=0; i<(G->num_nodes); ++i) {
		if (G->nodes[i].max_n > 0)
			free(G->nodes[i].neighbours);
	}
	free(G->nodes);
}

int graph_zus_komp(struct graph *G)
{
	if (G->is_directed) {
		fprintf(stderr, "Error counting ZusKomp: graph is directed!\n");
		exit(1);
	}
	// current root to DFS from
	int cur_node, neighbour_id;
	int r = 0;
	int components = 0;
	char *visited = calloc(G->num_nodes, 1);
	// not nice:
	int *unexplored = malloc(G->num_nodes * sizeof(int));
	int *sp = unexplored; // points to highest element in stack, empty if unexplored-1

	while (r < G->num_nodes) {
		++components;
		*(sp++) = r; // push r
		visited[r] = true;

		while (sp > unexplored) {
			cur_node = *(--sp);
			for (int i=0; i<(G->nodes[cur_node].num_n); ++i) {
				neighbour_id = G->nodes[cur_node].neighbours[i].id;
				if (! visited[neighbour_id]) {
					visited[neighbour_id] = 1;
					*(sp++) = neighbour_id;
				}
			}
		}

		// find new r
		while ((r < G->num_nodes) && visited[r]) ++r;
	}

	free(visited);
	free(unexplored);

	return components;
}
