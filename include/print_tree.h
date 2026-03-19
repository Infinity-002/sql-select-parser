#ifndef PRINT_TREE_H
#define PRINT_TREE_H

#include <stdio.h>

#include "ast.h"

void print_select_statement_tree(FILE *output, const SelectStatement *statement);

#endif
