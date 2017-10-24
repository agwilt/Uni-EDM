#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

long fast_atol(char **str)
{
	long val = 0;
	while ( !(**str == ' ' || **str == '\n' || **str == '\0')) {
		val = val*10 + (*(*str)++ - '0');
	}
	return val;
}

node_id graph_add_nodes(struct graph *G, node_id n)
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

node_id node_remove_edge(struct node *x, node_id offset) // n: offset in array of neighbours
{
	node_id y = x->neighbours[offset].id;
	x->neighbours[offset] = x->neighbours[--(x->num_n)];
	return y;
}

node_id graph_remove_edge(struct graph *G, node_id x, node_id offset)
{
	node_id y = node_remove_edge(G->nodes+x, offset);
	if (! G->is_directed) {
		offset = 0;
		while (G->nodes[y].neighbours[offset].id != x) ++offset;
		node_remove_edge(G->nodes+y, offset);
	}
	return y;
}

void node_add_neighbour(struct node *x, node_id y, double weight)
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

void graph_add_edge(struct graph *G, node_id start, node_id end, double weight)
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

	printf("with %ld vertices.\n", G->num_nodes);
	bool no_edges = true;
	for (node_id i=0; i<(G->num_nodes); ++i) {
		for (node_id j=0; j<(G->nodes[i].num_n); ++j) {
			if (no_edges) {
				puts("The edges are:");
				no_edges = false;
			}
			printf("%c%ld,%ld%c, weight %lf\n", ldelim, i, G->nodes[i].neighbours[j].id, rdelim, G->nodes[i].neighbours[j].weight);
		}
	}
	if (no_edges)
		puts("There are no edges.");
}

struct graph graph_from_file(char const *filename, bool is_directed)
{
	FILE *fp = fopen(filename, "r");
	node_id num = 0; // number of vertices
	node_id head, tail;

	char *line = NULL; // line from getline
	char *p; // for parsing line
	size_t len = 0;
	double weight;

	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open file: %s\n", filename);
		exit(1);
	}

	// Get number of vertices
	if (fscanf(fp, "%ld\n", &num) != 1) {
		fprintf(stderr, "Error: invalid file format.\n");
		goto error;
	}
	struct graph G = {.num_nodes = 0, .max_nodes = 0, .is_directed=is_directed};
	graph_add_nodes(&G, num);

	// Get edges
	while (getline(&line, &len, fp) != -1) {
		// format: %ld %ld [%lf]
		p = line;

		head = fast_atol(&p);
		++p;
		tail = fast_atol(&p);

		weight = 1.0;
		/*
		if (*p == ' ')
			weight = atof(p+1);
		*/
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
	node_id cur_node, neighbour_id;
	node_id r = 0;
	int components = 0;
	char *visited = calloc(G->num_nodes, 1);
	// not nice:
	node_id *unexplored = malloc(G->num_nodes * sizeof(node_id));
	node_id *sp = unexplored; // points to highest element in stack, empty if unexplored-1

	while (r < G->num_nodes) {
		++components;
		*(sp++) = r; // push r
		visited[r] = true;

		while (sp > unexplored) {
			cur_node = *(--sp);
			for (node_id i=0; i<(G->nodes[cur_node].num_n); ++i) {
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

	return components;
}
