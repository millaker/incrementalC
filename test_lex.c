#include <stdio.h>
#include "lexer.h"
#include "femtoC.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Usage: test_lex.out  [filename]\n");
        exit(1);
    }
    Token *tok;
    file_init(argv[1]);
    while((tok = get_token()) && tok->type != EOFTYPE){
        print_token(tok);
        free_token(tok);
    }
    return 0;
}
