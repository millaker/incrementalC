#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "femtoC.h"
#include "AST.h"
#include "code_gen.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Usage: test.out  [filename]\n");
        exit(1);
    }
    /* Parse filename */
    char filename[32] = {0};
    char c;
    int i = 0;
    while((c = argv[1][i]) && c != '.'){
        filename[i++] = c;
    }
    //printf("Output filename : %s.s\n", filename);
    /* Check input file type */
    if(c == '.' && argv[1][++i] != 'c'){
        fprintf(stderr, "Wrong file type, .c file required\n");
        exit(1);
    }
    file_init(argv[1]);
    AST *program = read_program();
    //print_AST(program);
    code_gen(filename, program);
    //printf("Success\n");
    return 0;
}
