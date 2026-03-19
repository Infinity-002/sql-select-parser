CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
BUILD_DIR = build
BIN = $(BUILD_DIR)/sql_parser
TEST_BIN = $(BUILD_DIR)/test_runner

SRC = \
	src/ast.c \
	src/lexer.c \
	src/parser.c \
	src/print_tree.c \
	src/main.c

TEST_SRC = \
	src/ast.c \
	src/lexer.c \
	src/parser.c \
	src/print_tree.c \
	tests/test_runner.c

.PHONY: all clean test

all: $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

$(TEST_BIN): $(TEST_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)
