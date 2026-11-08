CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -DUNITY_INCLUDE_DOUBLE

all: app

app: main.o logic.o
	$(CC) $(CFLAGS) -o app main.o logic.o

main.o: main.c logic.h
	$(CC) $(CFLAGS) -c main.c

logic.o: logic.c logic.h
	$(CC) $(CFLAGS) -c logic.c

test: test_app

test_app: test_logic.o logic.o unity.o
	$(CC) $(CFLAGS) -o test_app test_logic.o logic.o unity.o

test_logic.o: test_logic.c logic.h unity.h
	$(CC) $(CFLAGS) -c test_logic.c

unity.o: unity.c unity.h unity_internals.h
	$(CC) $(CFLAGS) -c unity.c

run_test: test
	./test_app

clean:
	rm -f app test_app *.o

.PHONY: all test run_test clean
