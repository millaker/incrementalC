#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "code_gen.h"
#include "femtoC.h"
#include "AST.h"

FILE* outfile = NULL;
static int label_count = 0;

#define emit(...) emitf(__VA_ARGS__)

void emitf(char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    vfprintf(outfile, fmt, arg);
    va_end(arg);
}

/* Forward Declaration of code-generating functions */
static void __code_gen(AST *root);
static void gen_func(AST *root);
static void gen_ret(AST *root);
static void gen_literal(AST *root);
static void gen_neg(AST* root);
static void gen_not(AST *root);
static void gen_complement(AST *root);
static void gen_log_or(AST *root);
static void gen_log_and(AST *root);
static void gen_binary_no_order(AST *root);
static void gen_binary_ordered(AST *root);
static int get_label();

static int get_label(){
    return label_count++;
}
static void gen_func(AST *root){
    emit("\t.globl %s\n", root->fname);
    emit("%s:\n", root->fname);
    __code_gen(root->stmt);
}

static void gen_ret(AST *root){
    __code_gen(root->retval);
    emit("\tret\n");
}

static void gen_literal(AST *root) {
    emit("\tmovl\t$%d, %%eax\n", root->intval);
}

static void gen_neg(AST* root){
    emit("\tneg\t%%eax\n");
}

