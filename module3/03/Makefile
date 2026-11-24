CC      = gcc
CFLAGS  = -std=gnu11 -Wall -Wextra -pedantic -pthread
LDLIBS  = -lrt -pthread

UNITY   = unity/unity.c
BIN     = p2p_chat
TESTS   = tests/test_chat

.PHONY: all test clean

all: $(BIN)

$(BIN): main.c chat.c chat.h
	$(CC) $(CFLAGS) main.c chat.c -o $@ $(LDLIBS)

tests/test_chat: tests/test_chat.c chat.c chat.h $(UNITY)
	$(CC) $(CFLAGS) -Iunity -I. tests/test_chat.c chat.c $(UNITY) -o $@ $(LDLIBS)

test: tests/test_chat
	./tests/test_chat

clean:
	rm -f $(BIN) $(TESTS)
