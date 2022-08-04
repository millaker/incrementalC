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

static char (*prefix)[5] = NULL;
static int max_depth = 16;

static const char *table[4] = {
    " ├─ ",
    " │  ",
    " └─ ",
    "    "
};

static inline void extend_prefix(){
    int old = max_depth;
    max_depth *= 2;
    prefix = realloc(prefix, max_depth);
    for(int i = old; i < max_depth; i++){
        for(int j = 0; j < 5; j++){
            prefix[i][j] = 0;
        }
    }
}

static inline void new_prefix(int indent, int last){
    if(indent >= max_depth)
        extend_prefix();
    strncpy(prefix[indent], last ? table[3]: table[1], 5);
}

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
            printf("BINARY(%c)\n", root->bop);
            __print_AST(root->lexpr, indent + 1, 0);
            __print_AST(root->rexpr, indent + 1, 1);
            break;
        default:
            printf("Unknown AST node\n");
    }
}

void print_AST(AST *root) {
    prefix = (char (*)[5]) malloc(sizeof(char [5]) * max_depth);
    for(int i = 0; i < max_depth; i++){
        for(int j = 0; j < 5; j++){
            prefix[i][j] = 0;
        }
    }
    __print_AST(root, 0, 1);
}






