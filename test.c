#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "femtoC.h"
#include "AST.h"
#include "code_gen.h"

static char *filename = NULL;
/* Other arguments can be implemented in the future */

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Usage: test.out  [filename]\n");
        exit(1);
    }
    /* Parse filename */
    int size = strlen(argv[1]);
    filename = malloc(sizeof(char) * (size + 1));
    char c;
    int i = 0;
    while((c = argv[1][i]) && c != '.'){
        filename[i++] = c;
    }
    /* Check input file type */
    if(c == '.' && ++i <= size &&  argv[1][i] != 'c'){
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
