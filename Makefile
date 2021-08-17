CC = gcc
CFLAGS = -Wall -Wmissing-prototypes -Wextra -std=c11 -pedantic -O3
LFLAGS = -x c -fPIC -c
BINARY_PATH = ~/.local/bin

default: bin/tablify

clean:
	$(RM) bin/tablify obj/*

install: bin/tablify
	cp bin/tablify $(BINARY_PATH)/tablify

obj:
	mkdir -p obj

bin:
	mkdir -p bin

bin/tablify: obj/tablify.o obj/libargs.so obj/libstringview.so bin
	$(CC) $(CFLAGS) obj/tablify.o -L./obj -lstringview -largs -o bin/tablify 

obj/tablify.o: src/tablify.c src/tablify.h src/io.h obj
	$(CC) $(CFLAGS) -c src/tablify.c -o obj/tablify.o

obj/libargs.so: lib/args.h obj
	$(CC) $(CFLAGS) $(LFLAGS) -DARGS_IMPLEMENTATION lib/args.h -o obj/libargs.so

obj/libstringview.so: lib/stringview.h obj
	$(CC) $(CFLAGS) $(LFLAGS) -DSTRINGVIEW_IMPLEMENTATION lib/stringview.h -o obj/libstringview.so

