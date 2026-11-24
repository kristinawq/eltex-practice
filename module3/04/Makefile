CC       = gcc
CFLAGS   = -std=gnu11 -Wall -Wextra -pedantic

BIN      = prodcons
LIB_SRC  = producer_consumer.c
UNITY    = unity/unity.c
TEST_SRC = $(wildcard tests/*.c)
TEST_BIN = tests/run_tests

.PHONY: all test clean

all: $(BIN)

$(BIN): main.c $(LIB_SRC) producer_consumer.h
	$(CC) $(CFLAGS) main.c $(LIB_SRC) -o $@

$(TEST_BIN): $(TEST_SRC) $(LIB_SRC) producer_consumer.h $(UNITY)
	$(CC) $(CFLAGS) -Iunity -I. $(TEST_SRC) $(LIB_SRC) $(UNITY) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(BIN) $(TEST_BIN)
