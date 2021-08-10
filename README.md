# Tablify

## A simple table formatter

The idea is to have a simple tool to get a table in the format:

```

// |col1| col2| col3 |
// |=
// || 10 | 3.14
// |-
//  |20

```

and automatically format it nicely to something like:

```

 // | col1 | col2 | col3 |
 // |======|======|======|
 // |      | 10   | 3.14 |
 // |------|------|------|
 // | 20   |      |      |

```

This can be integrated into an editor like vim: for example `vip:!tablify<CR>`
will send the current paragraph (as in a sequence of non-empty lines containing
the cursor) to `tablify` and include the output in their place.

## build system:

tablify uses (`nobuild`)[https://github.com/tsoding/nobuild], a header-only library
for building programs with only a `C` compiler. Just run:

```
cc nobuild.c -o nobuild
./nobuild libs
```

