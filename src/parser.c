#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const TokenStream *stream;
    size_t current;
    char *error_buffer;
    size_t error_buffer_size;
} ParserState;

static void parser_error(ParserState *state, const char *message, const Token *token) {
    snprintf(
        state->error_buffer,
        state->error_buffer_size,
        "%s at position %zu near token '%s'.",
        message,
        token->position,
        token->lexeme
    );
}

static const Token *peek(const ParserState *state) {
    return &state->stream->tokens[state->current];
}

static const Token *previous(const ParserState *state) {
    return &state->stream->tokens[state->current - 1U];
}

static int is_at_end(const ParserState *state) {
    return peek(state)->type == TOKEN_EOF;
}

static int match(ParserState *state, TokenType type) {
    if (peek(state)->type != type) {
        return 0;
    }

    ++state->current;
    return 1;
}

static int consume(ParserState *state, TokenType type, const char *message) {
    if (match(state, type)) {
        return 1;
    }

    parser_error(state, message, peek(state));
    return 0;
}

static char *copy_text(const char *source) {
    size_t length = strlen(source);
    char *copy = (char *)malloc(length + 1U);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, source, length + 1U);
    return copy;
}

static int append_column(ColumnList *columns, const char *name, ParserState *state) {
    char **new_items = (char **)realloc(columns->items, sizeof(char *) * (columns->count + 1U));
    if (new_items == NULL) {
        snprintf(state->error_buffer, state->error_buffer_size, "Memory allocation failed while storing column list.");
        return 0;
    }

    columns->items = new_items;
    columns->items[columns->count] = copy_text(name);
    if (columns->items[columns->count] == NULL) {
        snprintf(state->error_buffer, state->error_buffer_size, "Memory allocation failed while copying column name.");
        return 0;
    }

    ++columns->count;
    return 1;
}

static int parse_value(ParserState *state, ValueNode *value) {
    const Token *token = peek(state);

    if (match(state, TOKEN_IDENTIFIER)) {
        value->type = VALUE_IDENTIFIER;
        value->text = copy_text(previous(state)->lexeme);
    } else if (match(state, TOKEN_NUMBER)) {
        value->type = VALUE_NUMBER;
        value->text = copy_text(previous(state)->lexeme);
    } else if (match(state, TOKEN_STRING)) {
        value->type = VALUE_STRING;
        value->text = copy_text(previous(state)->lexeme);
    } else {
        parser_error(state, "Expected identifier, number, or string literal", token);
        return 0;
    }

    if (value->text == NULL) {
        snprintf(state->error_buffer, state->error_buffer_size, "Memory allocation failed while parsing value.");
        return 0;
    }

    return 1;
}

static int parse_comparison_operator(ParserState *state, ComparisonOperator *operator_type) {
    if (match(state, TOKEN_EQ)) {
        *operator_type = COMP_EQ;
        return 1;
    }
    if (match(state, TOKEN_NEQ)) {
        *operator_type = COMP_NEQ;
        return 1;
    }
    if (match(state, TOKEN_LT)) {
        *operator_type = COMP_LT;
        return 1;
    }
    if (match(state, TOKEN_GT)) {
        *operator_type = COMP_GT;
        return 1;
    }
    if (match(state, TOKEN_LTE)) {
        *operator_type = COMP_LTE;
        return 1;
    }
    if (match(state, TOKEN_GTE)) {
        *operator_type = COMP_GTE;
        return 1;
    }

    parser_error(state, "Expected comparison operator", peek(state));
    return 0;
}

static ExpressionNode *parse_expression(ParserState *state);

static ExpressionNode *allocate_expression(ParserState *state, ExpressionType type) {
    ExpressionNode *node = (ExpressionNode *)calloc(1U, sizeof(ExpressionNode));
    if (node == NULL) {
        snprintf(state->error_buffer, state->error_buffer_size, "Memory allocation failed while building expression tree.");
        return NULL;
    }

    node->type = type;
    return node;
}

static void free_expression_node(ExpressionNode *node) {
    if (node == NULL) {
        return;
    }

    if (node->type == EXPR_AND || node->type == EXPR_OR) {
        free_expression_node(node->data.logical.left);
        free_expression_node(node->data.logical.right);
    } else {
        free(node->data.comparison.left.text);
        free(node->data.comparison.right.text);
    }

    free(node);
}

