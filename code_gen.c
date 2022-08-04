#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "code_gen.h"
#include "AST.h"

FILE* outfile = NULL;

#define emit(...) emitf(__VA_ARGS__)

void emitf(char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    vfprintf(outfile, fmt, arg);
    va_end(arg);
}

void __code_gen(AST *root) {
    switch(root->type) {
        case AST_FUNC:
            emit("\t.globl %s\n", root->fname);
            emit("%s:\n", root->fname);
            __code_gen(root->stmt);
            break;
        case AST_RET:
            __code_gen(root->retval);
            emit("\tret\n");
            break;
        case AST_LITERAL:
            emit("\tmovl\t$%d, %%eax\n", root->intval);
            break;
        case AST_UNARY:
            __code_gen(root->expr);
            switch(root->uop) {
                case '-': /* Neg */
                    emit("\tneg\t%%eax\n");
                    break;
                case '!': /* logical not */
                    emit("\tcmpl\t$0, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
                    emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
                    emit("\tsete\t%%al\t\t#Set if zflag is on\n");
                    break;
                case '~': /* bitwise complement */
                    emit("\tnot\t%%eax\n");
                    break;
            }
            break;
        default:
            fprintf(stderr, "Code gen error: Invalid AST type\n");
            exit(1);
    }
}

void code_gen(char *filename, AST *ast) {
    char *str = (char*) malloc(sizeof(char) * (strlen(filename) + 3));
    strncpy(str, filename, strlen(filename));
    strcat(str, ".s");
    
    // Open out file
    outfile = fopen(str, "w");
    if(!outfile) {
        fprintf(stderr, "Open output file error\n");
        exit(1);
    }
    __code_gen(ast);
    fclose(outfile);
}

