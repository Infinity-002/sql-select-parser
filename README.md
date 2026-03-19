# SQL SELECT Parser in C

This project implements a small compiler-style front end for a subset of SQL `SELECT` statements.

## Supported grammar

```text
query          -> SELECT select_list FROM identifier where_clause? ;
select_list    -> * | identifier ( , identifier )*
where_clause   -> WHERE expression
expression     -> or_expr
or_expr        -> and_expr ( OR and_expr )*
and_expr       -> primary ( AND primary )*
primary        -> comparison | ( expression )
comparison     -> value comp_op value
value          -> identifier | number | string
comp_op        -> = | != | < | > | <= | >=
```

## Project layout

- `include/` public headers
- `src/` lexer, parser, AST, tree printer, and CLI
- `tests/` documented test suite and runner
- `build/` generated binaries

## Build

```sh
make
```

## Run

```sh
./build/sql_parser "SELECT name, age FROM student WHERE age > 18;"
```

## Example output

```text
query
  SELECT: SELECT
  select_list
    IDENTIFIER: name
    COMMA: ,
    IDENTIFIER: age
  FROM: FROM
  IDENTIFIER: student
  where_clause
    WHERE: WHERE
    expression
      comparison
        value
          Identifier: age
        comp_op: >
        value
          Number: 18
  SEMICOLON: ;
```

## Test

```sh
make test
```

## Design notes

- The lexer is case-insensitive for SQL keywords.
- The parser uses recursive descent and prints a fuller grammar-style parse tree representation.
- Errors are reported with token positions to make debugging easier.
