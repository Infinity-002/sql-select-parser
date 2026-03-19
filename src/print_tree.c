#include "print_tree.h"

#include <stdio.h>

static const char *comparison_operator_name(ComparisonOperator op) {
    switch (op) {
        case COMP_EQ: return "=";
        case COMP_NEQ: return "!=";
        case COMP_LT: return "<";
        case COMP_GT: return ">";
        case COMP_LTE: return "<=";
        case COMP_GTE: return ">=";
    }

    return "?";
}

static const char *value_type_name(ValueType type) {
    switch (type) {
        case VALUE_IDENTIFIER: return "Identifier";
        case VALUE_NUMBER: return "Number";
        case VALUE_STRING: return "String";
    }

    return "Unknown";
}

static void print_indent(FILE *output, int depth) {
    int index;

    for (index = 0; index < depth; ++index) {
        fputs("  ", output);
    }
}

static void print_node_label(FILE *output, int depth, const char *label) {
    print_indent(output, depth);
    fprintf(output, "%s\n", label);
}

static void print_terminal(FILE *output, int depth, const char *token_name, const char *lexeme) {
    print_indent(output, depth);
    fprintf(output, "%s: %s\n", token_name, lexeme);
}

static void print_value(FILE *output, const ValueNode *value, int depth) {
    print_node_label(output, depth, "value");
    print_indent(output, depth + 1);
    fprintf(output, "%s: %s\n", value_type_name(value->type), value->text);
}

static void print_comparison(FILE *output, const ExpressionNode *expression, int depth) {
    print_node_label(output, depth, "comparison");
    print_value(output, &expression->data.comparison.left, depth + 1);
    print_terminal(
        output,
        depth + 1,
        "comp_op",
        comparison_operator_name(expression->data.comparison.op)
    );
    print_value(output, &expression->data.comparison.right, depth + 1);
}

static void print_expression(FILE *output, const ExpressionNode *expression, int depth) {
    print_node_label(output, depth, "expression");

    if (expression->type == EXPR_COMPARISON) {
        print_comparison(output, expression, depth + 1);
        return;
    }

    if (expression->type == EXPR_AND || expression->type == EXPR_OR) {
        print_expression(output, expression->data.logical.left, depth + 1);
        print_terminal(
            output,
            depth + 1,
            "logical_op",
            expression->type == EXPR_AND ? "AND" : "OR"
        );
        print_expression(output, expression->data.logical.right, depth + 1);
        return;
    }
}

void print_select_statement_tree(FILE *output, const SelectStatement *statement) {
    size_t index;

    print_node_label(output, 0, "query");
    print_terminal(output, 1, "SELECT", "SELECT");

    print_node_label(output, 1, "select_list");
    if (statement->columns.is_wildcard) {
        print_terminal(output, 2, "STAR", "*");
    } else {
        for (index = 0; index < statement->columns.count; ++index) {
            print_terminal(output, 2, "IDENTIFIER", statement->columns.items[index]);
            if (index + 1U < statement->columns.count) {
                print_terminal(output, 2, "COMMA", ",");
            }
        }
    }

    print_terminal(output, 1, "FROM", "FROM");
    print_terminal(output, 1, "IDENTIFIER", statement->table_name);

    if (statement->where_clause != NULL) {
        print_node_label(output, 1, "where_clause");
        print_terminal(output, 2, "WHERE", "WHERE");
        print_expression(output, statement->where_clause, 2);
    }

    print_terminal(output, 1, "SEMICOLON", ";");
}
