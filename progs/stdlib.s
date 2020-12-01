;.export putc
;.text
; function putc [0] 0 0
putc:
; alloc 0 for locals, 4 for spill
; offset sp by -4
seta sl
lit b 4
alu sub
puta sl
seta sh
lit b 0
alu sbc
puta sh
; offset end
; INDIRI1  {
lit off 7
seta m[so]
puta xl
; } INDIRI1() -> xl
;
; CNSTP2 16384 {
litw y 16384
; } CNSTP2(16384) -> y
;
; ASGNI1 1 {
seta xl
puta m[y]
; } ASGNI1(1) -> no_reg
;
; LABELV __label_1 {
__label_1:
; } LABELV(__label_1) -> 32 bit

;
; end function putc
; offset sp by 4
seta sl
lit b 4
alu add
puta sl
seta sh
lit b 0
alu adc
puta sh
; offset end
ret
;.export puts
; function puts [0] 0 2
puts:
; alloc 0 for locals, 4 for spill
; offset sp by -4
seta sl
lit b 4
alu sub
puta sl
seta sh
lit b 0
alu sbc
puta sh
; offset end
; JUMPV  {
jmp $__label_4
; } JUMPV() -> 32 bit

;
; LABELV __label_3 {
__label_3:
; } LABELV(__label_3) -> 32 bit

