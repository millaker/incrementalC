CC = gcc
CFLAGS = -Wall -g
OBJECTS = lexer.o parser.o AST.o test.o code_gen.o
OUTFILE = test.out

all: $(OUTFILE)

$(OUTFILE) : $(OBJECTS)
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -o $@ $^

%.o : %.c
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean run
clean:
	rm -rf *.o *.out
run:
	@printf "[RUN] $(OUTFILE)\n"
	@./test.out test_input.c
	

