CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: app

contacts.o: contacts.c contacts.h
	$(CC) $(CFLAGS) -c contacts.c

libcontacts.a: contacts.o
	ar rcs libcontacts.a contacts.o

main.o: main.c contacts.h
	$(CC) $(CFLAGS) -c main.c

app: main.o libcontacts.a
	$(CC) $(CFLAGS) -o app main.o -L. -lcontacts

clean:
	rm -f app *.o *.a

.PHONY: all clean
