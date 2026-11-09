CC     = gcc
CFLAGS = -Wall -Wextra -std=c99
LDLIBS = -lm -ldl

all: app plugins

app: main.o calc.o
	$(CC) $(CFLAGS) -rdynamic -o app main.o calc.o $(LDLIBS)

main.o: main.c calc.h
	$(CC) $(CFLAGS) -c main.c

calc.o: calc.c calc.h
	$(CC) $(CFLAGS) -c calc.c

plugins: plugins/add.so plugins/sub.so plugins/mul.so plugins/div.so plugins/pow.so plugins/mod.so plugins/sqrt.so

plugins/%.so: plugins/%.c calc.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $< $(LDLIBS)

clean:
	rm -f app *.o plugins/*.so

.PHONY: all clean plugins
