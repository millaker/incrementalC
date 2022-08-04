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
    exit(1);
}

bool peek_token(int type) {
    Token *t = get_token();
    unget_token(t);
    return t->type == type;
}


bool is_punct(Token *t, char c) {
    if(t->type != PUNCT || t->charval != c){
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

AST *read_intliteral() {
    Token *tok = get_token();
    if(tok->type != INTLITERAL) {
        unget_token(tok);
        error("Expect integer literal, but got %s\n", token_to_string(tok));
    }
    return ast_int(tok->intval);
}

AST *read_expr() {
    /* Read unary operator can be a separate function */
    if(peek_token(PUNCT)) {
        Token *t = get_token();
        AST *expr;
        if(is_punct(t, '~') || is_punct(t, '!') || is_punct(t, '-')){
            expr = read_expr();
            return ast_uop(t->charval, expr);
        }
        unget_token(t);
    }
    if(peek_token(INTLITERAL)){
        return read_intliteral();
    }
    error("Expect expression\n");
    return NULL;
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
    Token *tok = get_token();
    if(!is_punct(tok, ';')){
        unget_token(tok);
        error("Expect semicolon, but got %s\n", token_to_string(tok));
    }
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
    tok = get_token();
    if(!is_punct(tok, '(')){
        unget_token(tok);
        error("Expect \'(\', but got %s\n", token_to_string(tok));
    }
    tok = get_token();
    if(!is_punct(tok, ')')){
        unget_token(tok);
        error("Expect \')\', but got %s\n", token_to_string(tok));
    }
    tok = get_token();
    if(!is_punct(tok, '{')){
        unget_token(tok);
        error("Expect \'{\', but got %s\n", token_to_string(tok));
    }

    AST *stmt = read_stmt();
    if(!stmt){
        error("Parse statement error\n");
        return NULL;
    }
    tok = get_token();
    if(!is_punct(tok, '}')){
        unget_token(tok);
        error("Expect punct \'}\', but got %s", token_to_string(tok));
    }
    return ast_func(fname->string, stmt);
}

AST *read_program() {
    return read_func_decl();
}
