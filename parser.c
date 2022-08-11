#include "AST.h"
#include <stdlib.h>
#include <string.h>
#include "femtoC.h"
#include "lexer.h"
#include "list.h"

#define NEW_AST ((AST*) malloc(sizeof(AST)))

#define KEYWORD_SIZE 10
static const char *keyword_table[KEYWORD_SIZE] = {
    "int", "return", "for", "if", "while", "else", "void", "break", "continue", "do"
};

/* Higher level error messages can be generated
 * Initial thought: Store error messages in a buffer and generate them when neededj*/

/* Forward declaration */
AST *ast_uop(int op, AST *expr);
AST *ast_bop(int op, AST *left, AST *right);
AST *ast_func(char *name, AST* stmt_list, List *param);
AST *ast_ret(AST *retval);
AST *ast_int(int val);
AST *ast_var_decl(char* name);
AST *ast_var(char *name);
AST *ast_postuop(int op, AST *expr);
AST *ast_nop();
AST *ast_if(AST* cond, AST *then, AST *els);
AST *ast_tenary(AST *cond, AST *then, AST *els);
AST *ast_comp(List *comp_list);
AST *ast_for(AST* decl, AST *cond, AST *expr, AST *body);
AST *ast_while(AST *cond, AST *body);
AST *ast_do(AST *cond, AST *body);
AST *ast_break();
AST *ast_continue();
AST *ast_program();
AST *ast_func_call(char *name, List *arg);

