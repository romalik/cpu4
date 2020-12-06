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
__crt_MULU1:

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

jmp c $__crt_MULU1_c
;   if carry r += x;
jmp $__crt_MULU1_nc
__crt_MULU1_c:

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
__crt_MULI2:

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

jmp c $__crt_MULI2_c
;   if carry r += x;
jmp $__crt_MULI2_nc
__crt_MULI2_c:


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




; divide two 16-bit arguments
; 
; arg_1 = sp+2+2+1 =    sp+9 = N
; arg_2 = sp+2+2+2+1 =  sp+7 = D
; retval = sp+2+1 =     sp+5 = Q
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

;   if R ≥ D (y >= sp+5)
get_rel_sp b 8 ;  b <- Dh
seta yh
cmp sub

jmp l $__crt_DIVI2_end_cmp
jmp g $__crt_DIVI2_cmp_true

get_rel_sp b 7 ;  b <- Dl
seta yl
cmp sub
jmp ge $__crt_DIVI2_cmp_true
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

jmp c $__crt_DIVI2_end ; if I == 1, last shift will push 1 to carry, exit
jmp $__crt_DIVI2_loop
__crt_DIVI2_end:
ret

__crt_DIVI2:
call $__crt_divide_worker
ret

__crt_MODI2:
call $__crt_divide_worker
put_rel_sp_w y 3
ret