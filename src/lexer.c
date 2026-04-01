#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *duplicate_range(const char *start, size_t length) {
    char *copy = (char *)malloc(length + 1U);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, start, length);
    copy[length] = '\0';
    return copy;
}

static int append_token(TokenStream *stream, Token token, char *error_buffer, size_t error_buffer_size) {
    Token *new_tokens = (Token *)realloc(stream->tokens, sizeof(Token) * (stream->count + 1U));
    if (new_tokens == NULL) {
        snprintf(error_buffer, error_buffer_size, "Memory allocation failed while building token stream.");
        free(token.lexeme);
        return 0;
    }

    stream->tokens = new_tokens;
    stream->tokens[stream->count++] = token;
    return 1;
}

static int is_identifier_start(char ch) {
    return isalpha((unsigned char)ch) || ch == '_';
}

static int is_identifier_part(char ch) {
    return isalnum((unsigned char)ch) || ch == '_';
}

static TokenType keyword_type(const char *lexeme) {
    if (strcmp(lexeme, "SELECT") == 0) {
        return TOKEN_SELECT;
    }
    if (strcmp(lexeme, "FROM") == 0) {
        return TOKEN_FROM;
    }
    if (strcmp(lexeme, "WHERE") == 0) {
        return TOKEN_WHERE;
    }
    if (strcmp(lexeme, "AND") == 0) {
        return TOKEN_AND;
    }
    if (strcmp(lexeme, "OR") == 0) {
        return TOKEN_OR;
    }
    if (strcmp(lexeme, "IN") == 0) {
        return TOKEN_IN;
    }
    return TOKEN_IDENTIFIER;
}

static void uppercase_in_place(char *text) {
    size_t index;

    for (index = 0; text[index] != '\0'; ++index) {
        text[index] = (char)toupper((unsigned char)text[index]);
    }
}

