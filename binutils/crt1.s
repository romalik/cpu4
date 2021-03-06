
; 8bit mult
;
; r = 0;
; while(y) {
;   y >> 1;
;   if carry r += x;
;   x << 1;
; }
; arg_1 = sp+2+1+1 =    sp+4  -> xl
; arg_2 = sp+2+1+1+1 =  sp+5  -> xh
; retval = sp+2+1 =     sp+3  -> yl
;
.section text___crt_MULU1
__crt_MULU1:
.export __crt_MULU1
get_rel_sp xl 4
get_rel_sp xh 5

; r = 0;
lit yl 0


__crt_MULU1_loop:
; while(y) {

seta xh
lit b 0
cmp sub
jmp z $__crt_MULU1_end

;   y >> 1;
seta xh
alu shr
puta xh

;   if carry r += x;
jmp nc $__crt_MULU1_nc

seta xl
setb yl
alu add
puta yl
__crt_MULU1_nc:
;   x << 1;
seta xl
alu shl
puta xl
jmp $__crt_MULU1_loop

; }
__crt_MULU1_end:

put_rel_sp yl 3
ret


; multiply two 16-bit arguments
; 
; arg_1 = sp+2+2+1 =    sp+5
; arg_2 = sp+2+2+2+1 =  sp+7
; retval = sp+2+1 =     sp+3
;
; r = 0;
; while(y) {
;   y >> 1;
;   if carry r += x;
;   x << 1;
; }
;
;
.section text_MUL2
__crt_MULI2:
.export __crt_MULI2
__crt_MULU2:
.export __crt_MULU2

; r = 0;
litw a 0
put_rel_sp_w a 3

__crt_MULI2_loop:
; while(y) {

lit b 0
get_rel_sp a 8
alu sub
jmp ne $__crt_MULI2_continue
get_rel_sp a 7
alu sub
jmp e $__crt_MULI2_end


__crt_MULI2_continue:

;   y >> 1;

alus1 shr 8
alus1 shrc 7

jmp nc $__crt_MULI2_nc


alus2 add 5 3
alus2 adc 6 4

__crt_MULI2_nc:

;   x << 1;

alus1 shl 5
alus1 shlc 6

jmp $__crt_MULI2_loop

; }

__crt_MULI2_end:
ret


; multiply two 32-bit arguments
; 
; arg_1 (y)=            sp+11
; arg_2 (x)=            sp+7
; retval =              sp+3
;
; r = 0;
; while(y) {
;   y >> 1;
;   if carry r += x;
;   x << 1;
; }
;
;
.section text_MUL4

__crt_MULI4:
.export __crt_MULI4
__crt_MULU4:
.export __crt_MULU4

; r = 0;
litw a 0
put_rel_sp_w a 3
put_rel_sp_w a 5

__crt_MULI4_loop:
; while(y) {

lit b 0
get_rel_sp a 14
alu sub
jmp ne $__crt_MULI4_continue
get_rel_sp a 13
alu sub
jmp ne $__crt_MULI4_continue
get_rel_sp a 12
alu sub
jmp ne $__crt_MULI4_continue
get_rel_sp a 11
alu sub
jmp e $__crt_MULI4_end


__crt_MULI4_continue:

;   y >> 1;

alus1 shr 14
alus1 shrc 13
alus1 shrc 12
alus1 shrc 11

jmp nc $__crt_MULI4_nc


alus2 add 7 3
alus2 adc 8 4
alus2 adc 9 5
alus2 adc 10 6

__crt_MULI4_nc:

;   x << 1;

alus1 shl 7
alus1 shlc 8
alus1 shlc 9
alus1 shlc 10

jmp $__crt_MULI4_loop

; }

__crt_MULI4_end:
ret



; divide two 16-bit arguments
; 
; arg_1 = sp+2+2+1 =    sp+9 = N
; arg_2 = sp+2+2+2+1 =  sp+7 = D
; retval = sp+2+1 =     sp+5 = Q
; retaddr (front)       sp+3
; retaddr (this)        sp+1
;                          y = R
;                          x = I
;                       
;
; if D = 0 then error(DivisionByZeroException) end
; Q := 0                  -- Initialize quotient and remainder to zero
; R := 0                     
;  for i := n − 1 .. 0 do  -- Where n is number of bits in N
;   R := R << 1           -- Left-shift R by 1 bit
;   R(0) := N(i)          -- Set the least-significant bit of R equal to bit i of the numerator
;   if R ≥ D then
;     R := R − D
;     Q(i) := 1
;   end
;  end


