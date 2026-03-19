#include "ast.h"

#include <stdlib.h>

static void free_expression(ExpressionNode *expression) {
    if (expression == NULL) {
        return;
    }

    if (expression->type == EXPR_AND || expression->type == EXPR_OR) {
        free_expression(expression->data.logical.left);
        free_expression(expression->data.logical.right);
    } else {
        free(expression->data.comparison.left.text);
        free(expression->data.comparison.right.text);
    }

    free(expression);
}

void free_select_statement(SelectStatement *statement) {
    size_t index;

    if (statement == NULL) {
        return;
    }

    for (index = 0; index < statement->columns.count; ++index) {
        free(statement->columns.items[index]);
    }

    free(statement->columns.items);
    free(statement->table_name);
    free_expression(statement->where_clause);

    statement->columns.items = NULL;
    statement->columns.count = 0;
    statement->columns.is_wildcard = 0;
    statement->table_name = NULL;
    statement->where_clause = NULL;
}
