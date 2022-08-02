#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "femtoC.h"

enum {
    EOFTYPE,
    PUNCT,
    IDENTIFIER,
    INTLITERAL,
};

typedef struct Token {
    int type;
    union {
        int intval;
        char charval;
        char *string;
    };
} Token;

Token *get_token();
void unget_token(Token *);
bool file_init(char *s);
void print_token(Token *);
void free_token(Token *);

#endif
