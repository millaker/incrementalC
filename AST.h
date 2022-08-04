#ifndef AST_H
#define AST_H

enum {
    AST_LITERAL,
    AST_FUNC,
    AST_RET,
    AST_UNARY,
    AST_BINARY
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
    };
} AST;

/* Function for printing AST */
void print_AST(AST *root);

void free_AST(AST *root);


#endif
