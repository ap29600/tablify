# Tablify

## A (not so) simple table formatter

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

- [ ] support for multiline entries.
- [X] support for table alignment with `|---:|:----|---|` syntax.
    - [X] basic support
    - [ ] support for multiple alignments in the same column
- [ ] clean up the parsing loop.
- [X] Add support for calculations in python.
    - [ ] find a way to stitch the result back into the table, without
          sacrificing stability (running the program on its own output 
          should not change anything).

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

## Quick start

The `install` recipe will build the program and install it into `~/.local/bin`. You can specify
where to put the executable with the `BINARY_PATH` variable.

```
$ make clean install
```

## Usage
(A random excuse to insert a table in this readme)

The program supports a few CLI options:

|           Flag |  Type  | Description                                                                                                    |
|---------------:|--------|:---------------------------------------------------------------------------------------------------------------|
|      `--delim` |  Char  | The character that separates columns. Default is `'\|'`.                                                       |
|    `--compute` |  Int   | Whether or not to compute the python expressions in the cells. Default is `0`.                                 |
|      `--input` | String | The input file to read the table from. If no file is provided, input is read from `stdin`.                     |
|     `--ignore` | String | Characters that are allowed to be on the separator line without invalidating it. Defaults are `'/'` and `':'`. |
| `--separators` | String | Characters that can be used to construct a separator line. Defaults are `'-'` and `'='`.                       |




