CC=cc


all: asm link


asm: asm.c labels.c token.c util.c
	$(CC) -o asm asm.c labels.c token.c util.c

link: link.c util.c labels.c
	$(CC) -o link link.c util.c labels.c




