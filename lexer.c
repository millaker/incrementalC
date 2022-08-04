#include "femtoC.h"
#include "lexer.h"

#define new_token ((Token *) malloc(sizeof(Token)))

static FILE* infile = NULL;
Token *token_buf = NULL;
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
    strncpy(curr_filename, s, strlen(s));
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

/*
 * Get next token
 * Return a pointer to Token struct
 */
Token *get_token() {
    if (token_buf) {
        Token *t = token_buf;
        token_buf = NULL;
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
    Token *t = new_token;
    t->type = PUNCT;
    t->charval = c;
    return t;
}

/*
 * Unget token
 */
void unget_token(Token *token) {
    token_buf = token;
}

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
            printf("PUNCT(%c) ", token->charval);
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
            tempstr[0] = '\'';
            tempstr[1] = tok->charval;
            tempstr[2] = '\'';
            tempstr[3] = '\0';
            break;
        default:
            strcpy(tempstr, "Unknown type");
    }
    return tempstr;
}