.section text_crt_divide_worker
__crt_divide_worker:

; Q := 0                  -- Initialize quotient and remainder to zero
lit a 0
put_rel_sp a 5
put_rel_sp a 6

; R := 0                     
litw y 0

;  for i := n − 1 .. 0 do  -- Where n is number of bits in N
litw x 0x8000 ; I = 0b10000000 00000000
__crt_DIVI2_loop:

;   R := R << 1           -- Left-shift R by 1 bit
seta yl
alu shl
puta yl

seta yh
alu shlc
puta yh

;   R(0) := N(i)          -- Set the least-significant bit of R equal to bit i of the numerator
get_rel_sp a 10 ;  a <- Nh
setb xh        ;  b <- Ih
cmp and
jmp z $__crt_DIVI2_no_high_bit

y++ ; set R[0] = 1

jmp $__crt_DIVI2_end_set
__crt_DIVI2_no_high_bit:
get_rel_sp a 9 ;  a <- Nl
setb xl        ;  b <- Il
cmp and
jmp z $__crt_DIVI2_end_set

y++ ; set R[0] = 1

__crt_DIVI2_end_set:

;   if R ≥ D (y >= sp+7)
get_rel_sp b 8 ;  b <- Dh
seta yh
cmp sub

jmp b $__crt_DIVI2_end_cmp
jmp a $__crt_DIVI2_cmp_true

get_rel_sp b 7 ;  b <- Dl
seta yl
cmp sub
jmp ae $__crt_DIVI2_cmp_true
jmp $__crt_DIVI2_end_cmp

__crt_DIVI2_cmp_true:
; then

;     R := R − D
get_rel_sp b 7 ;  b <- Dl
seta yl
alu sub
puta yl
get_rel_sp b 8 ;  b <- Dh
seta yh
alu sbc
puta yh

;     Q(i) := 1
get_rel_sp a 5 ; a <- Ql
setb xl        ; b <- Il
alu or
put_rel_sp a 5
get_rel_sp a 6 ; a <- Qh
setb xh        ; b <- Ih
alu or
put_rel_sp a 6

;   end
__crt_DIVI2_end_cmp:

; I >> 1
seta xh
alu shr
puta xh
seta xl
alu shrc
puta xl

jmp nc $__crt_DIVI2_loop

ret


.section text_crt_DIVU2
__crt_DIVU2:
.export __crt_DIVU2
call $__crt_divide_worker
ret

__crt_MODU2:
.export __crt_MODU2
call $__crt_divide_worker
put_rel_sp_w y 3
ret




.section text_crt_negate_16
__negate_tos_16:
; op1h   +5
; op1l   +4
; retaddr
; retaddr
; zero   +1
lit a 0
push a
alus3 sub 4 1 4 ; zero - opl
alus3 sbc 5 1 5 ; zero - oph
adjust_sp s 1
ret

.section text_crt_DIVI2
__crt_DIVI2:
.export __crt_DIVI2
; reserve 9 bytes
; op1h   +23
; op1l   +20
; op2h   +19
; op2l   +16
; ret    +15
; ret    +12
; retaddr
; retaddr
; sgn    +9
; |op1h| +8
; |op1l| +5
; |op2h| +4
; |op2l| +1


; set sign counter to 0
lit xl 0
push xl
; now op1 is s+8
get_rel_sp_w a 8
pushw a

; now b = high of op1
lit a 0x80
cmp and
jmp z $__divi2_move_1_end

x++

call $__negate_tos_16

__divi2_move_1_end:

; now op2 is s+8

get_rel_sp_w a 8
pushw a

; now b = high of op2
lit a 0x80
cmp and
jmp z $__divi2_move_2_end

x++

call $__negate_tos_16

__divi2_move_2_end:

; alloc retval
pushw a

put_rel_sp xl 7

call $__crt_DIVU2

; now retval is s+10

get_rel_sp xl 7
lit a 1
setb xl

cmp and
jmp z $__divi2_move_retval

call $__negate_tos_16

__divi2_move_retval:

popw a
; now retval is s+8
put_rel_sp_w a 8

adjust_sp s 5

ret



.section text_crt_MODI2
__crt_MODI2:
.export __crt_MODI2
; reserve 9 bytes
; op1h   +23
; op1l   +20
; op2h   +19
; op2l   +16
; ret    +15
; ret    +12
; retaddr
; retaddr
; sgn    +9
; |op1h| +8
; |op1l| +5
; |op2h| +4
; |op2l| +1


