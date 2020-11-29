start:
litw sl 0xffdd
call $main
halt
hlt_loop:
jmp $hlt_loop


fn_hello:
litw xl $msg
call $puts
ret

main:

echo_loop:

litw xl $msgprompt
call $puts

litw xl $in_str
call $gets

litw xl $msg2
call $puts

litw xl $in_str
call $puts

; put " and nl
lit a '"'
call $putc
lit a 0x0a
call $putc

litw xl $cmd_hello
litw yl $in_str
call $strcmp
lit b 0
cmp sub
jmp nz $skip_cmd_hello
call $fn_hello
skip_cmd_hello:

jmp $echo_loop

ret

cmd_hello:
.ascii 'hello'
.byte 0x00

msg:
.ascii 'Hello world!'
.byte 0x0a
.byte 0x00

msg2:
.ascii 'Recv string: "'
.byte 0x00

msgnl:
.byte 0x0a
.byte 0x00

msgprompt:
.ascii '> '
.byte 0x00

in_str:
.skip 128


