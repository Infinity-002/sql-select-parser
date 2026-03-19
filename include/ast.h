#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef enum {
    VALUE_IDENTIFIER,
    VALUE_NUMBER,
    VALUE_STRING
} ValueType;

typedef struct ValueNode {
    ValueType type;
    char *text;
} ValueNode;

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
    COMP_GTE
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

typedef struct SelectStatement {
    ColumnList columns;
    char *table_name;
    ExpressionNode *where_clause;
} SelectStatement;

void free_select_statement(SelectStatement *statement);

#endif
