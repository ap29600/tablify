# Tablify

# Well, that's awkward!
There is already a vim plugin that works very similarly
to this program and is called tablify. I will be changing the
name as soon as I can think of one.

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
    - [X] find a way to stitch the result back into the table, without
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

### Default behaviour
By default `tablify` will read the table from `stdin` and output it to `stdout` formatted.
Columns are separated by the `'|'` character, and any row containing only whitespace, `'|'`, `'/'`, `':'` and
exactly one of the characters `'-'`and `'='` (this character may repeat, but both are not allowed at the same time)
is interpreted as a separator line and filled with either `'-'` or `'='` depending on wich is found.
Within a separator line it is possible to choose left, right or center alignment respectively by placing a `':'` character either before
the separators, after them or by omitting it:

```
| Left aligned cell | center aligned cell | Right aligned cell |
|:------------------|---------------------|-------------------:|
| left              |       center        |              right |
```

### With the `--compute 1` flag
if the compute flag is provided, cells whose content starts with `=` (whitesapce excluded) are interpreted as expressions
and can contain python expressions, which will be evaluated and added at the end of the cell as a python comment:

For example the following table:

```
| A | B | C |
| - 
| 123 | =A1 | = A1 +  0.5 * B1 |
```

Will be turned into:
```
|  A  |     B     |            C            |
|-----|-----------|-------------------------|
| 123 | =A1 # 123 | = A1 + 0.5 * B1 # 184.5 |
```

the nice thing about this is that any python object is also allowed: here's an example with lists.
```
|     A     |   B    |              C              |
|-----------|--------|-----------------------------|
| [1, 2, 3] | [4, 5] | = A1 + B1 # [1, 2, 3, 4, 5] |
```

And of course the `math` library is available as `m`: 

```
|              A               |        B         |
|------------------------------|------------------|
| =m.exp(2) # 7.38905609893065 | =m.log(A1) # 2.0 |
```

### Flags
(A random excuse to insert a table in this readme)

|           Flag |  Type  | Description                                                                                                    |
|---------------:|--------|:---------------------------------------------------------------------------------------------------------------|
|      `--delim` |  Char  | The character that separates columns. Default is `'\|'`.                                                       |
|    `--compute` |  Int   | Whether or not to compute the python expressions in the cells. Default is `0`.                                 |
|      `--input` | String | The input file to read the table from. If no file is provided, input is read from `stdin`.                     |
|     `--ignore` | String | Characters that are allowed to be on the separator line without invalidating it. Defaults are `'/'` and `':'`. |
| `--separators` | String | Characters that can be used to construct a separator line. Defaults are `'-'` and `'='`.                       |



