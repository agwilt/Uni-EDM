#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc<2) {
		printf("Usage: ./makegraph n\n");
		return 1;
	}

	int n = atoi(argv[1]);
	unsigned int w = 1;

	printf("%d\n", n);
	for (int i=0; i<n; ++i) {
		for (int j=0; j<n; ++j) {
			if (i!=j)
				printf("%d %d %u\n", i, j, w);
			++w;
		}
	}

	return 0;
}