static void gen_not(AST *root) {
    emit("\tcmpl\t$0, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
    emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
    emit("\tsete\t%%al\t\t#Set if zflag is on\n");
}

static void gen_complement(AST *root){
    emit("\tnot\t%%eax\n");
}

static void gen_log_or(AST *root){
    int e2_label = get_label(), end_label = get_label();;
    __code_gen(root->lexpr);
    emit("\tcmpl\t$0, %%eax\t\t#Compare first result\n");
    // Jump to .e2 if result is  zero else jump to .end
    emit("\tje\t.L%d\t\t#Jump to evaluate e2\n", e2_label);
    // set result to 1
    emit("\tmovl\t$1, %%eax\n");
    emit("\tjmp\t.L%d\n", end_label);
    // .e2
    emit(".L%d:\n",e2_label);
    // eval right side
    __code_gen(root->rexpr);
    // set accordingly
    emit("\tcmpl\t$0, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
    emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
    emit("\tsetne\t%%al\n");
    // .end
    emit(".L%d:\n",end_label);
    // Result is in eax
}

static void gen_log_and(AST *root){
    int e2_label = get_label(), end_label = get_label();
    // Eval first, if false, set zero, jump to end
    // Eval second , set accordingly
    __code_gen(root->lexpr);
    emit("\tcmpl\t$0, %%eax\t\t#Compare first result\n");
    emit("\tjne\t.L%d\t\t#Jump to evaluate e2\n", e2_label);
    emit("\tmovl\t$0, %%eax\n");
    emit("\tjmp\t.L%d\n", end_label);
    emit(".L%d:\n",e2_label);
    __code_gen(root->rexpr);
    emit("\tcmpl\t$0, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
    emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
    emit("\tsetne\t%%al\n");
    emit(".L%d:\n",end_label);
}

static void gen_binary_no_order(AST *root){
    __code_gen(root->lexpr);
    emit("\tpush\t%%rax\t\t#Push left expression result onto stack\n");
    __code_gen(root->rexpr);
    emit("\tpop\t%%rcx\t\t#Pop left expression result\n");
    switch(root->bop){
        case '+':
            emit("\taddl\t%%ecx, %%eax\n");
            break;
        case '*':
            emit("\timull\t%%ecx, %%eax\n");
            break;
        case '|':
            emit("\torl\t%%ecx, %%eax\n");
            break;
        case '&':
            emit("\tandl\t%%ecx, %%eax\n");
            break;
        case '^':
            emit("\txorl\t%%ecx, %%eax\n");
            break;
        case PUNCT_EQ:
            emit("\tcmpl\t%%ecx, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
            emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
            emit("\tsete\t%%al\t\t#Set if zflag is on\n");
            break;
        case PUNCT_NEQ:
            emit("\tcmpl\t%%ecx, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
            emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
            emit("\tsetne\t%%al\t\t#Set if zflag is on\n");
            break;
        default:
            fprintf(stderr, "Undefined no order binary operator\n");
            exit(1);
    }
}
static void gen_binary_ordered(AST *root){
    __code_gen(root->rexpr);
    emit("\tpush\t%%rax\t\t#Push right expression result onto stack\n");
    __code_gen(root->lexpr);
    emit("\tpop\t%%rcx\t\t#Pop right expression result\n");
    //Left in %eax, Right in %ecx
    switch(root->bop){
        case '/':
            emit("\tcdq\t\t#Convert double word to quad word, prep for idiv\n");
            emit("\tidivl\t%%ecx\t\t#IDIV: EDX:EAX divided by %%ecx, result stored in %%eax\n");
            break;
        case '%':
            emit("\tcdq\t\t#Convert double word to quad word, prep for idiv\n");
            emit("\tidivl\t%%ecx\t\t#IDIV: EDX:EAX divided by %%ecx, result stored in %%eax\n");
            emit("\tmovl\t%%edx, %%eax\t\t#Move remainder to eax\n");
            break;
        case '-':
            emit("\tsubl\t%%ecx, %%eax\n");
            break;
        case '>':
            emit("\tcmpl\t%%ecx, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
            emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
            emit("\tsetg\t%%al\t\t#Set if zflag is on\n");
            break;
        case '<':
            emit("\tcmpl\t%%ecx, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
            emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
            emit("\tsetl\t%%al\t\t#Set if zflag is on\n");
            break;
        case PUNCT_GTE:
            emit("\tcmpl\t%%ecx, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
            emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
            emit("\tsetge\t%%al\t\t#Set if zflag is on\n");
            break;
        case PUNCT_LTE:
            emit("\tcmpl\t%%ecx, %%eax\t\t#Compare %%eax with 0, sets zflag\n");
            emit("\tmovl\t$0, %%eax\t\t#Zero out %%eax\n");
            emit("\tsetle\t%%al\t\t#Set if zflag is on\n");
            break;
        case PUNCT_RSHIFT:
            emit("\tsarl\t%%cl, %%eax\n");
            break;
        case PUNCT_LSHIFT:
            emit("\tsall\t%%cl, %%eax\n");
            break;
        default:
            fprintf(stderr, "Undefined ordered binary operator\n");
            exit(1);
    }
}
static void __code_gen(AST *root) {
    switch(root->type) {
        case AST_FUNC:
            gen_func(root);
            break;
        case AST_RET:
            gen_ret(root);
            break;
        case AST_LITERAL:
            gen_literal(root);
            break;
        case AST_UNARY:
            __code_gen(root->expr);
            switch(root->uop) {
                case '-': /* Neg */
                    gen_neg(root);
                    break;
                case '!': /* logical not */
                    gen_not(root);
                    break;
                case '~': /* bitwise complement */
                    gen_complement(root);
                    break;
            }
            break;
        case AST_BINARY:
            switch(root->bop){
                case '+': /* Unordered binary operations */
                case '*': 
                case '|':
                case '&':
                case '^':
                case PUNCT_EQ:
                case PUNCT_NEQ:
                    gen_binary_no_order(root);
                    break;
                case '-': /* Ordered binary operator */
                case '/':
                case '%':
                case '>':
                case '<':
                case PUNCT_GTE:
                case PUNCT_LTE:
                case PUNCT_RSHIFT:
                case PUNCT_LSHIFT:
                    gen_binary_ordered(root);
                    break;
                case PUNCT_LOG_AND:
                    gen_log_and(root);
                    break;
                case PUNCT_LOG_OR:
                    gen_log_or(root);
                    break;
                default:
                    fprintf(stderr, "Code gen: Unkown binary operator\n");
                    exit(1);
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

