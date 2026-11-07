CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -DUNITY_INCLUDE_DOUBLE
LDLIBS = -lm

all: calc

calc: main.o calc.o
	$(CC) $(CFLAGS) -o calc main.o calc.o $(LDLIBS)

main.o: main.c calc.h
	$(CC) $(CFLAGS) -c main.c

calc.o: calc.c calc.h
	$(CC) $(CFLAGS) -c calc.c

test: test_calc

test_calc: calc.o test_calc.o unity.o
	$(CC) $(CFLAGS) -o test_calc calc.o test_calc.o unity.o $(LDLIBS)

test_calc.o: test_calc.c calc.h unity.h
	$(CC) $(CFLAGS) -c test_calc.c

unity.o: unity.c unity.h unity_internals.h
	$(CC) $(CFLAGS) -c unity.c

run_test: test
	./test_calc

clean:
	rm -f calc test_calc *.o

.PHONY: all test run_test clean
