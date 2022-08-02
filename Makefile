CC = gcc
CFLAGS = -Wall -g
OBJECTS = lexer.o
LEX_OBJECTS = lexer.o test_lex.o

all: test_lex.out

test_lex.out : $(LEX_OBJECTS)
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -o $@ $^

%.o : %.c
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean 
clean:
	rm -rf *.o *.out
	

