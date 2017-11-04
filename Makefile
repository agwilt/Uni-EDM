CC = gcc
#CFLAGS = -Wall -Wextra -Ofast -pedantic -std=c11
CFLAGS = -Wall -Wextra -g -O0 -pedantic -std=c11

EULERWEG = Eulerweg

$(EULERWEG): $(EULERWEG).c graph.c graph.h path.c path.h euler.c euler.h Makefile
	$(CC) $(CFLAGS) fib_heap.c path.c euler.c graph.c $(EULERWEG).c -o $(EULERWEG)

heapsort: heapsort.c fib_heap.h fib_heap.c Makefile
	$(CC) $(CFLAGS) fib_heap.c heapsort.c -o heapsort

heap_test: heap_test.c fib_heap.h fib_heap.c Makefile
	$(CC) $(CFLAGS) fib_heap.c heap_test.c -o heap_test

clean:
	rm -f $(EULERWEG) heapsort heap_test
