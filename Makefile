CC = gcc
CFLAGS = -Wall -g
OBJECTS = lexer.o parser.o AST.o test.o code_gen.o
OUTFILE = test.out
TESTLEX = lexer.out
TESTLEXOBJ = lexer.o test_lexer.o
TESTPARSE = parser.out
TESTPARSEOBJ = lexer.o test_parser.o parser.o AST.o

all: $(OUTFILE) $(TESTLEX) $(TESTPARSE)

$(OUTFILE) : $(OBJECTS)
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -o $@ $^

$(TESTLEX) : $(TESTLEXOBJ)
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -o $@ $^

$(TESTPARSE) : $(TESTPARSEOBJ)
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -o $@ $^
	
%.o : %.c
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean run lex parse help
clean:
	rm -rf *.o *.out
run:
	@printf "[RUN] $(OUTFILE)\n"
	@./test.out test_input.c
lex:
	@printf "[TEST][$(TESTLEX)]\n"
	@./lexer.out test_input.c
parse:
	@printf "[TEST][$(TESTPARSE)]\n"
	@./parser.out test_input.c
help:
	@printf "\thelp\t--\tPrint out this message\n"
	@printf "\tlex\t--\tRun only the lexer with test_input.c as the input file\n"
	@printf "\tparse\t--\tRun the parser with test_input.c as the input file\n"
	@printf "\trun\t--\tRun the full compiler with test_input.c as the input file\n"
