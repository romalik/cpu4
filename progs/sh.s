start:
litw s 0xffdd
call $main
halt
hlt_loop:
jmp $hlt_loop


fn_hello:
litw x $msg
call $puts
ret

main:

echo_loop:

litw x $msgprompt
call $puts

litw x $in_str
call $gets

litw x $__msg2
call $puts

litw x $in_str
call $puts

; put " and nl
lit a '"'
call $putc
lit a 0x0a
call $putc

litw x $cmd_hello
litw y $in_str
call $strcmp
lit b 0
cmp sub
jmp nz $skip_cmd_hello
call $fn_hello
skip_cmd_hello:

jmp $echo_loop

ret

cmd_hello:
.ascii "hello\0"

;.lit
msg:
.ascii "Hello world!\n\0"

__msg2:
.ascii "Recv string: \"\0"

msgnl:
.byte 0x0a
.byte 0x00

msgprompt:
.ascii "> \0"

in_str:
.skip 128
