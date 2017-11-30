#define _POSIX_C_SOURCE 201112L

#include <err.h>
#include <stdlib.h>
#include <stdio.h>

#include "digraph.h"

#include "config.h"

static void vert_add_edge(int *deg, int *max_deg, int **edges, int edge_id);

static void check_null(void *pointer)
{
	if (pointer == NULL)
		err(1, "Error: go buy more RAM!\n");
}

static int fast_atoi(char **str)
{
	int val = 0;
	while ( !(**str == ' ' || **str == '\n' || **str == '\0')) {
		val = val*10 + (*(*str)++ - '0');
	}
	return val;
}

// returns last vert in G
int graph_add_nodes(struct graph *G, int num)
{
	if (num <= 0) return -1;
	if (G->_max_n == 0) {
		G->V = malloc(num * sizeof(struct vert));
		G->_max_n = num;
		check_null(G->V);
	} else while (G->n + num > G->_max_n) {
		G->V = realloc(G->V, 2*G->_max_n*sizeof(struct vert));
		G->_max_n <<= 1;
		check_null(G->V);
	}
	// TODO: be a bit more intelligent up there

	while (num  --> 0) {
		G->V[G->n] = (struct vert) {
			.id = G->n,
			.d_plus = 0, .d_minus = 0, ._max_plus = 0, ._max_minus = 0,
			.to = NULL, .from = NULL };
		(G->n)++;
	}

	return G->n - 1;
}

static void vert_add_edge(int *deg, int *max_deg, int **edges, int edge_id)
{
	if (*max_deg == 0) {
		*edges = malloc(sizeof(struct edge));
		*max_deg = 1;
		check_null(*edges);
	} else if (*deg >= *max_deg) {
		*edges = realloc(*edges, *max_deg*2*sizeof(struct edge));
		*max_deg <<= 1;
		check_null(*edges);
	}
	(*edges)[*deg++] = edge_id;
}

int graph_add_edge(struct graph *G, int x, int y, long weight)
{
	// first add edge to graph
	if (G->_max_m == 0) {
		G->E = malloc(sizeof(struct edge));
		G->_max_m = 1;
		check_null(G->E);
	} else if (G->m >= G->_max_m) {
		G->E = realloc(G->E, 2*G->_max_m*sizeof(struct edge));
		G->_max_m <<= 1;
		check_null(G->E);
	}
	int edge_id = G->m++;

	G->E[edge_id] = (struct edge) {.id = edge_id, .x = x, .y = y, .weight = weight};

	// then to end-points
	vert_add_edge(&(G->V[x].d_plus), &(G->V[x]._max_plus), &(G->V[x].to), edge_id);
	vert_add_edge(&(G->V[y].d_minus), &(G->V[y]._max_minus), &(G->V[y].from), edge_id);

	return edge_id;
}

void graph_free(struct graph *G)
{
	// free delta_minus, delta_plus for each vert
	for (int i=0; i<G->n; ++i) {
		if (G->V[i]._max_plus > 0)
			free(G->V[i].to);
		if (G->V[i]._max_minus > 0)
			free(G->V[i].from);
	}
	// free list of verts, edges
	if (G->n > 0) free(G->V);
	if (G->m > 0) free(G->E);

	return;
}

void graph_print(struct graph *G)
{
	setvbuf(stdout, NULL, _IOFBF, 0);
	printf("n = %d, m = %d\n", G->n, G->m);

	if (G->m == 0) {
		printf("There are no edges.\n");
	} else {
		printf("Edges:\n");
		for (int e=0; e<G->m; ++e) {
			printf("(%d, %d) weight %ld\n", G->E[e].x, G->E[e].y, G->E[e].weight);
		}
	}
}

struct graph graph_from_file(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	int n = 0;
	int x, y;
	long weight;

	char *line = NULL;
	char *p;
	size_t len = 0;

	if (fp == NULL)
		err(1, "Error: cannot open file: %s\n", filename);

	if (fscanf(fp, "%d\n", &n) != 1)
		err(1, "Error: invalid file format.\n");

	struct graph G = {.n=0, .m=0, ._max_n=0, ._max_m=0, .E=NULL, .V=NULL};
	graph_add_nodes(&G, n);

	// Get edges
	while (getline(&line, &len, fp) != -1) {
		p = line;
		x = fast_atoi(&p);
		++p;
		y = fast_atoi(&p);

		if (*p != ' ' || sscanf(p, " %ld\n", &weight) == 0)
			weight = 1;

		if (x != y)
			graph_add_edge(&G, x, y, weight);
		else
			err(1, "Error: invalied file format: loops not allowed.\n");
	}

	free(line);
	fclose(fp);

	return G;
}
