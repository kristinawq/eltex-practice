CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -DUNITY_INCLUDE_DOUBLE
LDLIBS = 

all: app

app: main.o tree.o
	$(CC) $(CFLAGS) -o app main.o tree.o $(LDLIBS)

main.o: main.c tree.h
	$(CC) $(CFLAGS) -c main.c

tree.o: tree.c tree.h
	$(CC) $(CFLAGS) -c tree.c

test: test_app

test_app: test_tree.o tree.o unity.o
	$(CC) $(CFLAGS) -o test_app test_tree.o tree.o unity.o $(LDLIBS)

test_tree.o: test_tree.c tree.h unity.h
	$(CC) $(CFLAGS) -c test_tree.c

unity.o: unity.c unity.h unity_internals.h
	$(CC) $(CFLAGS) -c unity.c

run_test: test
	./test_app

clean:
	rm -f app test_app *.o

.PHONY: all test run_test clean
