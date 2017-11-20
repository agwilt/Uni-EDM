CC = gcc
CFLAGS = -Wall -Wextra -Ofast -pedantic -std=c11
#CFLAGS = -Wall -Wextra -g -O0 -pedantic -std=c11

DEPS = config.h euler.h fib_heap.h graph_alg.h graph.h path.h
BIN = bin

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bfs: bfs.o graph.o graph_alg.o $(BIN)
	$(CC) -o $(BIN)/$@ $^ $(CFLAGS)

mst: mst.o graph.o graph_alg.o $(BIN)
	$(CC) -o $(BIN)/$@ $^ $(CFLAGS)

Eulerweg: Eulerweg.o graph.o path.o euler.o $(BIN)
	$(CC) -o $(BIN)/$@ $^ $(CFLAGS)

heapsort: heapsort.o fib_heap.o $(BIN)
	$(CC) -o $(BIN)/$@ $^ $(CFLAGS)

heap_test: heap_test.o fib_heap.o $(BIN)
	$(CC) -o $(BIN)/$@ $^ $(CFLAGS)

$(BIN):
	mkdir $(BIN)

.PHONY: clean
clean:
	rm -f *.o
