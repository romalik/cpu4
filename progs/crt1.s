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
; info 1
get_rel_sp xl 4
get_rel_sp xh 5

; r = 0;
lit yl 0
; info 2

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
; info 3
jmp c $__crt_MULU1_c
;   if carry r += x;
jmp $__crt_MULU1_nc
__crt_MULU1_c:
; info 4
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
get_rel_sp a 8
alu shr
put_rel_sp a 8
get_rel_sp a 7
alu shrc
put_rel_sp a 7

jmp c $__crt_MULI2_c
;   if carry r += x;
jmp $__crt_MULI2_nc
__crt_MULI2_c:

get_rel_sp a 5
get_rel_sp b 3
alu add
put_rel_sp a 3

get_rel_sp a 6
get_rel_sp b 4
alu adc
put_rel_sp a 4

__crt_MULI2_nc:

;   x << 1;

get_rel_sp a 5
alu shl
put_rel_sp a 5
get_rel_sp a 6
alu shlc
put_rel_sp a 6

jmp $__crt_MULI2_loop

; }

__crt_MULI2_end:
ret

__crt_DIVI2:
__crt_MODI2:

ret