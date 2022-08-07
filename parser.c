#include "AST.h"
#include <stdlib.h>
#include <string.h>
#include "femtoC.h"
#include "lexer.h"
#include "list.h"

#define NEW_AST ((AST*) malloc(sizeof(AST)))

#define KEYWORD_SIZE 7
static const char *keyword_table[KEYWORD_SIZE] = {
    "int", "return", "for", "if", "while", "else", "void"
};

/* Higher level error messages can be generated
 * Initial thought: Store error messages in a buffer and generate them when neededj*/

/* Forward declaration */
AST *ast_uop(int op, AST *expr);
AST *ast_bop(int op, AST *left, AST *right);
AST *ast_func(char *name, List* stmt_list);
AST *ast_ret(AST *retval);
AST *ast_int(int val);
AST *ast_var_decl(char* name);
AST *ast_var(char *name);

List *read_stmtlist();
AST *read_identifier();
AST *read_assign();
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
AST *read_var_decl();
AST *read_identifier();


bool is_keyword();
#define error(...) parser_errorf(__FILE__,__LINE__,__VA_ARGS__)

void parser_errorf(char *name, int line, char *fmt,...) {
    fprintf(stderr,"In %s:%d\n",name, line);
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr,fmt,arg);
    va_end(arg);
    exit(1);
}

bool is_keyword(char *name){
    for(int i = 0; i < KEYWORD_SIZE; i++){
        if(!strcmp(name, keyword_table[i]))
            return TRUE;
    }
    return FALSE;
}

bool peek_token(int type){
    Token *t = get_token();
    unget_token(t);
    if(t->type == type)
        return TRUE;
    else
        return FALSE;
}

/* Only frees node, need to handle contents pointed to by val manually */
void free_list(List *l){
    List *curr = l->next;
    while(curr != l){
        List *f = curr;
        curr = curr->next;
        free(f);
    }
    free(curr);
}

