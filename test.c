#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "femtoC.h"
#include "AST.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Usage: test_lex.out  [filename]\n");
        exit(1);
    }
    file_init(argv[1]);
    AST *program = read_program();
    print_AST(program);
    printf("Success, no seg fault\n");
    return 0;
}
