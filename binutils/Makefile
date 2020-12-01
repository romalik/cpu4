CC=gcc -O0 -g


all: cpu4as cpu4ld


cpu4as: asm.c labels.c token.c util.c
	$(CC) -o cpu4as asm.c labels.c token.c util.c

cpu4ld: link.c util.c labels.c
	$(CC) -o cpu4ld link.c util.c labels.c

install: all
	install -d /opt/cpu4/bin
	install -m 755 cpu4as /opt/cpu4/bin/
	install -m 755 cpu4ld /opt/cpu4/bin/

clean:
	-rm cpu4as
	-rm cpu4ld

