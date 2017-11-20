CC = gcc
CFLAGS = -Wall -Wextra -Ofast -pedantic -std=c11
#CFLAGS = -Wall -Wextra -g -O0 -pedantic -std=c11

BIN = bin

mst: mst.c graph.c graph.h graph_alg.c graph_alg.h Makefile
	$(CC) $(CFLAGS) fib_heap.c graph.c graph_alg.c mst.c -o $(BIN)/mst

Eulerweg: Eulerweg.c graph.c graph.h path.c path.h euler.c euler.h Makefile
	$(CC) $(CFLAGS) path.c euler.c graph.c Eulerweg.c -o $(BIN)/Eulerweg

heapsort: heapsort.c fib_heap.h fib_heap.c Makefile
	$(CC) $(CFLAGS) fib_heap.c heapsort.c -o $(BIN)/heapsort

heap_test: heap_test.c fib_heap.h fib_heap.c Makefile
	$(CC) $(CFLAGS) fib_heap.c heap_test.c -o $(BIN)/heap_test

$(BIN):
	mkdir $(BIN)

.PHONY: clean
clean:
	rm -f $(BIN)/*
