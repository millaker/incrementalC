#include "AST.h"
#include <stdlib.h>
#include <string.h>
#include "femtoC.h"
#include "lexer.h"

#define NEW_AST ((AST*) malloc(sizeof(AST)))

#define error(...) parser_errorf(__FILE__,__LINE__,__VA_ARGS__)

void parser_errorf(char *name, int line, char *fmt,...) {
    fprintf(stderr,"In %s:%d\n",name, line);
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr,fmt,arg);
    va_end(arg);
}

bool is_punct(Token *t, char c) {
    if(t->type != PUNCT || t->charval != c){
        unget_token(t);
        return FALSE;
    }
    return TRUE;
}

AST *ast_func(char *name, AST* stmt) {
    AST *n = NEW_AST;
    n->type = AST_FUNC;
    n-> stmt = stmt;
    char *tempstr = (char *) malloc(sizeof(char) * (strlen(name) + 1));
    strncpy(tempstr, name, strlen(name) + 1);
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

AST *read_intliteral() {
    Token *tok = get_token();
    if(tok->type != INTLITERAL) {
        unget_token(tok);
        error("Expect integer literal, but got %s\n", token_to_string(tok));
        exit(1);
    }
    return ast_int(tok->intval);
}

AST *read_expr() {
    return read_intliteral();
}

AST *read_return() {
    Token *tok = get_token();
    if(tok->type != IDENTIFIER || strcmp(tok->string, "return")){
        unget_token(tok);
        error("Expect keyword \"return\", but got %s\n", token_to_string(tok));
        exit(1);
    }
    //read expr
    AST *retval = read_expr();
    return ast_ret(retval);
}

AST *read_stmt() {
    AST *stmt = read_return();
    Token *tok = get_token();
    if(!is_punct(tok, ';')){
        error("Expect semicolon, but got %s\n", token_to_string(tok));
        unget_token(tok);
        exit(1);
    }
    return stmt;
}

AST *read_func_decl() {
    Token *tok = get_token();
    if(tok->type != IDENTIFIER || strcmp(tok->string, "int")){
        unget_token(tok);
        error("Expect keyword \"int\", but got %s\n", token_to_string(tok));
        exit(1);
    }
    //parse func name
    tok = get_token();
    Token *fname;
    if(tok->type == IDENTIFIER && strcmp(tok->string, "int")){
        fname = tok;
    } else {
        error("Expect function name, but got %s\n", token_to_string(tok));
        exit(1);
    }
    tok = get_token();
    if(!is_punct(tok, '(')){
        error("Expect \'(\', but got %s\n", token_to_string(tok));
        unget_token(tok);
        exit(1);
    }
    tok = get_token();
    if(!is_punct(tok, ')')){
        error("Expect \')\', but got %s\n", token_to_string(tok));
        unget_token(tok);
        exit(1);
    }
    tok = get_token();
    if(!is_punct(tok, '{')){
        error("Expect \'{\', but got %s\n", token_to_string(tok));
        unget_token(tok);
        exit(1);
    }

    AST *stmt = read_stmt();
    if(!stmt){
        error("Parse statement error\n");
        return NULL;
    }
    tok = get_token();
    if(!is_punct(tok, '}')){
        error("Expect punct \'}\', but got %s", token_to_string(tok));
        unget_token(tok);
        exit(1);
    }
    return ast_func(fname->string, stmt);
}

AST *read_program() {
    return read_func_decl();
}
