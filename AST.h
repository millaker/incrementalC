#ifndef AST_H
#define AST_H

#include "list.h"

enum {
    AST_LITERAL,
    AST_FUNC,
    AST_RET,
    AST_UNARY,
    AST_POST_UNARY,
    AST_BINARY,
    AST_VAR,
    AST_VAR_DECL,
    AST_NOP
};

typedef struct AST {
    int type;
    union {
        /* Literal */
        int intval;
        /* FUNC */
        struct {
            char *fname;
            List *stmt;
        };
        /* AST_RET */
        struct {
            struct AST *retval;
        };
        /* AST_UNARY */
        struct {
            int uop;
            struct AST *expr;
        };
        /* AST_BINARY */
        struct {
            int bop;
            struct AST *lexpr;
            struct AST *rexpr;
        };
        /* AST_VAR_DECL */
        /* Variable reference uses the same format but with init left NULL */
        struct {
            char *vname;
            struct AST *init;
        };
    };
} AST;

/* Function for printing AST */
void print_AST(AST *root);

void free_AST(AST *root);


#endif
