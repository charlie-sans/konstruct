#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    uint32_t length;
    uint32_t line;
} Token;

void init_lexer(const char *source);
Token next_token();

#endif // LEXER_H