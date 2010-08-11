CC=clang

all: bogart.o example.o trie.o example

bogart.o: bogart.c bogart.h
	$(CC) -ggdb -c bogart.c -o bogart.o -I/Users/tyler/Code/hiredis

trie.o: trie.c trie.h
	$(CC) -ggdb -c trie.c -o trie.o

example.o: example.c
	$(CC) -ggdb -c example.c -o example.o -I/Users/tyler/Code/hiredis

example: example.o bogart.o
	$(CC) -ggdb -o example bogart.o trie.o example.o -levent -L/Users/tyler/Code/hiredis -lhiredis

clean:
	rm *.o example
