#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// Structure to represent a node in the abstract syntax tree (AST)
typedef struct ASTNode {
    enum { NODE_TYPE_INT, NODE_TYPE_FLOAT, NODE_TYPE_STRING, NODE_TYPE_IDENTIFIER } type;
    union {
        int intValue;
        float floatValue;
        char* stringValue;
        char* identifier;
    } value;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

// Function declarations
ASTNode* parse_expression(Lexer* lexer);
ASTNode* parse_statement(Lexer* lexer);
void free_ast(ASTNode* node);

#endif // PARSER_H