bool peek_assign(){
    List *lookahead = NEW_LIST;
    bool has_assign = FALSE;
    Token *t;
    while((t = get_token()) && t->type != EOF){
        list_insert_head(lookahead, t);
        if(t->type == PUNCT && (t->charval == '=' || t->charval == PUNCT_ADD_ASSIGN ||
                    t->charval == PUNCT_SUB_ASSIGN || t->charval == PUNCT_DIV_ASSIGN ||
                    t->charval == PUNCT_MUL_ASSIGN || t->charval == PUNCT_BIT_AND_ASSIGN ||
                    t->charval == PUNCT_BIT_OR_ASSIGN || t->charval == PUNCT_BIT_XOR_ASSIGN)) { /* Other assign operators can be here */
            has_assign = TRUE;
            break;
        }else if(t->type == PUNCT && t->charval == ';'){
            break;
        }
    }
    for_each_node_unsafe(lookahead,ptr){
        unget_token((Token*)ptr->val);
    }
    free_list(lookahead);
    return has_assign;
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

AST *ast_var(char *name){
    AST *n = NEW_AST;
    n->type = AST_VAR;
    char *tempstr = (char*) malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(tempstr, name);
    n->vname = tempstr;
    n->init = NULL;
    return n;
}

AST *ast_var_decl(char* name){
    AST *n = NEW_AST;
    n->type = AST_VAR_DECL;
    char *tempstr = (char*) malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(tempstr, name);
    n->vname = tempstr;
    n->init = NULL;
    return n;
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

AST *ast_func(char *name, List *stmt_list) {
    AST *n = NEW_AST;
    n->type = AST_FUNC;
    n-> stmt = stmt_list;
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
        AST *right = read_factor();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
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
    if((ast = read_identifier())){
        return ast;
    }
    return NULL;
}

AST *read_intliteral() {
    Token *tok = get_token();
    if(tok->type != INTLITERAL) {
        unget_token(tok);
        return NULL;
    }
    return ast_int(tok->intval);
}

AST *read_uop() {
    AST *expr;
    if(is_punct('-') || is_punct('~') || is_punct('!')){
        Token *t = get_token();
        expr = read_factor();
        if(!expr)
            return NULL;
        return ast_uop(t->charval, expr);
    }
    return NULL;
}

AST *read_expr() {
    if(peek_token(IDENTIFIER) && peek_assign()){
        return read_assign();
    }
    return  read_log_OR();
}

AST *read_log_OR() {
    AST *res = read_log_AND();
    while(is_punct(PUNCT_LOG_OR)){
        Token *t = get_token();
        AST *right = read_log_AND();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}

AST *read_log_AND() {
    AST *res = read_bit_OR();
    while(is_punct(PUNCT_LOG_AND)){
        Token *t = get_token();
        AST *right = read_log_OR();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}

AST *read_bit_OR(){
    AST *res = read_bit_XOR();
    while(is_punct('|')){
        Token *t = get_token();
        AST *right = read_bit_XOR();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}

AST *read_bit_XOR(){
    AST *res = read_bit_AND();
    while(is_punct('^')){
        Token *t = get_token();
        AST *right = read_bit_AND();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}

AST *read_bit_AND(){
    AST *res = read_eq();
    while(is_punct('&')){
        Token *t = get_token();
        AST *right = read_eq();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}

AST *read_eq(){
    AST *res = read_rel();
    while(is_punct(PUNCT_EQ) || is_punct(PUNCT_NEQ)){
        Token *t = get_token();
        AST *right = read_rel();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}

AST *read_rel(){
    AST *res = read_bit_shift();
    while(is_punct(PUNCT_GTE) || is_punct(PUNCT_LTE) ||
          is_punct('>') || is_punct('<')){
        Token *t = get_token();
        AST *right =read_bit_shift();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}


AST *read_bit_shift(){
    AST *res = read_additive();
    while(is_punct(PUNCT_LSHIFT) || is_punct(PUNCT_RSHIFT)){
        Token *t = get_token();
        AST *right = read_additive();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}

AST *read_additive(){
    AST *res = read_term();
    while(is_punct('+') || is_punct('-')){
        Token *t = get_token();
        AST *right = read_term();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}

AST *read_return() {
    Token *tok = get_token();
    if(tok->type != IDENTIFIER || strcmp(tok->string, "return")){
        unget_token(tok);
        return NULL;
    }
    //read expr
    AST *retval = read_expr();
    if(!retval)
        return NULL;
    return ast_ret(retval);
}

AST *read_stmt() {
    AST *stmt = read_var_decl();
    if(!stmt)
        stmt = read_expr();
    if(!stmt)
        stmt = read_return();
    if(stmt){
        if(!is_punct(';')){
            error("Expect semicolon, but got %s\n", token_to_string(get_token()));
        }
        get_token();
    }
    return stmt;
}

AST *read_assign(){
    AST *var = read_identifier();
    if(!var)
        return NULL;
    if(!is_punct('=') && !is_punct(PUNCT_ADD_ASSIGN) && !is_punct(PUNCT_BIT_AND_ASSIGN)
            && !is_punct(PUNCT_BIT_OR_ASSIGN) && !is_punct(PUNCT_SUB_ASSIGN) 
            && !is_punct(PUNCT_BIT_XOR_ASSIGN) && !is_punct(PUNCT_MUL_ASSIGN)
            && !is_punct(PUNCT_DIV_ASSIGN)){
        error("Expect assign operator, but got %s\n", token_to_string(get_token()));
    }
    Token *op = get_token();
    AST *expr = read_expr();
    if(!expr)
        error("Missing assignment expression\n");
    // parse shorthand operations as binary ops and assign
    switch(op->charval){
        case PUNCT_ADD_ASSIGN:
            return ast_bop('=', var, ast_bop('+', var, expr));
        case PUNCT_MUL_ASSIGN:
            return ast_bop('=', var, ast_bop('*', var, expr));
        case PUNCT_DIV_ASSIGN:
            return ast_bop('=', var, ast_bop('/', var, expr));
        case PUNCT_BIT_AND_ASSIGN:
            return ast_bop('=', var, ast_bop('&', var, expr));
        case PUNCT_BIT_OR_ASSIGN:
            return ast_bop('=', var, ast_bop('^', var, expr));
        case PUNCT_SUB_ASSIGN:
            return ast_bop('=', var, ast_bop('-', var, expr));
        case PUNCT_BIT_XOR_ASSIGN:
            return ast_bop('=', var, ast_bop('^', var, expr));
        default:
            return ast_bop(op->charval, var, expr);
    }
}

AST *read_identifier(){
    Token *tok = get_token();
    if(tok->type != IDENTIFIER || is_keyword(tok->string)){
        unget_token(tok);
        return NULL;
    }
    return ast_var(tok->string);
}

AST *read_var_decl(){
    Token *tok = get_token();
    if(tok->type != IDENTIFIER || strcmp(tok->string, "int")){ //Will do type check
        unget_token(tok);
        return NULL;
    }
    AST *var = read_identifier();
    if(!var)
        error("Expect ID , but got %s\n", token_to_string(get_token()));
    AST *res = ast_var_decl(var->vname);
    if(!is_punct('=')){
        return res;
    }
    get_token();
    AST *init = read_expr();
    if(!init)
        error("Expect expression, but got %s\n", token_to_string(get_token()));
    res->init = init;
    return res;
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
    if(tok->type == IDENTIFIER && !is_keyword(tok->string)){
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

    List *stmt_list = read_stmtlist();
    if(list_is_empty(stmt_list)){
        error("Expected statement.\n");
    }
    if(!is_punct('}')){
        error("Expect punct \'}\', but got %s\n", token_to_string(get_token()));
    }
    get_token();
    return ast_func(fname->string, stmt_list);
}

AST *read_program() {
    return read_func_decl();
}

List *read_stmtlist(){
    List *l = NEW_LIST;
    AST *curr = NULL;
    while ((curr = read_stmt())){
        list_insert_tail(l, curr);
    }
    return l;
}
