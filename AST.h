#ifndef AST_H
#define AST_H

enum {
    AST_LITERAL,
    AST_FUNC,
    AST_RET,
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
    };
} AST;

/* Function for printing AST */
void print_AST(AST *root);

void free_AST(AST *root);


#endif
