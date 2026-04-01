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
        case COMP_IN: return "IN";
    }

    return "?";
}

static const char *value_type_name(ValueType type) {
    switch (type) {
        case VALUE_IDENTIFIER: return "Identifier";
        case VALUE_NUMBER: return "Number";
        case VALUE_STRING: return "String";
        case VALUE_SUBQUERY: return "Subquery";
    }

    return "Unknown";
}

static void print_branch_prefix(FILE *output, const char *prefix, int is_last) {
    fputs(prefix, output);
    fputs(is_last ? "`-- " : "|-- ", output);
}

static void build_child_prefix(char *destination, size_t destination_size, const char *prefix, int is_last) {
    snprintf(destination, destination_size, "%s%s", prefix, is_last ? "    " : "|   ");
}

static void print_node_label(FILE *output, const char *prefix, int is_last, const char *label) {
    print_branch_prefix(output, prefix, is_last);
    fprintf(output, "%s\n", label);
}

static void print_terminal(FILE *output, const char *prefix, int is_last, const char *token_name, const char *lexeme) {
    print_branch_prefix(output, prefix, is_last);
    fprintf(output, "%s: %s\n", token_name, lexeme);
}

static void print_query(FILE *output, const SelectStatement *statement, const char *prefix, int is_last);
static void print_from_source(FILE *output, const FromSource *from_source, const char *prefix);

static void print_value(FILE *output, const ValueNode *value, const char *prefix, int is_last) {
    char child_prefix[256];

    print_node_label(output, prefix, is_last, "value");
    build_child_prefix(child_prefix, sizeof(child_prefix), prefix, is_last);

    if (value->type == VALUE_SUBQUERY) {
        print_node_label(output, child_prefix, 0, value_type_name(value->type));
        print_terminal(output, child_prefix, 0, "LPAREN", "(");
        print_query(output, value->subquery, child_prefix, 0);
        print_terminal(output, child_prefix, 1, "RPAREN", ")");
        return;
    }

    print_terminal(output, child_prefix, 1, value_type_name(value->type), value->text);
}

static void print_comparison(FILE *output, const ExpressionNode *expression, const char *prefix, int is_last) {
    char child_prefix[256];

    print_node_label(output, prefix, is_last, "comparison");
    build_child_prefix(child_prefix, sizeof(child_prefix), prefix, is_last);

    print_value(output, &expression->data.comparison.left, child_prefix, 0);
    print_terminal(
        output,
        child_prefix,
        0,
        "comp_op",
        comparison_operator_name(expression->data.comparison.op)
    );
    print_value(output, &expression->data.comparison.right, child_prefix, 1);
}

static void print_expression(FILE *output, const ExpressionNode *expression, const char *prefix, int is_last) {
    char child_prefix[256];

    print_node_label(output, prefix, is_last, "expression");
    build_child_prefix(child_prefix, sizeof(child_prefix), prefix, is_last);

    if (expression->type == EXPR_COMPARISON) {
        print_comparison(output, expression, child_prefix, 1);
        return;
    }

    if (expression->type == EXPR_AND || expression->type == EXPR_OR) {
        print_expression(output, expression->data.logical.left, child_prefix, 0);
        print_terminal(
            output,
            child_prefix,
            0,
            "logical_op",
            expression->type == EXPR_AND ? "AND" : "OR"
        );
        print_expression(output, expression->data.logical.right, child_prefix, 1);
    }
}

static void print_query(FILE *output, const SelectStatement *statement, const char *prefix, int is_last) {
    size_t index;
    char query_prefix[256];

    print_node_label(output, prefix, is_last, "query");
    build_child_prefix(query_prefix, sizeof(query_prefix), prefix, is_last);
    print_terminal(output, query_prefix, 0, "SELECT", "SELECT");

    print_node_label(output, query_prefix, 0, "select_list");
    if (statement->columns.is_wildcard) {
        print_terminal(output, "|   ", 1, "STAR", "*");
    } else {
        for (index = 0; index < statement->columns.count; ++index) {
            int has_more_columns = index + 1U < statement->columns.count;

            print_terminal(
                output,
                "|   ",
                has_more_columns,
                "IDENTIFIER",
                statement->columns.items[index]
            );
            if (has_more_columns) {
                print_terminal(output, "|   ", 0, "COMMA", ",");
            }
        }
    }

    print_terminal(output, query_prefix, 0, "FROM", "FROM");
    print_from_source(output, &statement->from_source, query_prefix);
    if (statement->where_clause != NULL) {
        print_node_label(output, query_prefix, 0, "where_clause");
        print_terminal(output, "|   ", 0, "WHERE", "WHERE");
        print_expression(output, statement->where_clause, "|   ", 1);
        return;
    }
}

static void print_from_source(FILE *output, const FromSource *from_source, const char *prefix) {
    if (from_source->subquery != NULL) {
        print_terminal(output, prefix, 0, "LPAREN", "(");
        print_query(output, from_source->subquery, prefix, 0);
        print_terminal(output, prefix, 0, "RPAREN", ")");
        print_terminal(output, prefix, 0, "IDENTIFIER", from_source->alias);
        return;
    }

    print_terminal(output, prefix, 0, "IDENTIFIER", from_source->table_name);
}

void print_select_statement_tree(FILE *output, const SelectStatement *statement) {
    fprintf(output, "query\n");
    print_terminal(output, "", 0, "SELECT", "SELECT");

    print_node_label(output, "", 0, "select_list");
    if (statement->columns.is_wildcard) {
        print_terminal(output, "|   ", 1, "STAR", "*");
    } else {
        size_t index;

        for (index = 0; index < statement->columns.count; ++index) {
            int has_more_columns = index + 1U < statement->columns.count;

            print_terminal(
                output,
                "|   ",
                has_more_columns,
                "IDENTIFIER",
                statement->columns.items[index]
            );
            if (has_more_columns) {
                print_terminal(output, "|   ", 0, "COMMA", ",");
            }
        }
    }

    print_terminal(output, "", 0, "FROM", "FROM");
    print_from_source(output, &statement->from_source, "");
    if (statement->where_clause != NULL) {
        print_node_label(output, "", 0, "where_clause");
        print_terminal(output, "|   ", 0, "WHERE", "WHERE");
        print_expression(output, statement->where_clause, "|   ", 1);
        print_terminal(output, "", 1, "SEMICOLON", ";");
        return;
    }
    print_terminal(output, "", 1, "SEMICOLON", ";");
}