AST *read_identifier();
AST *read_assign();
AST *read_intliteral();
AST *read_term();
AST *read_primary();
AST *read_uop();
AST *read_uexpr();
AST *read_postexpr();
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
AST *read_program();
AST *read_var_decl();
AST *read_identifier();
AST *read_selection();
AST *read_condexpr();
AST *read_compound();
AST *read_blockitem();
AST *read_exprstmt();
AST *read_jumpstmt();
AST *read_break();
AST *read_continue();
AST *read_iteration();
AST *read_while();
AST *read_do();
AST *read_for();
List *read_paramlist();
AST *read_param();
AST *read_func_decl();
List *read_func_arg();


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
  /* Current method is to look for assignment operator until the first
   * semicolon. This will lead to parsing error for if() while() for()
   * statements because no semicolon is required for their condition expression.
   * One way to try is to look for unmatching parentheses. I'll implement it tomorrow.
   */
    List *lookahead = NEW_LIST;
    bool has_assign = FALSE;
    int paren = 0;
    Token *t;
    while((t = get_token())){
        list_insert_head(lookahead, t);
        if(t->type == PUNCT && (t->charval == '=' || t->charval == PUNCT_ADD_ASSIGN ||
                    t->charval == PUNCT_SUB_ASSIGN || t->charval == PUNCT_DIV_ASSIGN ||
                    t->charval == PUNCT_MUL_ASSIGN || t->charval == PUNCT_BIT_AND_ASSIGN ||
                    t->charval == PUNCT_BIT_OR_ASSIGN || t->charval == PUNCT_BIT_XOR_ASSIGN ||
                    t->charval == PUNCT_MODULO_ASSIGN || t->charval == PUNCT_LSHIFT_ASSIGN ||
                    t->charval == PUNCT_RSHIFT_ASSIGN)) { /* Other assign operators can be here */
            has_assign = TRUE;
            break;
        }else {
            if(t->type == EOFTYPE)
                break;
            if(t->type == PUNCT && t->charval == ';'){
                break;
            }
            if(t->type == PUNCT && t->charval == '(')
                paren++;
            if(t->type == PUNCT && t->charval == ')')
                paren--;
            if(paren < 0)
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

AST *ast_func_call(char *name, List *arg){
    AST *n = NEW_AST;
    n->type = AST_FUNC_CALL;
    n->fname = name;
    n->param = arg;
    return n;
}

AST *ast_program(List *l){
    AST *n = NEW_AST;
    n->type = AST_PROGRAM;
    n->func_decl = l;
    return n;
}

AST *ast_for(AST* decl, AST *cond, AST *expr, AST *body){
    AST *n = NEW_AST;
    n->type = AST_FOR;
    n->fdecl = decl;
    n->fcond = cond;
    n->fexpr = expr;
    n->fbody = body;
    return n;
}

AST *ast_while(AST *cond, AST *body){
    AST *n = NEW_AST;
    n->type = AST_WHILE;
    n->wcond = cond;
    n->wbody = body;
    return n;
}

AST *ast_do(AST *cond, AST *body){
    AST *n = NEW_AST;
    n->type = AST_DO;
    n->wcond = cond;
    n->wbody = body;
    return n;
}

AST *ast_break(){
    AST *n = NEW_AST;
    n->type = AST_BREAK;
    return n;
}

AST *ast_continue(){
    AST *n = NEW_AST;
    n->type = AST_CONTINUE;
    return n;
}

AST *ast_tenary(AST *cond, AST *then, AST *els){
    AST *n = NEW_AST;
    n->type = AST_TENARY;
    n->cond = cond;
    n->then = then;
    n->els = els;
    return n;
}

AST *ast_comp(List *comp_list){
    AST *n = NEW_AST;
    n->type = AST_COMPOUND;
    n->comp_stmt = comp_list;
    return n;
}

AST *ast_if(AST *cond, AST *then, AST *els){
    AST *n = NEW_AST;
    n->type = AST_IF;
    n->cond = cond;
    n->then = then;
    n->els = els;
    return n;
}

AST *ast_nop(){
    AST *n = NEW_AST;
    n->type = AST_NOP;
    return n;
}

AST *ast_postuop(int op, AST *expr){
    AST *n = NEW_AST;
    n->type = AST_POST_UNARY;
    n->uop = op;
    n->expr = expr;
    return n;
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

AST *ast_func(char *name, AST *stmt_list, List *param) {
    AST *n = NEW_AST;
    n->type = AST_FUNC;
    n->stmt = stmt_list;
    n->param = param;
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
    AST *res = read_uexpr();
    //Parse more factors
    while(is_punct('*') || is_punct('/') || is_punct('%')){
        Token *t = get_token();
        AST *right = read_uexpr();
        if(!right || !res)
            return NULL;
        res = ast_bop(t->charval, res, right);
    }
    return res;
}

AST *read_uexpr(){
    AST *ast = read_uop();
    if(!ast){
        ast = read_postexpr();
    }
    return ast;
}

List *read_func_arg(){
    List *l = NEW_LIST;
    AST *ast;
    while((ast = read_expr())){
        list_insert_tail(l, ast);
        if(!is_punct(','))
            break;
        get_token();
    }
    return l;
}

/* currently a very broken practice, should allow recursive call to parse
 * postfix expression and do type checking afterwards
 */
AST *read_postexpr(){
    AST *ast = read_primary();
    if(!ast)
        return NULL;
    if(is_punct('(')){
        get_token();
        List *l = read_func_arg();
        if(!is_punct(')'))
            error("Expect closing parenthesis, got %s\n", token_to_string(get_token()));
        get_token();
        return ast_func_call(ast->vname, l);
    }
    while(is_punct(PUNCT_INC) || is_punct(PUNCT_DEC)){
        Token *t = get_token();
        if(!ast)
            return NULL;
        ast = ast_postuop(t->charval, ast);
    }
    return ast;
}

AST *read_primary() {
    //( expr )
    if(is_punct('(')){
        get_token();
        AST *expr = read_expr();
        if(!is_punct(')'))
            error("Expected ), but got %s\n", token_to_string(get_token()));
        get_token();
        return expr;
    }
    AST *ast;
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
    if(is_punct('-') || is_punct('~') || is_punct('!') 
            || is_punct(PUNCT_INC) || is_punct(PUNCT_DEC)){
        Token *t = get_token();
        expr = read_uexpr();
        if(!expr)
            return NULL;
        return ast_uop(t->charval, expr);
    }
    return NULL;
}

AST *read_iteration(){
    AST *ast = read_while();
    if(!ast)
        ast = read_for();
    if(!ast)
        ast = read_do();
    return ast;
}

AST *read_while(){
    Token *t = get_token();
    if(t->type != IDENTIFIER || strcmp("while", t->string)){
        unget_token(t);
        return NULL;
    }
    if(!is_punct('('))
        error("Expect \'(\' but got %s\n", token_to_string(get_token()));
    get_token();
    AST *cond = read_expr();
    if(!cond)
        error("Expect expression for while statement\n");
    if(!is_punct(')'))
        error("Expect \')\' but got %s\n", token_to_string(get_token()));
    get_token();
    AST *body = read_stmt();
    if(!body)
        error("Expect statement for while body\n");
    return ast_while(cond, body);
}

AST *read_do(){
    Token *t = get_token();
    if(t->type != IDENTIFIER || strcmp("do", t->string)){
        unget_token(t);
        return NULL;
    }
    AST *body = read_stmt();
    if(!body)
        error("Expect statement for do body\n");
    t = get_token();
    if(t->type != IDENTIFIER || strcmp("while", t->string)){
        unget_token(t);
        error("Expect \"while\" keyword, but got %s\n", token_to_string(get_token()));
    }
    if(!is_punct('('))
        error("Expect \'(\' but got %s\n", token_to_string(get_token()));
    get_token();
    AST *cond = read_expr();
    if(!cond)
        error("Expect condition expression for do statement\n");
    if(!is_punct(')'))
        error("Expect \')\' but got %s\n", token_to_string(get_token()));
    get_token();
    if(!is_punct(';'))
        error("Expect \';\' but got %s\n", token_to_string(get_token()));
    get_token();
    return ast_do(cond, body);
}

AST *read_for(){
    Token *t = get_token();
    if(t->type != IDENTIFIER || strcmp("for", t->string)){
        unget_token(t);
        return NULL;
    }
    if(!is_punct('('))
        error("Expect \'(\' but got %s\n", token_to_string(get_token()));
    get_token();
    AST *decl = read_var_decl();
    if(!decl){
        decl = read_expr();
        if(!is_punct(';'))
            error("Expect \';\' but got %s\n", token_to_string(get_token()));
        get_token();
    }
    AST *cond = read_expr();
    if(!is_punct(';'))
        error("Expect \';\' but got %s\n", token_to_string(get_token()));
    get_token();
    AST *expr = read_expr();
    if(!is_punct(')'))
        error("Expect \')\' but got %s\n", token_to_string(get_token()));
    get_token();
    AST *body = read_stmt();
    if(!body)
        error("Expect statement body for for\n");
    return ast_for(decl, cond, expr, body);
}

AST *read_jumpstmt(){
    AST *ast = read_return();
    if(!ast)
        ast = read_break();
    if(!ast)
        ast = read_continue();
    return ast;
}

AST *read_break(){
    Token *t = get_token();
    if(t->type != IDENTIFIER || strcmp("break", t->string)){
        unget_token(t);
        return NULL;
    }
    if(!is_punct(';')){
        error("Expect semicolon at the end of expression, got %s\n", token_to_string(t));
    }
    get_token();
    return ast_break();
}

AST *read_continue(){
    Token *t = get_token();
    if(t->type != IDENTIFIER || strcmp("continue", t->string)){
        unget_token(t);
        return NULL;
    }
    if(!is_punct(';')){
        error("Expect semicolon at the end of expression, got %s\n", token_to_string(t));
    }
    get_token();
    return ast_continue();
}

AST *read_exprstmt(){
    AST *ast = read_expr();
    if(ast && !is_punct(';')){
        error("Expect semicolon at the end of expression, got %s\n", token_to_string(get_token()));
    }
    if(!ast && is_punct(';')){
        get_token();
        return ast_nop();
    }else if(!ast){
        return ast;
    }
    get_token();
    return ast;
}

AST *read_expr() {
    if(peek_token(IDENTIFIER) && peek_assign()){
        return read_assign();
    }
    AST *ast = read_condexpr();
    return ast;
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
    if(!is_punct(';')){
        error("Expect semicolon at the end of the return statement\n");
    }
    get_token();
    return ast_ret(retval);
}

AST *read_stmt() {
    AST *stmt = read_selection();
    if(!stmt)
        stmt = read_iteration();
    if(!stmt)
        stmt = read_compound();
    if(!stmt)
        stmt = read_exprstmt();
    if(!stmt)
        stmt = read_jumpstmt();
    return stmt;
}

/* Update of peek assign is required */
AST *read_assign(){
    /* Need to be changed to unary expr and do type check at code gen */
    AST *var = read_identifier(); 
    if(!var)
        return NULL;
    if(!is_punct('=') && !is_punct(PUNCT_ADD_ASSIGN) && !is_punct(PUNCT_BIT_AND_ASSIGN)
            && !is_punct(PUNCT_BIT_OR_ASSIGN) && !is_punct(PUNCT_SUB_ASSIGN) 
            && !is_punct(PUNCT_BIT_XOR_ASSIGN) && !is_punct(PUNCT_MUL_ASSIGN)
            && !is_punct(PUNCT_DIV_ASSIGN) && !is_punct(PUNCT_MODULO_ASSIGN)
            && !is_punct(PUNCT_RSHIFT_ASSIGN) && !is_punct(PUNCT_LSHIFT_ASSIGN)){
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
            return ast_bop('=', var, ast_bop('|', var, expr));
        case PUNCT_SUB_ASSIGN:
            return ast_bop('=', var, ast_bop('-', var, expr));
        case PUNCT_BIT_XOR_ASSIGN:
            return ast_bop('=', var, ast_bop('^', var, expr));
        case PUNCT_MODULO_ASSIGN:
            return ast_bop('=', var, ast_bop('%', var, expr));
        case PUNCT_RSHIFT_ASSIGN:
            return ast_bop('=', var, ast_bop(PUNCT_RSHIFT, var, expr));
        case PUNCT_LSHIFT_ASSIGN:
            return ast_bop('=', var, ast_bop(PUNCT_LSHIFT, var, expr));
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
    AST *init = NULL;
    if(is_punct('=')){
        get_token();
        init = read_expr();
        if(!init)
            error("Expect expression, but got %s\n", token_to_string(get_token()));
    }
    res->init = init;
    if(!is_punct(';')){
        error("Expect semicolon at the end\n");
    }
    get_token();
    return res;
}

AST *read_param(){
    Token *t = get_token();
    if(t->type != IDENTIFIER || strcmp("int", t->string)){
        unget_token(t);
        return NULL;
    }
    AST *ast = read_identifier();
    if(!ast){
        char *n = malloc(sizeof(char) * 10 );
        strcpy(n, "anonymous");
        return ast_var(n);
    }
    return ast;
}

List *read_paramlist(){
    List *res = NEW_LIST;
    AST *var;
    while((var = read_param())){
        list_insert_tail(res, var);
        if(!is_punct(',')){
            break;
        }
        get_token();
    }
    return res;
}

AST *read_func_decl() {
    Token *tok = get_token();
    if(tok->type != IDENTIFIER || strcmp(tok->string, "int")){
        unget_token(tok);
        return NULL;
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
    List *param = read_paramlist();
    if(!is_punct(')')){
        error("Expect \')\', but got %s\n", token_to_string(get_token()));
    }
    get_token();

    if(is_punct(';')){
        /* Stmt will be NULL for function declaration */
        get_token();
        return  ast_func(fname->string, NULL, param);
    }
    AST *stmt_list = read_compound();
    if(!stmt_list){
        error("Expect function body, got %s\n", token_to_string(get_token()));
    }
    return ast_func(fname->string, stmt_list, param);
}

AST *read_program() {
    List *l = NEW_LIST;
    AST *ast;
    while((ast = read_func_decl())){
        list_insert_tail(l,ast);
    }
    if(list_is_empty(l)){
        error("No function error\n");
    }
    return ast_program(l);
}

AST *read_blockitem(){
    AST *ast = read_var_decl();
    if(!ast)
        ast = read_stmt();
    return ast;
}

AST *read_compound(){
    if(!is_punct('{'))
        return NULL; 
    get_token();
    List *l = NEW_LIST;
    AST *curr = NULL;
    while(1){
        curr = read_blockitem();
        if(!curr)
            break;
        list_insert_tail(l, curr);
    }
    if(!is_punct('}')){
        free_list(l);
        error("Expect closing bracket for compound statement, got %s\n", token_to_string(get_token()));
    }
    get_token();
    return ast_comp(l);
}

AST *read_condexpr(){
    AST *cond = read_log_OR();
    if(!is_punct('?'))
        return cond;
    get_token();
    if(!cond)
        error("Expect condition expression for tenary operator, got %s\n", token_to_string(get_token()));
    AST *e1 = read_expr();
    if(!e1)
        error("Expect expression 1 for tenary operator, got %s\n", token_to_string(get_token()));
    if(!is_punct(':')){
        error("Expect colon for tenary operator, got %s\n", token_to_string(get_token()));
    }
    get_token();
    AST *e2 = read_condexpr();
    if(!e2)
        error("Expect expression 2 for tenary operator, got %s\n", token_to_string(get_token()));
    return ast_tenary(cond, e1, e2);
}

AST *read_selection(){
    Token *t = get_token();
    if(t->type != IDENTIFIER || strcmp("if", t->string)){
        unget_token(t);
        return NULL;
    }
    if(!is_punct('(')){
        error("Expect punct \'(\', but got %s\n", token_to_string(get_token()));
    }
    get_token();
    AST *cond = read_expr();
    if(!cond)
        error("Expect expression for if condition\n");
    if(!is_punct(')')){
        error("Expect punct \')\', but got %s\n", token_to_string(get_token()));
    }
    get_token();
    AST *then = read_stmt();
    if(!then)
        error("Expect statement for if statement (then part)\n");
    t = get_token();
    if(t->type != IDENTIFIER || strcmp("else", t->string)){
        unget_token(t);
        return ast_if(cond, then, NULL);
    }
    AST *els = read_stmt();
    if(!els)
        error("Expect statement for if statement (else part)\n");
    return ast_if(cond, then, els);
}
