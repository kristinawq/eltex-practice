CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC

all: app

contacts.o: contacts.c contacts.h
	$(CC) $(CFLAGS) -c contacts.c

libcontacts.so: contacts.o
	$(CC) -shared -o libcontacts.so contacts.o

main.o: main.c contacts.h
	$(CC) -Wall -Wextra -std=c99 -c main.c

app: main.o libcontacts.so
	$(CC) -o app main.o -L. -lcontacts

clean:
	rm -f app *.o *.so

.PHONY: all clean
