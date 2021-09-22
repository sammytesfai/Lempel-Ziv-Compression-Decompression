FLAGS=-Wall -Wextra -Werror -Wpedantic
CC=clang $(CFLAGS)

all	:	encode decode
encode.o:	encode.c
	$(CC) -c encode.c trie.c io.c bv.c word.c
decode.o:	decode.c
	$(CC) -c decode.c word.c io.c bv.c
encode	:	encode.o
	$(CC) -o encode encode.o trie.o io.o bv.o word.o
decode	:	decode.o
	$(CC) -o decode decode.o word.o io.o bv.o
clean	:
	rm -f encode decode encode.o trie.o word.o io.o bv.o decode.o
infer	:
	make clean; infer-capture -- make; infer-analyze -- make
