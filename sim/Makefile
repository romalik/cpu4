CC=cc


all: cpu4sim


cpu4sim: sim.c 
	$(CC) -o cpu4sim sim.c

install: all
	install -d /opt/cpu4/bin
	install -m 755 cpu4sim /opt/cpu4/bin/

clean:
	-rm cpu4sim