static ExpressionNode *parse_primary(ParserState *state) {
    ExpressionNode *node;

    if (match(state, TOKEN_LPAREN)) {
        ExpressionNode *inner = parse_expression(state);
        if (inner == NULL) {
            return NULL;
        }

        if (!consume(state, TOKEN_RPAREN, "Expected ')' after expression")) {
            free_expression_node(inner);
            return NULL;
        }

        return inner;
    }

    node = allocate_expression(state, EXPR_COMPARISON);
    if (node == NULL) {
        return NULL;
    }

    if (!parse_value(state, &node->data.comparison.left) ||
        !parse_comparison_operator(state, &node->data.comparison.op) ||
        !parse_value(state, &node->data.comparison.right)) {
        free_expression_node(node);
        return NULL;
    }

    return node;
}

static ExpressionNode *parse_and(ParserState *state) {
    ExpressionNode *left = parse_primary(state);
    if (left == NULL) {
        return NULL;
    }

    while (match(state, TOKEN_AND)) {
        ExpressionNode *node = allocate_expression(state, EXPR_AND);
        ExpressionNode *right;

        if (node == NULL) {
            free_expression_node(left);
            return NULL;
        }

        right = parse_primary(state);
        if (right == NULL) {
            free_expression_node(node);
            free_expression_node(left);
            return NULL;
        }

        node->data.logical.left = left;
        node->data.logical.right = right;
        left = node;
    }

    return left;
}

static ExpressionNode *parse_expression(ParserState *state) {
    ExpressionNode *left = parse_and(state);
    if (left == NULL) {
        return NULL;
    }

    while (match(state, TOKEN_OR)) {
        ExpressionNode *node = allocate_expression(state, EXPR_OR);
        ExpressionNode *right;

        if (node == NULL) {
            free_expression_node(left);
            return NULL;
        }

        right = parse_and(state);
        if (right == NULL) {
            free_expression_node(node);
            free_expression_node(left);
            return NULL;
        }

        node->data.logical.left = left;
        node->data.logical.right = right;
        left = node;
    }

    return left;
}

static int parse_select_list(ParserState *state, ColumnList *columns) {
    if (match(state, TOKEN_STAR)) {
        columns->is_wildcard = 1;
        return 1;
    }

    if (!match(state, TOKEN_IDENTIFIER)) {
        parser_error(state, "Expected column name or '*'", peek(state));
        return 0;
    }

    if (!append_column(columns, previous(state)->lexeme, state)) {
        return 0;
    }

    while (match(state, TOKEN_COMMA)) {
        if (!consume(state, TOKEN_IDENTIFIER, "Expected column name after ','")) {
            return 0;
        }

        if (!append_column(columns, previous(state)->lexeme, state)) {
            return 0;
        }
    }

    return 1;
}

int parse_select_statement(
    const TokenStream *stream,
    SelectStatement *statement,
    char *error_buffer,
    size_t error_buffer_size
) {
    ParserState state;

    memset(statement, 0, sizeof(*statement));
    state.stream = stream;
    state.current = 0;
    state.error_buffer = error_buffer;
    state.error_buffer_size = error_buffer_size;

    if (!consume(&state, TOKEN_SELECT, "Expected 'SELECT' at start of query")) {
        return 0;
    }

    if (!parse_select_list(&state, &statement->columns)) {
        free_select_statement(statement);
        return 0;
    }

    if (!consume(&state, TOKEN_FROM, "Expected 'FROM' after select list")) {
        free_select_statement(statement);
        return 0;
    }

    if (!consume(&state, TOKEN_IDENTIFIER, "Expected table name after 'FROM'")) {
        free_select_statement(statement);
        return 0;
    }

    statement->table_name = copy_text(previous(&state)->lexeme);
    if (statement->table_name == NULL) {
        snprintf(error_buffer, error_buffer_size, "Memory allocation failed while copying table name.");
        free_select_statement(statement);
        return 0;
    }

    if (match(&state, TOKEN_WHERE)) {
        statement->where_clause = parse_expression(&state);
        if (statement->where_clause == NULL) {
            free_select_statement(statement);
            return 0;
        }
    }

    if (!consume(&state, TOKEN_SEMICOLON, "Expected ';' at end of query")) {
        free_select_statement(statement);
        return 0;
    }

    if (!is_at_end(&state)) {
        parser_error(&state, "Unexpected tokens after end of query", peek(&state));
        free_select_statement(statement);
        return 0;
    }

    return 1;
}
