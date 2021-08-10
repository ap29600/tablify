#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"

#define CFLAGS "-Wall", "-Wextra", "-std=c11", "-pedantic", "-Os"
#define LFLAGS "-x", "c", "-fPIC", "-c"

int contains(int size, char **haystack, char *needle) {
  for (int i = 0; i < size; i++)
    if (!strcmp(haystack[i], needle))
      return 1;
  return 0;
}

int main(int argc, char **argv) {
  GO_REBUILD_URSELF(argc, argv);

  if (argc > 1 && contains(argc - 1, argv + 1, "libs")) {
    // generate library shared object
    CMD("cc", CFLAGS, LFLAGS, "-DSTRINGVIEW_IMPLEMENTATION",
        "stringview.h", "-o", "libstringview.so");
  }

  // generate main object
  CMD("cc", CFLAGS, "-c", "tablify.c", "-o", "tablify.o");
  // link objects together
  CMD("cc", CFLAGS, "tablify.o", "-L.", "-lstringview", "-o", "tablify");

  if (argc > 1 && contains(argc - 1, argv + 1, "test")) {
    // run the program on input.txt
    CMD("sh", "-c", "cat ./input.txt | ./tablify");
  }

  return 0;
}
