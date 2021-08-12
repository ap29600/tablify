CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pedantic -O3
LFLAGS = -x c -fPIC -c
BINARY_PATH = ~/.local/bin

default: bin/tablify

clean:
	$(RM) bin/tablify obj/*

install: bin/tablify
	cp bin/tablify $(BINARY_PATH)/tablify

bin/tablify: obj/tablify.o obj/libargs.so obj/libstringview.so
	$(CC) $(CFLAGS) obj/tablify.o -L./obj -lstringview -largs -o bin/tablify 

obj/tablify.o: src/tablify.c src/tablify.h src/table.c
	$(CC) $(CFLAGS) -c src/tablify.c -o obj/tablify.o

obj/libargs.so: lib/args.h
	$(CC) $(CFLAGS) $(LFLAGS) -DARGS_IMPLEMENTATION lib/args.h -o obj/libargs.so

obj/libstringview.so: lib/stringview.h
	$(CC) $(CFLAGS) $(LFLAGS) -DSTRINGVIEW_IMPLEMENTATION lib/stringview.h -o obj/libstringview.so

