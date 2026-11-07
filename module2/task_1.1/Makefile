CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -DUNITY_INCLUDE_DOUBLE

all: phonebook

phonebook: main.c contacts.c
	$(CC) $(CFLAGS) main.c contacts.c -o phonebook

test: test_app

test_app: test_contacts.o contacts.o unity.o
	$(CC) $(CFLAGS) -o test_app test_contacts.o contacts.o unity.o

test_contacts.o: test_contacts.c contacts.h unity.h
	$(CC) $(CFLAGS) -c test_contacts.c

unity.o: unity.c unity.h unity_internals.h
	$(CC) $(CFLAGS) -c unity.c

run_test: test
	./test_app

clean:
	rm -f phonebook test_app *.o

.PHONY: all test run_test clean
