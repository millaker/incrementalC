#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "code_gen.h"
#include "femtoC.h"
#include "AST.h"
#include "list.h"
#include "symtab.h"

FILE* outfile = NULL;
symtab *curr_scope = NULL;
static int label_count = 0;

#define emit(...) emitf(__VA_ARGS__)
#define error(...) errorf(__VA_ARGS__)
#define ALIGN_SIZE 8
#define align_8(x) (((x) + ALIGN_SIZE - 1) & ~(x - 1))

void emitf(char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    vfprintf(outfile, fmt, arg);
    va_end(arg);
}

void errorf(char *fmt, ...){
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    exit(1);
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
static void gen_var_decl(AST *root);
static void gen_assign(AST *root);
static void gen_prefix(AST *root);
static void gen_postfix(AST *root);
static void gen_if(AST *root);
static void gen_tenary(AST *root);
static int  gen_compound(AST *root);
static int  get_label();
static void new_scope();
static int  close_scope();
static List* search_curr_scope(char *name);
static List* search_all_scope(char *name);

/* Return 1 if the last statement is a return statement
 * used to signal function generation
 */
static int gen_compound(AST *root){
    int ret = 0;
    new_scope();
    for_each_node_unsafe(root->comp_stmt, ptr){
        __code_gen((AST*)ptr->val);
        if(ptr->next == root->comp_stmt && ((AST*)ptr->val)->type == AST_RET)
            ret = 1;
    }
    int deallocate = close_scope();
    emit("\tadd \t$%d, %%rsp\n", deallocate);
    return ret;
}

static List* search_curr_scope(char *name){
    return symtab_find(curr_scope, name);
}

static List* search_all_scope(char *name){
    symtab *curr = curr_scope;
    List *res = NULL;
    while(curr){
        res = symtab_find(curr, name);
        if(res)
            return res;
        curr = curr->parent;
    }
    return NULL;
}

/* new_scope() and close_scope() will be called when entering and leaving a new
 * compound statement.
 */

/* Returns the size to be deallocate */
static int close_scope(){
    int old_off = *((int*)curr_scope->table->val);
    symtab *closed = curr_scope;
    curr_scope = curr_scope->parent;
    free_symtab(closed);
    int curr_off = curr_scope ? *((int*)curr_scope->table->val) : 0;
    return old_off - curr_off; 
}

static void new_scope(){
    /* First offset will be 8 bytes because of saved %rbp */
    symtab *new_tab = NEW_SYMTAB;
    new_tab->parent = curr_scope;
    int *off = (int*) malloc(sizeof(int));
    *off = curr_scope ? *((int*)curr_scope->table->val): 0; /* inherit offset from parent */
    new_tab->table->val = off;
    curr_scope = new_tab;
}

static void gen_if(AST *root){
    if(root->els){
        gen_tenary(root);
        return;
    }
    int end_label = get_label();
    __code_gen(root->cond);
    emit("\tcmpl\t$0, %%eax\t\t\n");
    emit("\tje  \t.L%d\n", end_label);
    __code_gen(root->then);
    emit(".L%d:\n", end_label);
}

static void gen_tenary(AST *root){
    int e2_label = get_label(), end_label = get_label();
    __code_gen(root->cond);
    emit("\tcmpl\t$0, %%eax\t\t\n");
    emit("\tje  \t.L%d\n", e2_label);
    __code_gen(root->then);
    emit("\tjmp \t.L%d\n", end_label);
    emit(".L%d:\n", e2_label);
    __code_gen(root->els);
    emit(".L%d:\n", end_label);
}

static void gen_postfix(AST *root){
    /* check if operand is lvalue. Identifier is the only legal lvalue at the moment */
    if(root->expr->type != AST_VAR){ 
        error("Expression is not an modifiable lvalue.\n");
    }
    List *v;
    if(!(v = search_all_scope(root->expr->vname))){
        error("Variable \"%s\" not declared\n", root->expr->vname);
    }
    int off = ((id_entry*)v->val)->var_address;
    emit("\tmovl\t-%d(%%rbp), %%eax\n", off);
    if(root->uop == PUNCT_INC)
        emit("\taddl\t$1, -%d(%%rbp)\n", off);
    else
        emit("\tsubl\t$1, -%d(%%rbp)\n", off);
}

static void gen_prefix(AST *root){
    /* check if operand is lvalue. Identifier is the only legal lvalue at the moment */
    if(root->expr->type != AST_VAR){ 
        error("Expression is not an modifiable lvalue.\n");
    }
    List *v;
    if(!(v = search_all_scope(root->expr->vname))){
        error("Variable \"%s\" not declared\n", root->expr->vname);
    }
    int off = ((id_entry*)v->val)->var_address;
    if(root->uop == PUNCT_INC)
        emit("\taddl\t$1, -%d(%%rbp)\n", off);
    else
        emit("\tsubl\t$1, -%d(%%rbp)\n", off);
    emit("\tmovl\t-%d(%%rbp), %%eax\n", off);
}

static void gen_assign(AST *root){
    List *v;
    if(!(v = search_all_scope(root->lexpr->vname))){
        error("Variable \"%s\" not declared\n", root->lexpr->vname);
    }
    int off = ((id_entry*)v->val)->var_address;
    __code_gen(root->rexpr);
    emit("\tmovl\t%%eax, -%d(%%rbp)\t\t#Save value to variable\n", off);
    /* According to C99 standard 6.15.6:  An assignment expression has the value
     * of the left operand after the assignment, but is not an lvalue. */
    emit("\tmovl\t-%d(%%rbp), %%eax\n", off);
}

static void gen_var_decl(AST* root){
    if(search_curr_scope(root->vname)){
        error("Redeclaration of variable \"%s\"\n", root->vname);
    }
    int offset = *((int*)curr_scope->table->val);
    int var_size = 4;
    *((int*)curr_scope->table->val) += align_8(var_size);
    symtab_enter(curr_scope, root->vname, offset);
    emit("\tsub \t$%d, %%rsp\t\t#Var declare Update new stack pointer\n",align_8(var_size));
    if(root->init){
        __code_gen(root->init);
        // Save on stack
        emit("\tmovl\t%%eax, -%d(%%rbp)\n", offset);
    }
}

static void gen_var(AST* root){
    List *v;
    if(!(v = search_all_scope(root->vname))){
        error("Variable \"%s\" not declared\n", root->vname);
    }
    int off = ((id_entry*)v->val)->var_address;
    // Get value from stack
    emit("\tmovl\t-%d(%%rbp), %%eax\n", off);
}

static int get_label(){
    return label_count++;
}

static void gen_func(AST *root){
    int ret = 0;
    emit("\t.globl %s\n", root->fname);
    emit("%s:\n", root->fname);
    // emit function prologue
    emit("\tpush\t%%rbp\t\t#Function prologue\n");
    emit("\tmov \t%%rsp, %%rbp\t\t#Function prologue\n");
    /* gen compound */
    ret = gen_compound(root->stmt);
    // emit function epilogue
    /*
     * Main function with no return statement, C99 standard 5.1.2.2.3 Program termination:  */
    /*If the return type of the main function is a type compatible with int, a return from the
initial call to the main function is equivalent to calling the exit function with the value
returned by the main function as its argument;10) reaching the } that terminates the
main function returns a value of 0. */
    if(!ret){
        emit("\tmov \t%%rbp, %%rsp\t\t#Function epilogue\n");
        emit("\tpop \t%%rbp\t\t#Function epilogue\n");
        emit("\tmovl\t$0, %%eax\n");
        emit("\tret \t#Function epilogue\n");
    }
}

static void gen_ret(AST *root){
    __code_gen(root->retval);
    emit("\tmov \t%%rbp, %%rsp\t\t#Function epilogue\n");
    emit("\tpop \t%%rbp\t\t#Function epilogue\n");
    emit("\tret \t#Function epilogue\n");
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
    emit("\tpop \t%%rcx\t\t#Pop left expression result\n");
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
            error("Undefined no order binary operator\n");
    }
}
static void gen_binary_ordered(AST *root){
    __code_gen(root->rexpr);
    emit("\tpush\t%%rax\t\t#Push right expression result onto stack\n");
    __code_gen(root->lexpr);
    emit("\tpop \t%%rcx\t\t#Pop right expression result\n");
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
            error("Undefined ordered binary operator\n");
    }
}
static void __code_gen(AST *root) {
    switch(root->type) {
        case AST_NOP:
            break;
        case AST_FUNC:
            gen_func(root);
            break;
        case AST_COMPOUND:
            gen_compound(root);
            break;
        case AST_IF:
            gen_if(root);
            break;
        case AST_TENARY:
            gen_tenary(root);
            break;
        case AST_RET:
            gen_ret(root);
            break;
        case AST_LITERAL:
            gen_literal(root);
            break;
        case AST_UNARY:
            switch(root->uop) {
                case '-': /* Neg */
                    __code_gen(root->expr);
                    gen_neg(root);
                    break;
                case '!': /* logical not */
                    __code_gen(root->expr);
                    gen_not(root);
                    break;
                case '~': /* bitwise complement */
                    __code_gen(root->expr);
                    gen_complement(root);
                    break;
                case PUNCT_INC:
                case PUNCT_DEC:
                    gen_prefix(root);
                    break;
                default:
                    error("Unknown unary operator\n");
            }
            break;
        case AST_POST_UNARY:
            switch(root->uop){
                case PUNCT_INC:
                case PUNCT_DEC:
                    gen_postfix(root);
                    break;
                default:
                    error("Unknown post unary operator\n");
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
                case '=':
                    gen_assign(root);
                    break;
                default:
                    error("Code gen: Unkown binary operator\n");
            }
            break;
        case AST_VAR:
            gen_var(root);
            break;
        case AST_VAR_DECL:
            gen_var_decl(root);
            break;
        default:
            error("Code gen error: Invalid AST type\n");
    }
}

void code_gen(char *filename, AST *ast) {
    char *str = (char*) malloc(sizeof(char) * (strlen(filename) + 3));
    strncpy(str, filename, strlen(filename));
    strcat(str, ".s");
    
    // Open out file
    outfile = fopen(str, "w");
    if(!outfile) {
        error("Open output file error\n");
    }

    __code_gen(ast);
    free(str);
    fclose(outfile);
}

