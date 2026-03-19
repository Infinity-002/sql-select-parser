# SQL SELECT Parser Test Suite

This folder contains intentionally curated test cases for the SQL `SELECT` parser.

## Coverage goals

- Valid projection lists
- Wildcard projection
- Case-insensitive SQL keywords
- Numeric, string, and identifier operands
- Boolean expressions with `AND`, `OR`, and parentheses
- Syntax failures that a compiler front-end should reject cleanly
- Lexical failures such as unterminated strings and invalid characters

## Test case catalog

| Test name | Category | Purpose | Expected result |
| --- | --- | --- | --- |
| `valid_basic_projection` | Positive | Baseline `SELECT column_list FROM table WHERE comparison;` | Accept |
| `valid_wildcard_projection` | Positive | Accept `SELECT * FROM table;` | Accept |
| `valid_case_insensitive_keywords` | Positive | Verify keywords are recognized regardless of case | Accept |
| `valid_string_literal_condition` | Positive | Accept string literals in predicates | Accept |
| `valid_parenthesized_boolean_expression` | Positive | Verify precedence support with `AND`, `OR`, and grouping | Accept |
| `valid_identifier_comparison` | Positive | Accept identifier-to-identifier comparisons | Accept |
| `invalid_missing_select_list` | Negative | Reject `SELECT` without projection targets | Reject |
| `invalid_missing_table_name` | Negative | Reject `FROM` without table name | Reject |
| `invalid_missing_semicolon` | Negative | Reject input without statement terminator | Reject |
| `invalid_double_comma` | Negative | Reject malformed column list | Reject |
| `invalid_incomplete_where_clause` | Negative | Reject incomplete comparison expression | Reject |
| `invalid_unterminated_string` | Negative | Reject lexical string-literal errors | Reject |
| `invalid_unknown_character` | Negative | Reject unsupported symbols during lexing | Reject |
| `invalid_unbalanced_parenthesis` | Negative | Reject unmatched grouping in predicates | Reject |

## Running tests

```sh
make test
```

## Notes

The current parser is intentionally scoped to a simple educational subset of SQL:

- One `SELECT` statement at a time
- A single table in the `FROM` clause
- Column list or `*`
- Optional `WHERE` clause
- Comparisons combined with `AND` and `OR`
- Parentheses for grouping
