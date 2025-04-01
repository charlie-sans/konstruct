#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

#define MAX_TOKEN_LENGTH 256

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LENGTH];
} Token;

static const char *keywords[] = {
    "if", "else", "while", "return", NULL
};

static int is_keyword(const char *word) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

Token *next_token(const char **input) {
    while (**input != '\0') {
        if (isspace(**input)) {
            (*input)++;
            continue;
        }

        if (isalpha(**input)) {
            const char *start = *input;
            while (isalnum(**input)) {
                (*input)++;
            }
            size_t length = *input - start;
            Token *token = malloc(sizeof(Token));
            strncpy(token->value, start, length);
            token->value[length] = '\0';
            token->type = is_keyword(token->value) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
            return token;
        }

        if (isdigit(**input)) {
            const char *start = *input;
            while (isdigit(**input)) {
                (*input)++;
            }
            size_t length = *input - start;
            Token *token = malloc(sizeof(Token));
            strncpy(token->value, start, length);
            token->value[length] = '\0';
            token->type = TOKEN_NUMBER;
            return token;
        }

        if (strchr("+-*/", **input)) {
            Token *token = malloc(sizeof(Token));
            token->type = TOKEN_OPERATOR;
            token->value[0] = **input;
            token->value[1] = '\0';
            (*input)++;
            return token;
        }

        (*input)++;
    }

    Token *token = malloc(sizeof(Token));
    token->type = TOKEN_EOF;
    token->value[0] = '\0';
    return token;
}

void free_token(Token *token) {
    free(token);
}