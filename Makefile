CC = gcc
CFLAGS = -Wall -Wextra -Ofast -pedantic -std=c11
#CFLAGS = -Wall -Wextra -g -O0 -pedantic -std=c11
DEPS = $(BIN) $(BUILD) Makefile

BIN = bin
BUILD = obj

$(BUILD)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

menger: $(BUILD)/menger.o $(BUILD)/graph.o $(DEPS)
	$(CC) $(CFLAGS) $(BUILD)/menger.o $(BUILD)/graph.o -o $(BIN)/menger

bfs: $(BUILD)/bfs.o $(BUILD)/graph.o $(BUILD)/graph_alg.o $(BUILD)/fib_heap.o $(DEPS)
	$(CC) $(CFLAGS) $(BUILD)/bfs.o $(BUILD)/graph.o $(BUILD)/graph_alg.o $(BUILD)/fib_heap.o -o $(BIN)/bfs

mst: $(BUILD)/mst.o $(BUILD)/graph.o $(BUILD)/graph_alg.o $(BUILD)/fib_heap.o graph.h graph_alg.h $(DEPS)
	$(CC) $(CFLAGS) $(BUILD)/mst.o $(BUILD)/graph.o $(BUILD)/graph_alg.o $(BUILD)/fib_heap.o -o $(BIN)/mst

Eulerweg: Eulerweg.c graph.c graph.h path.c path.h euler.c euler.h $(DEPS)
	$(CC) $(CFLAGS) path.c euler.c graph.c Eulerweg.c -o $(BIN)/Eulerweg

heapsort: heapsort.c fib_heap.h fib_heap.c $(DEPS)
	$(CC) $(CFLAGS) fib_heap.c heapsort.c -o $(BIN)/heapsort

heap_test: heap_test.c fib_heap.h fib_heap.c $(DEPS)
	$(CC) $(CFLAGS) fib_heap.c heap_test.c -o $(BIN)/heap_test

$(BIN):
	mkdir $(BIN)

$(BUILD):
	mkdir $(BUILD)

.PHONY: clean bin_clean
clean:
	rm -rf "$(BUILD)"
bin_clean:
	rm -f "$(BIN)/*"
