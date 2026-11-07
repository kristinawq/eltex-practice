CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -DUNITY_INCLUDE_DOUBLE
LDLIBS = 

all: app

app: main.o contacts.o
	$(CC) $(CFLAGS) -o app main.o contacts.o $(LDLIBS)

main.o: main.c contacts.h
	$(CC) $(CFLAGS) -c main.c

contacts.o: contacts.c contacts.h
	$(CC) $(CFLAGS) -c contacts.c

test: test_app

test_app: test_contacts.o contacts.o unity.o
	$(CC) $(CFLAGS) -o test_app test_contacts.o contacts.o unity.o $(LDLIBS)

test_contacts.o: test_contacts.c contacts.h unity.h
	$(CC) $(CFLAGS) -c test_contacts.c

unity.o: unity.c unity.h unity_internals.h
	$(CC) $(CFLAGS) -c unity.c

run_test: test
	./test_app

clean:
	rm -f app test_app *.o

.PHONY: all test run_test clean
