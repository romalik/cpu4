/opt/cpu4/bin/cpu4as ./sh.s sh.o && /opt/cpu4/bin/cpu4as ./s_stdlib.s stdlib.o && /opt/cpu4/bin/cpu4ld sh.o stdlib.o -o sh.bin && hexdump -C ./sh.bin