;
; INDIRP2  {
lit off 7
seta m[so]
puta b
lit off 8
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; INDIRI1  {
seta m[x]
puta xl
; } INDIRI1() -> xl
;
; CVII2 1 {
seta xl
lit b 0
puta xl
seta b
puta xh
; } CVII2(1) -> x
;
; ARGI2 2 {
pushw x
; sp +2
; } ARGI2(2) -> no_reg
;
; CALLV  {
; reserve retval
call $putc
; offset sp by 2
seta sl
lit b 2
alu add
puta sl
seta sh
lit b 0
alu adc
puta sh
; offset end
; } CALLV() -> 32 bit

;
; INDIRP2  {
lit off 7
seta m[so]
puta b
lit off 8
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; CNSTI2 1 {
litw y 1
; } CNSTI2(1) -> y
;
; ADDP2  {
; generic p->op = 304
; cmd: add
seta yl
puta b
seta xl
alu add
push a
; sp +1
seta yh
puta b
seta xh
alu adc
pop b
; sp +0
push a
; sp +1
push b
; sp +2
; } ADDP2() -> no_reg
;
; ASGNP2 2 {
popw a
; sp +0
lit off 7
puta m[so]
seta b
lit off 8
puta m[so]
; } ASGNP2(2) -> no_reg
;
; LABELV __label_4 {
__label_4:
; } LABELV(__label_4) -> 32 bit

;
; INDIRP2  {
lit off 7
seta m[so]
puta b
lit off 8
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; INDIRI1  {
seta m[x]
puta xl
; } INDIRI1() -> xl
;
; CVII2 1 {
seta xl
lit b 0
puta xl
seta b
puta xh
; } CVII2(1) -> x
;
; CNSTI2 0 {
litw y 0
; } CNSTI2(0) -> y
;
; NEI2 __label_3 {
seta yh
puta b
seta xh
cmp sub
jmp e $__jump_label__6
jmp s ne $__label_3
jmp $__jump_label_2__6
__jump_label__6:
seta yl
puta b
seta xl
cmp sub
jmp s ne $__label_3
__jump_label_2__6:
; } NEI2(__label_3) -> no_reg
;
; LABELV __label_2 {
__label_2:
; } LABELV(__label_2) -> 32 bit

;
; end function puts
; offset sp by 4
seta sl
lit b 4
alu add
puta sl
seta sh
lit b 0
alu adc
puta sh
; offset end
ret
;.export getc
; function getc [1] 0 0
getc:
; alloc 0 for locals, 4 for spill
; offset sp by -4
seta sl
lit b 4
alu sub
puta sl
seta sh
lit b 0
alu sbc
puta sh
; offset end
; CNSTP2 16384 {
litw x 16384
; } CNSTP2(16384) -> x
;
; INDIRI1  {
seta m[x]
puta xl
; } INDIRI1() -> xl
;
; CVII2 1 {
seta xl
lit b 0
puta xl
seta b
puta xh
; } CVII2(1) -> x
;
; RETI2  {
; >> Access frame!
; store x to frame as retval
lit off 7
seta xl
puta m[so]
lit off 8
seta xh
puta m[so]
; offset sp by 4
seta sl
lit b 4
alu add
puta sl
seta sh
lit b 0
alu adc
puta sh
; offset end
ret
; } RETI2() -> no_reg
;
; LABELV __label_7 {
__label_7:
; } LABELV(__label_7) -> 32 bit

;
; end function getc
; offset sp by 4
seta sl
lit b 4
alu add
puta sl
seta sh
lit b 0
alu adc
puta sh
; offset end
ret
;.export strcmp
; function strcmp [2] 7 0
strcmp:
; alloc 7 for locals, 4 for spill
; offset sp by -11
seta sl
lit b 11
alu sub
puta sl
seta sh
lit b 0
alu sbc
puta sh
; offset end
; INDIRP2  {
lit off 16
seta m[so]
puta b
lit off 17
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; ASGNP2 2 {
lit off 8
seta xl
puta m[so]
seta xh
lit off 9
puta m[so]
; } ASGNP2(2) -> no_reg
;
; INDIRP2  {
lit off 18
seta m[so]
puta b
lit off 19
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; ASGNP2 2 {
lit off 10
seta xl
puta m[so]
seta xh
lit off 11
puta m[so]
; } ASGNP2(2) -> no_reg
;
; CNSTI2 0 {
litw x 0
; } CNSTI2(0) -> x
;
; ASGNI2 2 {
lit off 5
seta xl
puta m[so]
seta xh
lit off 6
puta m[so]
; } ASGNI2(2) -> no_reg
;
; JUMPV  {
jmp $__label_10
; } JUMPV() -> 32 bit

;
; LABELV __label_9 {
__label_9:
; } LABELV(__label_9) -> 32 bit

;
; INDIRP2  {
lit off 8
seta m[so]
puta b
lit off 9
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; CNSTI2 1 {
litw y 1
; } CNSTI2(1) -> y
;
; ADDP2  {
; generic p->op = 304
; cmd: add
seta yl
puta b
seta xl
alu add
push a
; sp +1
seta yh
puta b
seta xh
alu adc
pop b
; sp +0
push a
; sp +1
push b
; sp +2
; } ADDP2() -> no_reg
;
; ASGNP2 2 {
popw a
; sp +0
lit off 8
puta m[so]
seta b
lit off 9
puta m[so]
; } ASGNP2(2) -> no_reg
;
; INDIRP2  {
lit off 8
seta m[so]
puta b
lit off 9
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; INDIRU1  {
seta m[x]
puta xl
; } INDIRU1() -> xl
;
; ASGNU1 1 {
seta xl
lit off 7
puta m[so]
; } ASGNU1(1) -> no_reg
;
; INDIRP2  {
lit off 10
seta m[so]
puta b
lit off 11
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; CNSTI2 1 {
litw y 1
; } CNSTI2(1) -> y
;
; ADDP2  {
; generic p->op = 304
; cmd: add
seta yl
puta b
seta xl
alu add
push a
; sp +1
seta yh
puta b
seta xh
alu adc
pop b
; sp +0
push a
; sp +1
push b
; sp +2
; } ADDP2() -> no_reg
;
; ASGNP2 2 {
popw a
; sp +0
lit off 10
puta m[so]
seta b
lit off 11
puta m[so]
; } ASGNP2(2) -> no_reg
;
; INDIRP2  {
lit off 8
seta m[so]
puta b
lit off 9
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; INDIRU1  {
seta m[x]
puta xl
; } INDIRU1() -> xl
;
; CVUI2 1 {
seta xl
lit b 0
puta xl
seta b
puta xh
; } CVUI2(1) -> x
;
; INDIRP2  {
lit off 10
seta m[so]
puta b
lit off 11
seta m[so]
puta yh
seta b
puta yl
; } INDIRP2() -> y
;
; INDIRU1  {
seta m[y]
puta yl
; } INDIRU1() -> yl
;
; CVUI2 1 {
seta yl
lit b 0
puta yl
seta b
puta yh
; } CVUI2(1) -> y
;
; SUBI2  {
; generic p->op = 320
; cmd: sub
seta yl
puta b
seta xl
alu sub
push a
; sp +1
seta yh
puta b
seta xh
alu sbc
pop b
; sp +0
push a
; sp +1
push b
; sp +2
; } SUBI2() -> no_reg
;
; ASGNI2 2 {
popw a
; sp +0
lit off 5
puta m[so]
seta b
lit off 6
puta m[so]
; } ASGNI2(2) -> no_reg
;
; INDIRI2  {
lit off 5
seta m[so]
puta b
lit off 6
seta m[so]
puta xh
seta b
puta xl
; } INDIRI2() -> x
;
; CNSTI2 0 {
litw y 0
; } CNSTI2(0) -> y
;
; NEI2 __label_14 {
seta yh
puta b
seta xh
cmp sub
jmp e $__jump_label__15
jmp s ne $__label_14
jmp $__jump_label_2__15
__jump_label__15:
seta yl
puta b
seta xl
cmp sub
jmp s ne $__label_14
__jump_label_2__15:
; } NEI2(__label_14) -> no_reg
;
; INDIRU1  {
lit off 7
seta m[so]
puta xl
; } INDIRU1() -> xl
;
; CVUI2 1 {
seta xl
lit b 0
puta xl
seta b
puta xh
; } CVUI2(1) -> x
;
; CNSTI2 0 {
litw y 0
; } CNSTI2(0) -> y
;
; NEI2 __label_12 {
seta yh
puta b
seta xh
cmp sub
jmp e $__jump_label__16
jmp s ne $__label_12
jmp $__jump_label_2__16
__jump_label__16:
seta yl
puta b
seta xl
cmp sub
jmp s ne $__label_12
__jump_label_2__16:
; } NEI2(__label_12) -> no_reg
;
; LABELV __label_14 {
__label_14:
; } LABELV(__label_14) -> 32 bit

;
; JUMPV  {
jmp $__label_11
; } JUMPV() -> 32 bit

;
; LABELV __label_12 {
__label_12:
; } LABELV(__label_12) -> 32 bit

;
; LABELV __label_10 {
__label_10:
; } LABELV(__label_10) -> 32 bit

;
; JUMPV  {
jmp $__label_9
; } JUMPV() -> 32 bit

;
; LABELV __label_11 {
__label_11:
; } LABELV(__label_11) -> 32 bit

;
; INDIRI2  {
lit off 5
seta m[so]
puta b
lit off 6
seta m[so]
puta xh
seta b
puta xl
; } INDIRI2() -> x
;
; RETI2  {
; >> Access frame!
; store x to frame as retval
lit off 14
seta xl
puta m[so]
lit off 15
seta xh
puta m[so]
; offset sp by 11
seta sl
lit b 11
alu add
puta sl
seta sh
lit b 0
alu adc
puta sh
; offset end
ret
; } RETI2() -> no_reg
;
; LABELV __label_8 {
__label_8:
; } LABELV(__label_8) -> 32 bit

;
; end function strcmp
; offset sp by 11
seta sl
lit b 11
alu add
puta sl
seta sh
lit b 0
alu adc
puta sh
; offset end
ret
;.export strcpy
; function strcpy [2] 5 0
strcpy:
; alloc 5 for locals, 4 for spill
; offset sp by -9
seta sl
lit b 9
alu sub
puta sl
seta sh
lit b 0
alu sbc
puta sh
; offset end
; INDIRP2  {
lit off 14
seta m[so]
puta b
lit off 15
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; ASGNP2 2 {
lit off 6
seta xl
puta m[so]
seta xh
lit off 7
puta m[so]
; } ASGNP2(2) -> no_reg
;
; INDIRP2  {
lit off 16
seta m[so]
puta b
lit off 17
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; ASGNP2 2 {
lit off 8
seta xl
puta m[so]
seta xh
lit off 9
puta m[so]
; } ASGNP2(2) -> no_reg
;
; LABELV __label_18 {
__label_18:
; } LABELV(__label_18) -> 32 bit

;
; INDIRP2  {
lit off 6
seta m[so]
puta b
lit off 7
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; CNSTI2 1 {
litw y 1
; } CNSTI2(1) -> y
;
; ADDP2  {
; generic p->op = 304
; cmd: add
seta yl
puta b
seta xl
alu add
push a
; sp +1
seta yh
puta b
seta xh
alu adc
pop b
; sp +0
push a
; sp +1
push b
; sp +2
; } ADDP2() -> no_reg
;
; ASGNP2 2 {
popw a
; sp +0
lit off 6
puta m[so]
seta b
lit off 7
puta m[so]
; } ASGNP2(2) -> no_reg
;
; INDIRP2  {
lit off 8
seta m[so]
puta b
lit off 9
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; CNSTI2 1 {
litw y 1
; } CNSTI2(1) -> y
;
; ADDP2  {
; generic p->op = 304
; cmd: add
seta yl
puta b
seta xl
alu add
push a
; sp +1
seta yh
puta b
seta xh
alu adc
pop b
; sp +0
push a
; sp +1
push b
; sp +2
; } ADDP2() -> no_reg
;
; ASGNP2 2 {
popw a
; sp +0
lit off 8
puta m[so]
seta b
lit off 9
puta m[so]
; } ASGNP2(2) -> no_reg
;
; INDIRP2  {
lit off 8
seta m[so]
puta b
lit off 9
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; INDIRI1  {
seta m[x]
puta xl
; } INDIRI1() -> xl
;
; ASGNI1 1 {
seta xl
lit off 5
puta m[so]
; } ASGNI1(1) -> no_reg
;
; INDIRP2  {
lit off 8
seta m[so]
puta b
lit off 9
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; INDIRI1  {
seta m[x]
puta xl
; } INDIRI1() -> xl
;
; INDIRP2  {
lit off 6
seta m[so]
puta b
lit off 7
seta m[so]
puta yh
seta b
puta yl
; } INDIRP2() -> y
;
; ASGNI1 1 {
seta xl
puta m[y]
; } ASGNI1(1) -> no_reg
;
; LABELV __label_19 {
__label_19:
; } LABELV(__label_19) -> 32 bit

;
; INDIRI1  {
lit off 5
seta m[so]
puta xl
; } INDIRI1() -> xl
;
; CVII2 1 {
seta xl
lit b 0
puta xl
seta b
puta xh
; } CVII2(1) -> x
;
; CNSTI2 0 {
litw y 0
; } CNSTI2(0) -> y
;
; NEI2 __label_18 {
seta yh
puta b
seta xh
cmp sub
jmp e $__jump_label__21
jmp s ne $__label_18
jmp $__jump_label_2__21
__jump_label__21:
seta yl
puta b
seta xl
cmp sub
jmp s ne $__label_18
__jump_label_2__21:
; } NEI2(__label_18) -> no_reg
;
; INDIRP2  {
lit off 14
seta m[so]
puta b
lit off 15
seta m[so]
puta xh
seta b
puta xl
; } INDIRP2() -> x
;
; RETP2  {
; >> Access frame!
; store x to frame as retval
lit off 12
seta xl
puta m[so]
lit off 13
seta xh
puta m[so]
; offset sp by 9
seta sl
lit b 9
alu add
puta sl
seta sh
lit b 0
alu adc
puta sh
; offset end
ret
; } RETP2() -> no_reg
;
; LABELV __label_17 {
__label_17:
; } LABELV(__label_17) -> 32 bit

;
; end function strcpy
; offset sp by 9
seta sl
lit b 9
alu add
puta sl
seta sh
lit b 0
alu adc
puta sh
; offset end
ret