int lex_sql(const char *input, TokenStream *stream, char *error_buffer, size_t error_buffer_size) {
    size_t position = 0;

    stream->tokens = NULL;
    stream->count = 0;

    while (input[position] != '\0') {
        Token token;
        size_t start;
        char current = input[position];

        if (isspace((unsigned char)current)) {
            ++position;
            continue;
        }

        token.lexeme = NULL;
        token.position = position;

        if (is_identifier_start(current)) {
            char *raw;

            start = position;
            while (is_identifier_part(input[position])) {
                ++position;
            }

            raw = duplicate_range(input + start, position - start);
            if (raw == NULL) {
                snprintf(error_buffer, error_buffer_size, "Memory allocation failed while scanning identifier.");
                free_token_stream(stream);
                return 0;
            }

            token.lexeme = raw;
            uppercase_in_place(raw);
            token.type = keyword_type(raw);
            if (token.type == TOKEN_IDENTIFIER) {
                free(raw);
                token.lexeme = duplicate_range(input + start, position - start);
                if (token.lexeme == NULL) {
                    snprintf(error_buffer, error_buffer_size, "Memory allocation failed while storing identifier.");
                    free_token_stream(stream);
                    return 0;
                }
            }

            if (!append_token(stream, token, error_buffer, error_buffer_size)) {
                free_token_stream(stream);
                return 0;
            }
            continue;
        }

        if (isdigit((unsigned char)current)) {
            start = position;
            while (isdigit((unsigned char)input[position])) {
                ++position;
            }

            if (input[position] == '.') {
                ++position;
                if (!isdigit((unsigned char)input[position])) {
                    snprintf(error_buffer, error_buffer_size, "Malformed numeric literal near position %zu.", start);
                    free_token_stream(stream);
                    return 0;
                }

                while (isdigit((unsigned char)input[position])) {
                    ++position;
                }
            }

            token.type = TOKEN_NUMBER;
            token.lexeme = duplicate_range(input + start, position - start);
            if (token.lexeme == NULL) {
                snprintf(error_buffer, error_buffer_size, "Memory allocation failed while scanning number.");
                free_token_stream(stream);
                return 0;
            }

            if (!append_token(stream, token, error_buffer, error_buffer_size)) {
                free_token_stream(stream);
                return 0;
            }
            continue;
        }

        if (current == '\'') {
            start = ++position;
            while (input[position] != '\0' && input[position] != '\'') {
                ++position;
            }

            if (input[position] != '\'') {
                snprintf(error_buffer, error_buffer_size, "Unterminated string literal near position %zu.", start - 1U);
                free_token_stream(stream);
                return 0;
            }

            token.type = TOKEN_STRING;
            token.lexeme = duplicate_range(input + start, position - start);
            if (token.lexeme == NULL) {
                snprintf(error_buffer, error_buffer_size, "Memory allocation failed while scanning string.");
                free_token_stream(stream);
                return 0;
            }

            ++position;
            if (!append_token(stream, token, error_buffer, error_buffer_size)) {
                free_token_stream(stream);
                return 0;
            }
            continue;
        }

        ++position;
        switch (current) {
            case ',':
                token.type = TOKEN_COMMA;
                token.lexeme = duplicate_range(",", 1U);
                break;
            case '*':
                token.type = TOKEN_STAR;
                token.lexeme = duplicate_range("*", 1U);
                break;
            case ';':
                token.type = TOKEN_SEMICOLON;
                token.lexeme = duplicate_range(";", 1U);
                break;
            case '(':
                token.type = TOKEN_LPAREN;
                token.lexeme = duplicate_range("(", 1U);
                break;
            case ')':
                token.type = TOKEN_RPAREN;
                token.lexeme = duplicate_range(")", 1U);
                break;
            case '=':
                token.type = TOKEN_EQ;
                token.lexeme = duplicate_range("=", 1U);
                break;
            case '!':
                if (input[position] == '=') {
                    ++position;
                    token.type = TOKEN_NEQ;
                    token.lexeme = duplicate_range("!=", 2U);
                    break;
                }
                snprintf(error_buffer, error_buffer_size, "Unexpected character '!' at position %zu.", position - 1U);
                free_token_stream(stream);
                return 0;
            case '<':
                if (input[position] == '=') {
                    ++position;
                    token.type = TOKEN_LTE;
                    token.lexeme = duplicate_range("<=", 2U);
                } else {
                    token.type = TOKEN_LT;
                    token.lexeme = duplicate_range("<", 1U);
                }
                break;
            case '>':
                if (input[position] == '=') {
                    ++position;
                    token.type = TOKEN_GTE;
                    token.lexeme = duplicate_range(">=", 2U);
                } else {
                    token.type = TOKEN_GT;
                    token.lexeme = duplicate_range(">", 1U);
                }
                break;
            default:
                snprintf(error_buffer, error_buffer_size, "Unexpected character '%c' at position %zu.", current, position - 1U);
                free_token_stream(stream);
                return 0;
        }

        if (token.lexeme == NULL) {
            snprintf(error_buffer, error_buffer_size, "Memory allocation failed while scanning punctuation.");
            free_token_stream(stream);
            return 0;
        }

        if (!append_token(stream, token, error_buffer, error_buffer_size)) {
            free_token_stream(stream);
            return 0;
        }
    }

    {
        Token eof_token;

        eof_token.type = TOKEN_EOF;
        eof_token.lexeme = duplicate_range("EOF", 3U);
        eof_token.position = position;
        if (eof_token.lexeme == NULL) {
            snprintf(error_buffer, error_buffer_size, "Memory allocation failed while finalizing token stream.");
            free_token_stream(stream);
            return 0;
        }

        if (!append_token(stream, eof_token, error_buffer, error_buffer_size)) {
            free_token_stream(stream);
            return 0;
        }
    }

    return 1;
}

void free_token_stream(TokenStream *stream) {
    size_t index;

    if (stream == NULL) {
        return;
    }

    for (index = 0; index < stream->count; ++index) {
        free(stream->tokens[index].lexeme);
    }

    free(stream->tokens);
    stream->tokens = NULL;
    stream->count = 0;
}

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_SELECT: return "SELECT";
        case TOKEN_FROM: return "FROM";
        case TOKEN_WHERE: return "WHERE";
        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_IN: return "IN";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NEQ: return "NEQ";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_LTE: return "LTE";
        case TOKEN_GTE: return "GTE";
        case TOKEN_EOF: return "EOF";
        case TOKEN_INVALID: return "INVALID";
    }

    return "UNKNOWN";
}
