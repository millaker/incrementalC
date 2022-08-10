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
    AST_NOP,
    AST_IF,
    AST_COMPOUND,
    AST_TENARY,
    AST_FOR,
    AST_WHILE,
    AST_DO,
    AST_BREAK,
    AST_CONTINUE
};

typedef struct AST {
    int type;
    union {
        /* Literal */
        int intval;
        /* FUNC */
        struct {
            char *fname;
            struct AST *stmt;
        };
        /* AST_RET */
        struct {
            struct AST *retval;
        };
        /* AST_UNARY or AST_POST_UNARY */
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
        /* AST_IF  or AST_TENARY */
        struct {
            struct AST *cond;
            struct AST *then;
            struct AST *els;
        };
        /* AST_COMPOUND */
        struct {
            List *comp_stmt;
        };
        /* AST_FOR */
        struct {
            struct AST *fdecl;
            struct AST *fcond;
            struct AST *fexpr;
            struct AST *fbody;
        };
        /* AST_WHILE or AST_DO */
        struct {
            struct AST *wcond;
            struct AST *wbody;
        };
    };
} AST;

/* Function for printing AST */
void print_AST(AST *root);

void free_AST(AST *root);


#endif
