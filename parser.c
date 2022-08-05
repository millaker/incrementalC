#include "AST.h"
#include <stdlib.h>
#include <string.h>
#include "femtoC.h"
#include "lexer.h"

#define NEW_AST ((AST*) malloc(sizeof(AST)))

/* Forward declaration */
AST *ast_uop(int op, AST *expr);
AST *ast_bop(int op, AST *left, AST *right);
AST *ast_func(char *name, AST* stmt);
AST *ast_ret(AST *retval);
AST *ast_int(int val);

AST *read_intliteral();
AST *read_term();
AST *read_factor();
AST *read_uop();
AST *read_expr();
AST *read_log_OR();
AST *read_log_AND();
AST *read_bit_OR();
AST *read_bit_XOR();
AST *read_bit_AND();
AST *read_eq();
AST *read_rel();
AST *read_bit_shift();
AST *read_additive();
AST *read_return();
AST *read_stmt();
AST *read_func_decl();
AST *read_program();

#define error(...) parser_errorf(__FILE__,__LINE__,__VA_ARGS__)

void parser_errorf(char *name, int line, char *fmt,...) {
    fprintf(stderr,"In %s:%d\n",name, line);
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr,fmt,arg);
    va_end(arg);
    exit(1);
}

bool peek_token(int type) {
    Token *t = get_token();
    unget_token(t);
    return t->type == type;
}

/*
 * Peek the next token, won't consume the next token
 * Return True if token is PUNCT and same with input
 * Return FALSE otherwise
 */
bool is_punct(int c) {
    Token *t = get_token();
    unget_token(t);
    if(t->type != PUNCT || c != t->charval){
        return FALSE;
    }
    return TRUE;
}

AST *ast_uop(int op, AST *expr) {
    AST *n = NEW_AST;
    n->type = AST_UNARY;
    n->expr = expr;
    n->uop = op;
    return n;
}

AST *ast_bop(int op, AST *left, AST *right) {
    AST *n = NEW_AST;
    n->type = AST_BINARY;
    n->bop = op;
    n->lexpr = left;
    n->rexpr = right;
    return n;
}

AST *ast_func(char *name, AST* stmt) {
    AST *n = NEW_AST;
    n->type = AST_FUNC;
    n-> stmt = stmt;
    char *tempstr = (char *) malloc(sizeof(char) * (strlen(name) + 1));
    strncpy(tempstr, name, strlen(name) + 1);
    n->fname = tempstr;
    return n;
}

AST *ast_ret(AST *retval) {
    AST *n = NEW_AST;
    n->type = AST_RET;
    n->retval = retval;
    return n;
}

AST *ast_int(int val) {
    AST *n = NEW_AST;
    n->type = AST_LITERAL;
    n->intval = val;
    return n;
}

AST *read_term() {
    AST *res = read_factor();
    //Parse more factors
    while(is_punct('*') || is_punct('/') || is_punct('%')){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_factor());
    }
    return res;
}

AST *read_factor() {
    //( expr )
    if(is_punct('(')){
        get_token();
        AST *expr = read_expr();
        if(!is_punct(')'))
            error("Expected ), but got %s\n", token_to_string(get_token()));
        get_token();
        return expr;
    }
    // <UnaryOP> <factor>
    AST *ast;
    if((ast = read_uop())){
        return ast;
    }
    //Int literal
    if((ast = read_intliteral())){
        return ast;
    }
    error("Error factor\n");
    return NULL;
}

AST *read_intliteral() {
    Token *tok = get_token();
    if(tok->type != INTLITERAL) {
        unget_token(tok);
        error("Expect integer literal, but got %s\n", token_to_string(tok));
    }
    return ast_int(tok->intval);
}

AST *read_uop() {
    AST *expr;
    if(is_punct('-') || is_punct('~') || is_punct('!')){
        Token *t = get_token();
        expr = read_factor();
        return ast_uop(t->charval, expr);
    }
    return NULL;
}

AST *read_expr() {
    return read_log_OR();
}

AST *read_log_OR() {
    AST *res = read_log_AND();
    while(is_punct(PUNCT_LOG_OR)){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_log_AND());
    }
    return res;
}

AST *read_log_AND() {
    AST *res = read_bit_OR();
    while(is_punct(PUNCT_LOG_AND)){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_bit_OR());
    }
    return res;
}

AST *read_bit_OR(){
    AST *res = read_bit_XOR();
    while(is_punct('|')){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_bit_XOR());
    }
    return res;
}

AST *read_bit_XOR(){
    AST *res = read_bit_AND();
    while(is_punct('^')){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_bit_AND());
    }
    return res;
}

AST *read_bit_AND(){
    AST *res = read_eq();
    while(is_punct('&')){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_eq());
    }
    return res;
}

AST *read_eq(){
    AST *res = read_rel();
    while(is_punct(PUNCT_EQ) || is_punct(PUNCT_NEQ)){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_rel());
    }
    return res;
}

AST *read_rel(){
    AST *res = read_bit_shift();
    while(is_punct(PUNCT_GTE) || is_punct(PUNCT_LTE) ||
          is_punct('>') || is_punct('<')){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_bit_shift());
    }
    return res;
}


AST *read_bit_shift(){
    AST *res = read_additive();
    while(is_punct(PUNCT_LSHIFT) || is_punct(PUNCT_RSHIFT)){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_additive());
    }
    return res;
}

AST *read_additive(){
    AST *res = read_term();
    while(is_punct('+') || is_punct('-')){
        Token *t = get_token();
        res = ast_bop(t->charval, res, read_term());
    }
    return res;
}

AST *read_return() {
    Token *tok = get_token();
    if(tok->type != IDENTIFIER || strcmp(tok->string, "return")){
        unget_token(tok);
        error("Expect keyword \"return\", but got %s\n", token_to_string(tok));
    }
    //read expr
    AST *retval = read_expr();
    return ast_ret(retval);
}

AST *read_stmt() {
    AST *stmt = read_return();
    if(!is_punct(';')){
        error("Expect semicolon, but got %s\n", token_to_string(get_token()));
    }
    get_token();
    return stmt;
}

AST *read_func_decl() {
    Token *tok = get_token();
    if(tok->type != IDENTIFIER || strcmp(tok->string, "int")){
        unget_token(tok);
        error("Expect keyword \"int\", but got %s\n", token_to_string(tok));
    }
    //parse func name
    tok = get_token();
    Token *fname;
    if(tok->type == IDENTIFIER && strcmp(tok->string, "int")){
        fname = tok;
    } else {
        error("Expect function name, but got %s\n", token_to_string(tok));
    }
    if(!is_punct('(')){
        error("Expect \'(\', but got %s\n", token_to_string(get_token()));
    }
    get_token();
    if(!is_punct(')')){
        error("Expect \')\', but got %s\n", token_to_string(get_token()));
    }
    get_token();
    if(!is_punct('{')){
        error("Expect \'{\', but got %s\n", token_to_string(get_token()));
    }
    get_token();

    AST *stmt = read_stmt();
    if(!stmt){
        error("Parse statement error\n");
        return NULL;
    }
    if(!is_punct('}')){
        error("Expect punct \'}\', but got %s\n", token_to_string(get_token()));
    }
    get_token();
    return ast_func(fname->string, stmt);
}

AST *read_program() {
    return read_func_decl();
}
