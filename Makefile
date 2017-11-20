CC = gcc
CFLAGS = -Wall -Wextra -Ofast -pedantic -std=c11
#CFLAGS = -Wall -Wextra -g -O0 -pedantic -std=c11

DEPS = config.h euler.h fib_heap.h graph_alg.h graph.h path.h
BIN = bin
BUILD = build

$(BUILD)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bfs: bfs.o graph.o graph_alg.o $(BIN)
	$(CC) -o $(BIN)/$@ bfs.o graph.o graph_alg.o $(CFLAGS)

mst: mst.o graph.o graph_alg.o $(BIN)
	$(CC) -o $(BIN)/$@ mst.o graph.o graph_alg.o $(CFLAGS)

Eulerweg: Eulerweg.o graph.o path.o euler.o $(BIN)
	$(CC) -o $(BIN)/$@ Eulerweg.o graph.o path.o euler.o $(CFLAGS)

heapsort: heapsort.o fib_heap.o $(BIN)
	$(CC) -o $(BIN)/$@ heapsort.o fib_heap.o $(CFLAGS)

heap_test: heap_test.o fib_heap.o $(BIN)
	$(CC) -o $(BIN)/$@ heap_test.o fib_heap.o $(CFLAGS)

$(BIN):
	mkdir $(BIN)

.PHONY: clean clean_bin
clean:
	rm -f $(BUILD)/*.o

clean_bin:
	rm -rf ./bin
