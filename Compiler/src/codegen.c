#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "compiler.h"

void generate_code(const ASTNode *node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case NODE_FUNCTION:
            printf("section .text\n");
            printf("%s:\n", node->function_name);
            generate_code(node->body);
            break;

        case NODE_VARIABLE:
            printf("mov rax, %s\n", node->var_name);
            break;

        case NODE_ASSIGNMENT:
            printf("mov %s, rax\n", node->var_name);
            generate_code(node->value);
            break;

        case NODE_LITERAL:
            printf("mov rax, %d\n", node->value);
            break;

        case NODE_BINARY_OP:
            generate_code(node->left);
            generate_code(node->right);
            printf("add rax, rbx\n"); // Example for addition
            break;

        default:
            fprintf(stderr, "Unknown AST node type: %d\n", node->type);
            exit(EXIT_FAILURE);
    }
}

void compile(const char *source) {
    ASTNode *ast = parse(source);
    generate_code(ast);
    free_ast(ast);
}