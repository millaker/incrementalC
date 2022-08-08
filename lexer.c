#include "femtoC.h"
#include "lexer.h"
#include "list.h"

#define new_token ((Token *) malloc(sizeof(Token)))

static FILE* infile = NULL;
static List *token_buf = NULL;
char curr_filename[33] = {0};
static int curr_line = 0;

void fail(char *fmt, ...){
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    exit(1);
}


/*
 * Init file descriptor
 * Return -1 if initialization failed
 * Return 0 if succeed
 */
bool file_init(char *s) {
    FILE *f = fopen(s, "r");
    if (!f) return FALSE;
    infile = f;
    strncpy(curr_filename, s, 32);
    token_buf = NEW_LIST;
    return TRUE;
}

static inline char mygetc(FILE *f){
    char c;
    c = fgetc(f);
    if(c == '\n')
        curr_line++;
    return c;
}

// Identifiers or reserved word
Token *lex_ident(char c) {
    Token *t = new_token;
    //Consume all the alphanumeric chars and store in buffer
    //Support up to 32 characters long identifier name
    char buffer[33] = {c};
    int i = 1;
    /* Ignores characters that exceeds limited length(32) */
    while(1){
        c = mygetc(infile);
        if((!isalnum(c) && c != '_') || c == EOF){
            ungetc(c, infile);
            break;
        }
        if(i >= 31)
            continue;
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    char *tempstr = (char*) malloc(sizeof(char) * (strlen(buffer) + 1));
    strncpy(tempstr, buffer, strlen(buffer) + 1);
    t->type = IDENTIFIER;
    t->string = tempstr;
    return t;
}

// Intliteral
Token *lex_int(char c) {
    Token *t = new_token;
    char buffer[33] = {c};
    int i = 1;
    while(1){
        c = mygetc(infile);
        if(!isdigit(c)){
            ungetc(c, infile);
            t->type = INTLITERAL;
            t->intval = atoi(buffer);
            return t;
        }
        if(i >= 31)
            continue;
        buffer[i++] = c;
        buffer[i] = '\0';
    }
}

Token *lex_punct(char c){
    Token *t = new_token;
    t->type = PUNCT;
    t->charval = c;
    switch(c){
        case '^':
            c = mygetc(infile);
            if(c == '=')
                t->charval = PUNCT_BIT_XOR_ASSIGN;
            else
                ungetc(c, infile);
            break;
        case '&':
            c = mygetc(infile);
            if(c == '&')
                t->charval = PUNCT_LOG_AND;
            else if(c == '=')
                t->charval = PUNCT_BIT_AND_ASSIGN;
            else
                ungetc(c, infile);
            break;
        case '|':
            c = mygetc(infile);
            if(c == '|'){
                t->charval = PUNCT_LOG_OR;
            }else if(c == '='){
                t->charval = PUNCT_BIT_OR_ASSIGN;
            }else {
                ungetc(c, infile);
            }
            break;
        case '=':
            c = mygetc(infile);
            if(c == '='){
                t->charval = PUNCT_EQ;
            } else {
                ungetc(c, infile);
            }
            break;
        case '!':
            c = mygetc(infile);
            if(c == '='){
                t->charval = PUNCT_NEQ;
            }else {
                ungetc(c, infile);
            }
            break;
        case '<':
            c = mygetc(infile);
            if(c == '<'){
                char c2 = mygetc(infile);
                if(c2 == '='){
                    t->charval = PUNCT_LSHIFT_ASSIGN;
                }else{
                    t->charval = PUNCT_LSHIFT;
                    ungetc(c2, infile);
                }
            } else if(c == '='){
                t->charval = PUNCT_LTE;
            } else {
                ungetc(c, infile);
            }
            break;
        case '>':
            c = mygetc(infile);
            if(c == '>') {
                char c2 = mygetc(infile);
                if(c2 == '='){
                    t->charval = PUNCT_RSHIFT_ASSIGN;
                }else{
                    ungetc(c2,infile);
                    t->charval = PUNCT_RSHIFT;
                }
            }else if(c == '='){
                    t->charval = PUNCT_GTE;
            }else{
                ungetc(c, infile);
            }
            break;
        case '+':
            c = mygetc(infile);
            if(c == '='){
                t->charval = PUNCT_ADD_ASSIGN;
            }else if(c == '+'){
                t->charval = PUNCT_INC;
            }else{
                ungetc(c, infile);
            }
            break;
        case '-':
            c = mygetc(infile);
            if(c == '='){
                t->charval = PUNCT_SUB_ASSIGN;
            }else if(c == '-'){
                t->charval = PUNCT_DEC;
            }else{
                ungetc(c, infile);
            }
            break;
        case '*':
            c = mygetc(infile);
            if(c == '='){
                t->charval = PUNCT_MUL_ASSIGN;
            }else{
                ungetc(c, infile);
            }
            break;
        case '/':
            c = mygetc(infile);
            if(c == '='){
                t->charval = PUNCT_DIV_ASSIGN;
            }else{
                ungetc(c, infile);
            }
            break;
        case '%':
            c = mygetc(infile);
            if(c == '='){
                t->charval = PUNCT_MODULO_ASSIGN;
            }else{
                ungetc(c, infile);
            }
            break;
        default:
            return t;
    }
    return t;
}

/*
 * Get next token
 * Return a pointer to Token struct
 */
Token *get_token() {
    if (!list_is_empty(token_buf)) {
        List *node = list_remove_head(token_buf);
        Token *t = (Token *) node->val;
        free(node);
        return t;
    }
    char c = mygetc(infile);
    //Skip white spaces and tabs
    if(c == ' ' || c == '\t' || c == '\n')
        return get_token();
    //EOF
    if (c == EOF) {
        Token *t = new_token;
        t->type = EOFTYPE;
        return t;
    }
    //Identifier
    if(isalpha(c) || c == '_'){
        return lex_ident(c);
    }
    // Int literal
    if (isdigit(c)) {
        return lex_int(c);
    }
    //PUNCT
    return lex_punct(c);
}

/*
 * Unget token
 */
void unget_token(Token *token) {
    list_insert_head(token_buf, token);
}


/* Look up table for multicharacter operators */
/* There is a same table in AST.c: drawing utilities */
/* Remember to update both table when needed*/ 
/* When adding new elements, update the table size as well */
static char *operator_table[] = {
    "&&", "||", "&=", "|=", "==", "!=", "<=", ">>", ">=", "<<", "++",
    "+=", "--", "-=", "*=", "/=", "^=", "<<=", ">>=", "%="
};

/*
 * For debugging
 * Print out token
 */
void print_token(Token* token) {
    switch(token->type){
        case EOFTYPE:
            printf("EOF ");
            break;
        case PUNCT:
            if(token->charval < 256)
                printf("PUNCT(%c) ", token->charval);
            else
                printf("PUNCT(%s) ", operator_table[token->charval - 256]);
            break;
        case IDENTIFIER:
            printf("ID(%s) ", token->string);
            break;
        case INTLITERAL:
            printf("INT(%d) ", token->intval);
            break;
        default:
            printf("UNKNOWN ");
    }
}


/*
 * Free Token Instance
 */
void free_token(Token *t) {
    if(t->type == IDENTIFIER)
        free(t->string);
    free(t);
}

char *token_to_string(Token *tok){
    char *tempstr = (char*) malloc(sizeof(char) * 32);
    switch(tok->type){
        case INTLITERAL:
            strcpy(tempstr, "integer literal");
            break;
        case IDENTIFIER:
            strcpy(tempstr, "Identifier:");
            strncat(tempstr,tok->string, 32);
            break;
        case PUNCT:
            strcpy(tempstr, "Punct: ");
            if(tok->charval > 255){
                strcat(tempstr, operator_table[tok->charval - 256]);
            }else{
                char c[2] = {tok->charval, 0};
                strcat(tempstr, c);
            }
            break;
        default:
            strcpy(tempstr, "Unknown type");
    }
    return tempstr;
}


