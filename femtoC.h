#ifndef FEMTOC_H
#define FEMTOC_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define bool int 
#define TRUE 1
#define FALSE 0
enum {
    EOFTYPE,
    PUNCT,
    IDENTIFIER,
    INTLITERAL,
};

enum {
    PUNCT_LOG_AND = 256,
    PUNCT_LOG_OR,
    PUNCT_BIT_AND_ASSIGN,
    PUNCT_BIT_OR_ASSIGN,
    PUNCT_EQ,
    PUNCT_NEQ,
    PUNCT_LTE,
    PUNCT_RSHIFT,
    PUNCT_GTE,
    PUNCT_LSHIFT,
    PUNCT_INC,
    PUNCT_ADD_ASSIGN,
    PUNCT_DEC,
    PUNCT_SUB_ASSIGN,
    PUNCT_MUL_ASSIGN,
    PUNCT_DIV_ASSIGN,
};

#endif
