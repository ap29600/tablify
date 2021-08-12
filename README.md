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

- [ ] support for multiline entries.
- [X] support for table alignment with `|---:|:----|---|` syntax.
    - [X] basic support
    - [ ] support for multiple alignments in the same column
- [ ] clean up the parsing loop.
- [ ] support some form of calculation similar to [minicel](https://github.com/tsoding/minicel)
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
|      `--input` |  path  | The input file to read the table from. If no file is provided, input is read from `stdin`.                     |
|      `--delim` |  char  | The character that separates columns. Default is `'|'`.                                                        |
|     `--ignore` | string | characters that are allowed to be on the separator line without invalidating it. Defaults are `'\'` and `':'`. |
| `--separators` | string | characters that can be used to construct a separator line. Defaults are `'-'` and `'='`.                       |


