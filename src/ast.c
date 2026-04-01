#include "ast.h"

#include <stdlib.h>

static void free_value(ValueNode *value) {
    if (value == NULL) {
        return;
    }

    free(value->text);
    value->text = NULL;

    if (value->subquery != NULL) {
        free_select_statement(value->subquery);
        free(value->subquery);
        value->subquery = NULL;
    }
}

static void free_expression(ExpressionNode *expression) {
    if (expression == NULL) {
        return;
    }

    if (expression->type == EXPR_AND || expression->type == EXPR_OR) {
        free_expression(expression->data.logical.left);
        free_expression(expression->data.logical.right);
    } else {
        free_value(&expression->data.comparison.left);
        free_value(&expression->data.comparison.right);
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
    free(statement->from_source.table_name);
    if (statement->from_source.subquery != NULL) {
        free_select_statement(statement->from_source.subquery);
        free(statement->from_source.subquery);
    }
    free(statement->from_source.alias);
    free_expression(statement->where_clause);

    statement->columns.items = NULL;
    statement->columns.count = 0;
    statement->columns.is_wildcard = 0;
    statement->from_source.table_name = NULL;
    statement->from_source.subquery = NULL;
    statement->from_source.alias = NULL;
    statement->where_clause = NULL;
}
