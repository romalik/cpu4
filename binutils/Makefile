CC=gcc -O0 -g


all: cpu4as cpu4ld crt0.o crt1.o crt_end.o

crt0.o: crt0.s
	./cpu4as crt0.s crt0.o
	
crt1.o: crt1.s
	./cpu4as crt1.s crt1.o

crt_end.o: crt_end.s
	./cpu4as crt_end.s crt_end.o

cpu4as: asm.c labels.c token.c util.c
	$(CC) -o cpu4as asm.c labels.c token.c util.c

cpu4ld: link.c util.c labels.c
	$(CC) -o cpu4ld link.c util.c labels.c

install: all
	install -d /opt/cpu4/bin
	install -m 755 cpu4as /opt/cpu4/bin/
	install -m 755 cpu4ld /opt/cpu4/bin/
	install -d /opt/cpu4/lib
	install -m 755 crt0.o /opt/cpu4/lib/
	install -m 755 crt1.o /opt/cpu4/lib/
	install -m 755 crt_end.o /opt/cpu4/lib/


clean:
	-rm cpu4as
	-rm cpu4ld
	-rm crt0.o
	-rm crt1.o
	-rm crt_end.o
