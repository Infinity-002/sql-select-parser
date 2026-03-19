#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#include "ast.h"
#include "lexer.h"

int parse_select_statement(
    const TokenStream *stream,
    SelectStatement *statement,
    char *error_buffer,
    size_t error_buffer_size
);

#endif
