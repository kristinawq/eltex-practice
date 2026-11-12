CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDLIBS = -lm

all: app

contacts.o: contacts.c contacts.h
	$(CC) $(CFLAGS) -c contacts.c

libcontacts.a: contacts.o
	ar rcs libcontacts.a contacts.o

main.o: main.c contacts.h
	$(CC) $(CFLAGS) -c main.c

app: main.o libcontacts.a
	$(CC) $(CFLAGS) -o app main.o -L. -lcontacts $(LDLIBS)

test_app: test_contacts.o contacts.o unity.o
	$(CC) $(CFLAGS) -o test_app test_contacts.o contacts.o unity.o $(LDLIBS)

test_contacts.o: test_contacts.c contacts.h unity.h
	$(CC) $(CFLAGS) -c test_contacts.c

unity.o: unity.c unity.h unity_internals.h
	$(CC) $(CFLAGS) -c unity.c

run_test: test_app
	./test_app

clean:
	rm -f app test_app *.o *.a

.PHONY: all clean run_test
