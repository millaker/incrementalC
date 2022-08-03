#include "AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DEPTH 16

/*
 * A printing method for AST.
 * Currently only supports max depth of 16
 * Dynamically allocated array can solve the problem
 *
 * When introducing new child for every node, make sure to only turn on last
 * if its the last child.
 */

static char prefix[5][MAX_DEPTH] = {0};

static const char *table[4] = {
    " ├─ ",
    " │  ",
    " └─ ",
    "    "
};

static inline void new_prefix(int indent, int last){
    strcpy(prefix[indent], last ? table[3]: table[1]);
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
        default:
            ;
    }
}

void print_AST(AST *root) {
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < MAX_DEPTH; j++){
            prefix[i][j] = '\0';
        }
    }
    __print_AST(root, 0, 1);
}





