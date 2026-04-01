#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef struct SelectStatement SelectStatement;

typedef enum {
    VALUE_IDENTIFIER,
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_SUBQUERY
} ValueType;

typedef struct ValueNode {
    ValueType type;
    char *text;
    SelectStatement *subquery;
} ValueNode;

typedef struct FromSource {
    char *table_name;
    SelectStatement *subquery;
    char *alias;
} FromSource;

typedef struct ColumnList {
    char **items;
    size_t count;
    int is_wildcard;
} ColumnList;

typedef enum {
    COMP_EQ,
    COMP_NEQ,
    COMP_LT,
    COMP_GT,
    COMP_LTE,
    COMP_GTE,
    COMP_IN
} ComparisonOperator;

typedef enum {
    EXPR_COMPARISON,
    EXPR_AND,
    EXPR_OR
} ExpressionType;

typedef struct ExpressionNode ExpressionNode;

struct ExpressionNode {
    ExpressionType type;
    union {
        struct {
            ValueNode left;
            ComparisonOperator op;
            ValueNode right;
        } comparison;
        struct {
            ExpressionNode *left;
            ExpressionNode *right;
        } logical;
    } data;
};

struct SelectStatement {
    ColumnList columns;
    FromSource from_source;
    ExpressionNode *where_clause;
};

void free_select_statement(SelectStatement *statement);

#endif
