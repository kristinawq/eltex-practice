CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -I.
LDFLAGS  =

OBJ      = copy_files.o
MAIN_OBJ = main.o

TARGET   = copy_files
TEST_BIN = test_copy_files

UNITY_SRC = tests/unity.c

.PHONY: all run_test clean

all: $(TARGET)

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

copy_files.o: copy_files.c copy_files.h
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c copy_files.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(OBJ) tests/test_copy_files.c $(UNITY_SRC)
	$(CC) $(CFLAGS) -Itests -o $@ \
		tests/test_copy_files.c $(OBJ) $(UNITY_SRC) $(LDFLAGS)

run_test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(OBJ) $(MAIN_OBJ) $(TARGET) $(TEST_BIN)
