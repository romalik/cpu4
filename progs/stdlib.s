
; print char in A
putc:
litw yl 0x4000
puta m[y]
ret


; print string pointed by X
puts:
lit b 0
puts_loop:
seta m[x]
cmp sub
jmp z $puts_end
call $putc
x++
jmp $puts_loop
puts_end:
ret

; read string from uart to mem X till newline
gets:
litw yl 0x4000
seta m[y]
; echo it back
puta m[y]

; skip if zero, read another
lit b 0
cmp sub
jmp z $gets

; stop if NL
lit b 0x0a
cmp sub
jmp z $gets_end

; put char to string, inc x
puta m[x]
x++

jmp $gets

gets_end:
; put termination zero, return
lit a 0
puta m[x]
ret


; compares strings pointed by X and Y, returns in A
strcmp:

; load *Y to B
seta m[y]
puta b

; load *X to A
seta m[x]

; A = A - B
alu sub

; diff found, quit
jmp nz $strcmp_end

; check if *X == 0 (and *Y == 0, in prev comparison)
; return if zero (A keeps 0)
seta m[x]
lit b 0
cmp sub
jmp z $strcmp_end

x++
y++
jmp $strcmp

strcmp_end:
ret

