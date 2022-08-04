#include "lexer.h"
#include <stdio.h>
#include "femtoC.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: test.out  [filename]\n");
        exit(1);
    }
    file_init(argv[1]);
    Token *c;
    while((c = get_token()) && c->type != EOFTYPE){
        print_token(c);
    }
    return 0;
}
