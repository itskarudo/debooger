SRC = breakpoints.c commands.c  disassembler.c eval.c lexer.c main.c parser.c 
CFLAGS = -I/usr/include/capstone -fsanitize=address,leak -ggdb
LIBS = -lcapstone

debooger: $(SRC)
	gcc -o $@ $(CFLAGS) $(LIBS) $^
