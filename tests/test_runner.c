#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"

#define ERROR_BUFFER_SIZE 256

typedef struct {
    const char *name;
    const char *sql;
    int should_pass;
} TestCase;

static int run_test(const TestCase *test_case) {
    TokenStream stream;
    SelectStatement statement;
    char error_buffer[ERROR_BUFFER_SIZE];
    int lexed;
    int parsed;

    lexed = lex_sql(test_case->sql, &stream, error_buffer, sizeof(error_buffer));
    if (!lexed) {
        if (!test_case->should_pass) {
            printf("[PASS] %s\n", test_case->name);
            printf("       Expected failure and lexer rejected input: %s\n", error_buffer);
            return 1;
        }

        printf("[FAIL] %s\n", test_case->name);
        printf("       Unexpected lexer error: %s\n", error_buffer);
        return 0;
    }

    parsed = parse_select_statement(&stream, &statement, error_buffer, sizeof(error_buffer));
    if (!parsed) {
        free_token_stream(&stream);

        if (!test_case->should_pass) {
            printf("[PASS] %s\n", test_case->name);
            printf("       Expected failure and parser rejected input: %s\n", error_buffer);
            return 1;
        }

        printf("[FAIL] %s\n", test_case->name);
        printf("       Unexpected parser error: %s\n", error_buffer);
        return 0;
    }

    free_select_statement(&statement);
    free_token_stream(&stream);

    if (!test_case->should_pass) {
        printf("[FAIL] %s\n", test_case->name);
        printf("       Query was accepted but should have failed.\n");
        return 0;
    }

    printf("[PASS] %s\n", test_case->name);
    return 1;
}

int main(void) {
    size_t index;
    size_t passed = 0;

    const TestCase test_cases[] = {
        {
            "valid_basic_projection",
            "SELECT name, age FROM student WHERE age > 18;",
            1
        },
        {
            "valid_wildcard_projection",
            "SELECT * FROM student;",
            1
        },
        {
            "valid_case_insensitive_keywords",
            "select name FROM student where age >= 18;",
            1
        },
        {
            "valid_string_literal_condition",
            "SELECT name FROM student WHERE department = 'CSE';",
            1
        },
        {
            "valid_parenthesized_boolean_expression",
            "SELECT name FROM student WHERE (age > 18 AND marks >= 75) OR city = 'Delhi';",
            1
        },
        {
            "valid_identifier_comparison",
            "SELECT name FROM student WHERE age > minimum_age;",
            1
        },
        {
            "valid_scalar_subquery_comparison",
            "SELECT name FROM student WHERE age > (SELECT minimum_age FROM thresholds WHERE level = 'senior');",
            1
        },
        {
            "valid_left_hand_scalar_subquery",
            "SELECT name FROM student WHERE (SELECT cutoff FROM thresholds WHERE level = 'junior') <= age;",
            1
        },
        {
            "valid_in_subquery",
            "SELECT name FROM student WHERE age IN (SELECT eligible_age FROM thresholds WHERE level = 'senior');",
            1
        },
        {
            "valid_from_subquery",
            "SELECT name FROM (SELECT name FROM student WHERE age > 18) senior_students;",
            1
        },
        {
            "invalid_missing_select_list",
            "SELECT FROM student WHERE age > 18;",
            0
        },
        {
            "invalid_missing_table_name",
            "SELECT name FROM WHERE age > 18;",
            0
        },
        {
            "invalid_missing_semicolon",
            "SELECT name FROM student WHERE age > 18",
            0
        },
        {
            "invalid_double_comma",
            "SELECT name,, age FROM student;",
            0
        },
        {
            "invalid_incomplete_where_clause",
            "SELECT name FROM student WHERE age > ;",
            0
        },
        {
            "invalid_unterminated_string",
            "SELECT name FROM student WHERE city = 'Delhi;",
            0
        },
        {
            "invalid_unknown_character",
            "SELECT name FROM student WHERE age @ 18;",
            0
        },
        {
            "invalid_unbalanced_parenthesis",
            "SELECT name FROM student WHERE (age > 18 AND city = 'Delhi';",
            0
        },
        {
            "invalid_incomplete_scalar_subquery",
            "SELECT name FROM student WHERE age > (SELECT minimum_age FROM thresholds WHERE);",
            0
        },
        {
            "invalid_from_subquery_missing_alias",
            "SELECT name FROM (SELECT name FROM student WHERE age > 18);",
            0
        }
    };

    for (index = 0; index < sizeof(test_cases) / sizeof(test_cases[0]); ++index) {
        passed += (size_t)run_test(&test_cases[index]);
    }

    printf("\nSummary: %zu/%zu tests passed.\n", passed, sizeof(test_cases) / sizeof(test_cases[0]));
    return passed == sizeof(test_cases) / sizeof(test_cases[0]) ? EXIT_SUCCESS : EXIT_FAILURE;
}
