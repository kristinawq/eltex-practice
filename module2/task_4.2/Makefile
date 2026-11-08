CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -DUNITY_INCLUDE_DOUBLE
LDLIBS = 

all: app

app: main.o queue.o
	$(CC) $(CFLAGS) -o app main.o queue.o $(LDLIBS)

main.o: main.c queue.h
	$(CC) $(CFLAGS) -c main.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

test: test_app

test_app: test_queue.o queue.o unity.o
	$(CC) $(CFLAGS) -o test_app test_queue.o queue.o unity.o $(LDLIBS)

test_queue.o: test_queue.c queue.h unity.h
	$(CC) $(CFLAGS) -c test_queue.c

unity.o: unity.c unity.h unity_internals.h
	$(CC) $(CFLAGS) -c unity.c

run_test: test
	./test_app

clean:
	rm -f app test_app *.o

.PHONY: all test run_test clean
