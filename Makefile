CC = gcc
CFLAGS = -Wall -Wextra -g -pedantic -O2 -std=c11

TEST = graph_test

$(TEST): $(TEST).c graph.c graph.h path.c path.h euler.c euler.h Makefile
	$(CC) $(CFLAGS) path.c euler.c graph.c $(TEST).c -o $(TEST)

clean:
	rm -f $(TEST)