; set sign counter to 0
lit xl 0
push xl
; now op1 is s+8
get_rel_sp_w a 8
pushw a

; now b = high of op1
lit a 0x80
cmp and
jmp z $__modi2_move_1_end

x++

call $__negate_tos_16

__modi2_move_1_end:

; now op2 is s+8

get_rel_sp_w a 8
pushw a

; now b = high of op2
lit a 0x80
cmp and
jmp z $__modi2_move_2_end

x++

call $__negate_tos_16

__modi2_move_2_end:

; alloc retval
pushw a

put_rel_sp xl 7

call $__crt_MODU2

; now retval is s+10

get_rel_sp xl 7
lit a 1
setb xl

cmp and
jmp z $__modi2_move_retval

call $__negate_tos_16

__modi2_move_retval:

popw a
; now retval is s+8
put_rel_sp_w a 8

adjust_sp s 5

ret








; divide two 32-bit arguments
; 
; arg_1 = sp+2+2+1 =    sp+21 = N
; arg_2 = sp+2+2+2+1 =  sp+17 = D
; retval = sp+2+1 =     sp+13 = Q
; retaddr (front)       sp+11
; retaddr (this)        sp+9
;                       sp+5  = R
;                       sp+1  = I
;                       
;
; if D = 0 then error(DivisionByZeroException) end
; Q := 0                  -- Initialize quotient and remainder to zero
; R := 0                     
;  for i := n − 1 .. 0 do  -- Where n is number of bits in N
;   R := R << 1           -- Left-shift R by 1 bit
;   R(0) := N(i)          -- Set the least-significant bit of R equal to bit i of the numerator
;   if R ≥ D then
;     R := R − D
;     Q(i) := 1
;   end
;  end

.section text_crt_divide32_worker
__crt_divide32_worker:

;alloc 2*4 bytes for I and R
seta sl
lit b 8
alu sub
puta sl
seta sh
lit b 0
alu sbc
puta sh


; Q := 0                  -- Initialize quotient and remainder to zero
litw a 0
put_rel_sp_w a 13
put_rel_sp_w a 15

; R := 0                     
put_rel_sp_w a 5
put_rel_sp_w a 7

;  for i := n − 1 .. 0 do  -- Where n is number of bits in N
put_rel_sp_w a 1
litw a 0x8000 ; I = 0b10000000 00000000 00000000 00000000
put_rel_sp_w a 3

__crt_DIVI4_loop:

;   R := R << 1           -- Left-shift R by 1 bit
alus1 shl  5
alus1 shlc 6
alus1 shlc 7
alus1 shlc 8


;   R(0) := N(i)          -- Set the least-significant bit of R equal to bit i of the numerator
lit xl 0
cmps2 and 24 4
jmp z $__crt_DIVI4_no_3_bit

x++

jmp $__crt_DIVI4_end_set
__crt_DIVI4_no_3_bit:


cmps2 and 23 3
jmp z $__crt_DIVI4_no_2_bit

x++

jmp $__crt_DIVI4_end_set
__crt_DIVI4_no_2_bit:


cmps2 and 22 2
jmp z $__crt_DIVI4_no_1_bit

x++

jmp $__crt_DIVI4_end_set
__crt_DIVI4_no_1_bit:


cmps2 and 21 1
jmp z $__crt_DIVI4_end_set

x++

jmp $__crt_DIVI4_end_set

__crt_DIVI4_end_set:

; set R[0] = x
setb xl
get_rel_sp a 5
alu or
put_rel_sp a 5


;   if R ≥ D (sp+5 >= sp+17)

cmps2 sub 20 8
jmp b $__crt_DIVI4_end_cmp
jmp a $__crt_DIVI4_cmp_true

cmps2 sub 19 7
jmp b $__crt_DIVI4_end_cmp
jmp a $__crt_DIVI4_cmp_true

cmps2 sub 18 6
jmp b $__crt_DIVI4_end_cmp
jmp a $__crt_DIVI4_cmp_true

cmps2 sub 17 5
jmp ae $__crt_DIVI4_cmp_true
jmp $__crt_DIVI4_end_cmp

__crt_DIVI4_cmp_true:
; then

;     R := R − D; r = sp+5   D = sp+17

alus2 sub 17 5
alus2 sbc 18 6
alus2 sbc 19 7
alus2 sbc 20 8


;     Q(i) := 1
; Q 13
; I 1
alus2 or 1 13
alus2 or 2 14
alus2 or 3 15
alus2 or 4 16

