CC = gcc
CFLAGS = -Wall -g
OBJECTS = lexer.o parser.o test.o

all: test.out

test.out : $(OBJECTS)
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -o $@ $^

%.o : %.c
	@printf "[CC]\t$@\n"
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean 
clean:
	rm -rf *.o *.out
	

