#ifndef CODEGEN_H
#define CODEGEN_H

// Function to generate assembly code from the abstract syntax tree
void generate_code(ASTNode *root);

// Function to initialize the code generation process
void init_codegen();

// Function to finalize the code generation process
void finalize_codegen();

#endif // CODEGEN_H