;   end
__crt_DIVI4_end_cmp:

; I >> 1
alus1 shr  4
alus1 shrc 3
alus1 shrc 2
alus1 shrc 1


jmp nc $__crt_DIVI4_loop

; R = sp+5
get_rel_sp_w x 5
get_rel_sp_w y 7


adjust_sp s 8
ret

;__crt_DIVI4:
.section text_crt_DIVU4

__crt_DIVU4:
.export __crt_DIVU4
call $__crt_divide32_worker
ret


.section text_crt_negate32
__negate_tos_32:
; op1h   +7
; op1
; op1
; op1l   +4
; retaddr
; retaddr
; zero   +1
lit a 0
push a
alus3 sub 4 1 4 ; zero - opl
alus3 sbc 5 1 5 ; zero - op
alus3 sbc 6 1 6 ; zero - op
alus3 sbc 7 1 7 ; zero - oph
adjust_sp s 1
ret


.section text_crt_DIVI4
__crt_DIVI4:
.export __crt_DIVI4
; reserve 9 bytes
; op1h   +23
; op1
; op1
; op1l   +20
; op2h   +19
; op2
; op2    
; op2l   +16
; ret    +15
; ret
; ret
; ret    +12
; retaddr
; retaddr
; sgn    +9
; |op1h| +8
; |op1 |
; |op1 |
; |op1l| +5
; |op2h| +4
; |op2 |
; |op2 |
; |op2l| +1


; set sign counter to 0
lit xl 0
push xl
; now op1 is s+12
get_rel_sp_w a 14
pushw a

; now op1 is s+14
get_rel_sp_w a 14
pushw a

get_rel_sp_w a 3
; now b = high of op1
lit a 0x80
cmp and
jmp z $__divi4_move_1_end

x++

call $__negate_tos_32

__divi4_move_1_end:

; now op2 is s+12 

get_rel_sp_w a 14
pushw a

; now op2 is s+14
get_rel_sp_w a 14
pushw a

get_rel_sp_w a 3
; now b = high of op2
lit a 0x80
cmp and
jmp z $__divi4_move_2_end

x++

call $__negate_tos_32

__divi4_move_2_end:

; alloc retval
pushw a
pushw a

put_rel_sp xl 13

call $__crt_DIVU4

; now retval is s+16

get_rel_sp xl 13
lit a 1
setb xl

cmp and
jmp z $__divi4_move_retval

call $__negate_tos_32

__divi4_move_retval:

popw a
; now retval is s+14
put_rel_sp_w a 14
popw a
; now retval is s+12
put_rel_sp_w a 14

adjust_sp s 9

ret

;__crt_MODI4:

.section text_crt_MODU4
__crt_MODU4:
.export __crt_MODU4
call $__crt_divide32_worker
put_rel_sp_w x 3
put_rel_sp_w y 5
ret


.section text_crt_MODI4
__crt_MODI4:
.export __crt_MODI4
; reserve 9 bytes
; op1h   +23
; op1
; op1
; op1l   +20
; op2h   +19
; op2
; op2    
; op2l   +16
; ret    +15
; ret
; ret
; ret    +12
; retaddr
; retaddr
; sgn    +9
; |op1h| +8
; |op1 |
; |op1 |
; |op1l| +5
; |op2h| +4
; |op2 |
; |op2 |
; |op2l| +1


; set sign counter to 0
lit xl 0
push xl
; now op1 is s+12
get_rel_sp_w a 14
pushw a

; now op1 is s+14
get_rel_sp_w a 14
pushw a

get_rel_sp_w a 3
; now b = high of op1
lit a 0x80
cmp and
jmp z $__modi4_move_1_end

x++

call $__negate_tos_32

__modi4_move_1_end:

; now op2 is s+12 

get_rel_sp_w a 14
pushw a

; now op2 is s+14
get_rel_sp_w a 14
pushw a

get_rel_sp_w a 3
; now b = high of op2
lit a 0x80
cmp and
jmp z $__modi4_move_2_end

x++

call $__negate_tos_32

__modi4_move_2_end:

; alloc retval
pushw a
pushw a

put_rel_sp xl 13
call $__crt_MODU4

; now retval is s+16

get_rel_sp xl 13
lit a 1
setb xl
cmp and
jmp z $__modi4_move_retval

call $__negate_tos_32

__modi4_move_retval:

popw a
; now retval is s+14
put_rel_sp_w a 14
popw a
; now retval is s+12
put_rel_sp_w a 14

adjust_sp s 9

ret

