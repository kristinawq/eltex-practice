CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: phonebook

phonebook: main.c contacts.c
	$(CC) $(CFLAGS) main.c contacts.c -o phonebook

clean:
	rm -f phonebook *.o

.PHONY: all clean
