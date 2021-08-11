# Tablify

## A simple table formatter

The idea is to have a simple tool to get a badly written markdown-style table:

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
// |      |  10  | 3.14 |
// |------|------|------|
// |  20  |      |      |

```

This can be integrated into an editor like vim: for example `vip:!tablify<CR>`
will send the current paragraph (as in a sequence of non-empty lines containing
the cursor) to `tablify` and include the output in their place.

It's also useful that anything before the start of the table is left untouched
(only preceding whitespace is removed), so that it is possible to use this tool
on tables in code comments.

## Todo:

- [] support for multiline entries.
- [] support for table alignment with `|---:|:----|---|` syntax.
- [] clean up the parsing loop.
- [] support some form of calculation similar to [minicel](https://github.com/tsoding/minicel)
     (maybe use python or js as the backend?).

## Motivation

This is an exercise in programming inspired by Tsoding's series on
[Minicel](https://github.com/tsoding/minicel), a CLI version of excel that
operates on `csv` files.  The string-view library is also similar to Tsoding's,
but it was completely written from scratch based on the interface I needed.

another reason to implement this is that it brings some of the comfort of org-mode
to vim while retaining the "do one thing and do it well" approach to the problem.

It would be nice to make a few of these tools that add more of org's functionality,
to make markdown on vim as appealing as org-mode on emacs (all while keeping the 
complexity in standalone executables).

## build system

Tablify uses [`nobuild`](https://github.com/tsoding/nobuild), a header-only
library for building programs with only a `C` compiler. Just run:

```
$ cc nobuild.c -o nobuild
$ ./nobuild libs
```

