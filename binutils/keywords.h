#ifndef KEYWORDS_H__
#define KEYWORDS_H__

/*
N	bin	sel	    alu	opcode

0	0	    A	    add	nop
1	1	    B	    sub	seta
2	10	  XL	  neg	puta
3	11	  XH	  shl	lit
4	100	  YL	  NOP	litw
5	101	  YH	  inc	push
6	110	  SL	  adc	pop
7	111	  SH	  sbc	pushw
8	1000	ZL	  not	popw
9	1001	ZH	  and	alu
a	1010	OFF	  or	cmp
b	1011	M[zo] xor	jmp
c	1100	M[z]	zer jmpx
d	1101	X[x]	NOP	ret
e	1110	M[y]	NOP	call
f	1111	M[s]	NOP	ext
*/





char * alu_args[16] = {
  "add",  
  "sub", 
  "neg", 
  "shl", 
  "NOP", 
  "inc", 
  "adc", 
  "sbc", 
  "not", 
  "and", 
  "or", 
  "xor", 
  "zero", 
  "NOP", 
  "NOP", 
  "NOP"

};
char * args[16] = {
  "a",  
  "b", 
  "xl", 
  "xh", 
  "yl", 
  "yh", 
  "sl", 
  "sh", 
  "zl", 
  "zh", 
  "off", 
  "m[zo]", 
  "m[z]", 
  "m[x]", 
  "m[y]", 
  "m[s]"

};

char * opcodes_0[16] = {
  "nop",   /* 0 0 */
  "seta",  /* 1 0 */
  "puta",  /* 2 0 */
  "lit",   /* 3 1 */
  "litw",  /* 4 2 */
  "push",  /* 5 0 */
  "pop",   /* 6 0 */
  "pushw", /* 7 0 */
  "popw",  /* 8 0 */
  "alu",   /* 9 - */
  "cmp",   /* 10 -*/
  "jmp",   /* 11 2*/
  "jmpx",  /* 12 0*/
  "ret",   /* 13 0*/
  "call",  /* 14 2*/
  "ext"    /* 15 -*/
   
};
char * opcodes_3[16] = {
  "info",   /* 0 0 */
  "x++",   /* 1 0 */
  "y++",   /* 2 0 */
  "nop",   /* 3 1 */
  "nop",   /* 4 2 */
  "nop",   /* 5 0 */
  "nop",   /* 6 0 */
  "nop",   /* 7 0 */
  "nop",   /* 8 0 */
  "nop",   /* 9 - */
  "nop",   /* 10 -*/
  "nop",   /* 11 2*/
  "nop",   /* 12 0*/
  "nop",   /* 13 0*/
  "nop",  /* 14 2*/
  "halt"   /* 15 -*/
   
};

#endif
