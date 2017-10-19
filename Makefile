CC = gcc
CFLAGS = -Wall -Wextra -g -pedantic -std=c11

EULERWEG = Eulerweg

$(EULERWEG): $(EULERWEG).c graph.c graph.h path.c path.h euler.c euler.h Makefile
	$(CC) $(CFLAGS) path.c euler.c graph.c $(EULERWEG).c -o $(EULERWEG)

clean:
	rm -f $(EULERWEG)
