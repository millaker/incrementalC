#include "AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * A printing method for AST.
 * Currently only supports max depth of 16
 * Dynamically allocated array can solve the problem
 *
 * When introducing new child for every node, make sure to only turn on last
 * if its the last child.
 */

static char **prefix = {0};
static int max_depth = 16;

static char *table[4] = {
    " ├─ ",
    " │  ",
    " └─ ",
    "    "
};


static inline void extend_prefix(){
    max_depth *= 2;
    prefix = realloc(prefix, sizeof(char*) * max_depth);
}


static inline void new_prefix(int indent, int last){
    if(indent >= max_depth)
        extend_prefix();
    prefix[indent] = last ? table[3] : table[1];
}

static char *operator_table[16] = {
    "&&", "||", "&=", "|=", "==", "!=", "<=", ">=", "<<", ">>", "++",
    "+=", "--", "--", "*=", "/="
};

void __print_AST(AST *root,int indent, int last){
    if(!root)
        return;
    for(int i = 0; i < indent; i++){
        fputs(prefix[i], stdout);
    }
    fputs(last ? table[2]: table[0], stdout);
    new_prefix(indent, last);
    switch(root->type) {
        case AST_LITERAL:
            /* This will always be a leaf
             No need for recursive call */
            printf("INT(%d)\n", root->intval);
            break;
        case AST_FUNC:
            /* Print statement lists */
            printf("FUNC(%s)\n", root->fname);
            __print_AST(root->stmt, indent + 1,  1);
            break;
        case AST_RET:
            printf("RETURN\n");
            __print_AST(root->retval, indent + 1,  1);
            break;
        case AST_UNARY:
            printf("UNARY(%c)\n", root->uop);
            __print_AST(root->expr, indent + 1, 1);
            break;
        case AST_BINARY:
            if(root->bop > 255) {
                printf("BINARY(%s)\n", operator_table[root->bop - 256]);
            }else{
                printf("BINARY(%c)\n", root->bop);
            }
            __print_AST(root->lexpr, indent + 1, 0);
            __print_AST(root->rexpr, indent + 1, 1);
            break;
        default:
            printf("Unknown AST node\n");
    }
}

void print_AST(AST *root) {
    prefix = malloc(sizeof(char*) * max_depth);
    for(int i = 0; i < max_depth; i++)
        prefix[i] = NULL;
    __print_AST(root, 0, 1);
}






