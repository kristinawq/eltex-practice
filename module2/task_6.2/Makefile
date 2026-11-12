CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC
LDLIBS = -lm

all: app

contacts.o: contacts.c contacts.h
	$(CC) $(CFLAGS) -c contacts.c

libcontacts.so: contacts.o
	$(CC) -shared -o libcontacts.so contacts.o

main.o: main.c contacts.h
	$(CC) -Wall -Wextra -std=c99 -c main.c

app: main.o libcontacts.so
	$(CC) -o app main.o -L. -lcontacts -Wl,-rpath,. $(LDLIBS)

test_app: test_contacts.o unity.o libcontacts.so
	$(CC) -o test_app test_contacts.o unity.o -L. -lcontacts -Wl,-rpath,. $(LDLIBS)

test_contacts.o: test_contacts.c contacts.h unity.h
	$(CC) $(CFLAGS) -c test_contacts.c

unity.o: unity.c unity.h unity_internals.h
	$(CC) $(CFLAGS) -c unity.c

run_test: test_app
	./test_app

clean:
	rm -f app test_app *.o *.so

.PHONY: all clean run_test
