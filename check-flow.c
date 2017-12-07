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
		printf("Usage: %s [-m] filename [s t]\nDefault: s=0, t=1.\n", argv[0]);
		printf("Options: -m: Merge parallel edges for better asymptotic runtime.\n");
		return 1;
	}

	struct graph G = graph_from_file(argv[1]);
	if (s >= G.n || t >= G.n) {
		fprintf(stderr, "Error: invalid s or t.\n");
		return 1;
	}

	long *ex = calloc(G.n, sizeof(long));
	long val = 0;

	if (scanf("%ld\n", &val) != 1) return 1;
	int e;
	long f;

	while (scanf("%d %ld\n", &e, &f) == 2) {
		if (f < 0) {
			printf("Error: f < 0\n");
		}
		if (f > G.E[e].weight) {
			printf("Error: f(%d) = %ld > u(%d) = %ld\n", e, f, e, G.E[e].weight);
		}
		ex[G.E[e].x] -= f;
		ex[G.E[e].y] += f;
	}

	long sum = 0;
	for (int i=0; i<G.n; ++i) {
		sum += ex[i];
	}
	if (sum != 0) printf("Error: unbalanced balance.\n");
	printf("Überschuss: {%ld", ex[0]);
	for (int i=1; i<G.n; ++i) printf(", %ld", ex[i]);
	printf("}\n");
	if (ex[t] != -ex[s]) {
		printf("Error: ex[s] != -ex[t]\n");
	}
	if (val != ex[t]) {
		printf("Error: bad val=%ld!\n", val);
	}
}
