#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "print_tree.h"

#define ERROR_BUFFER_SIZE 256

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s \"SELECT name, age FROM student WHERE age > 18;\"\n", program_name);
}

int main(int argc, char **argv) {
    TokenStream stream;
    SelectStatement statement;
    char error_buffer[ERROR_BUFFER_SIZE];

    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!lex_sql(argv[1], &stream, error_buffer, sizeof(error_buffer))) {
        fprintf(stderr, "Lexing error: %s\n", error_buffer);
        return EXIT_FAILURE;
    }

    if (!parse_select_statement(&stream, &statement, error_buffer, sizeof(error_buffer))) {
        fprintf(stderr, "Parsing error: %s\n", error_buffer);
        free_token_stream(&stream);
        return EXIT_FAILURE;
    }

    print_select_statement_tree(stdout, &statement);

    free_select_statement(&statement);
    free_token_stream(&stream);
    return EXIT_SUCCESS;
}
