#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
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




struct regs {
  uint8_t a;
  uint8_t b;
  uint8_t f;

  uint16_t s;
  uint16_t x;
  uint16_t y;
  uint16_t z;
  uint16_t p;
  uint8_t off;
  
  uint8_t ir;

  uint8_t sect;

};

int halt = 0;

uint8_t memory_image[64*1024];

uint8_t mem_read(uint16_t addr) {
  return memory_image[addr];
}

void mem_write(uint16_t addr, uint8_t val) {
  memory_image[addr] = val;
}

struct regs r;
#define ARG (r.ir & 0xf)
#define low(x) (((uint8_t *)&x)[0])
#define high(x) (((uint8_t *)&x)[1])

#define ZF ((r.f & 0x2) >> 1)
#define CF (r.f & 0x1)

uint16_t aluop() {


  uint16_t res;
  uint8_t Z;
  uint8_t C;
  uint16_t tmp;
  switch(ARG) {
    case 0:
      tmp = r.a + r.b;
      break;
    case 1:
      tmp = r.a - r.b;
      break;      
    case 2:
      tmp = -r.a;
      break;      
    case 3:
      tmp = r.a << 1;
      break;      
    case 4:
      break;      
    case 5:
      tmp = r.a++;      
      break;
    case 6:
      tmp = r.a + r.b + (r.f & 0x01);
      break;
    case 7:
      tmp = r.a - r.b - (r.f & 0x01);
      break;
    case 8:
      tmp = ~r.a;
      break;
    case 9:
      tmp = r.a & r.b;
      break;
    case 10:
      tmp = r.a | r.b;
      break;
    case 11:
      tmp = r.a ^ r.b;
      break;
    case 12:
      tmp = 0;
      break;
    default:
      break;

  }

  Z = ((tmp & 0xff) == 0)?1:0;
  C = ((tmp & 0xff00) == 0)?0:1;
  res = (Z << 9) | (C << 8) | (tmp & 0xff);
  return res;
}

uint8_t sel_get_value(uint8_t sel) {
  switch(sel) {
    case 0:
      return r.a;
    case 1:
      return r.b;
    case 2:
      return low(r.x);
    case 3:
      return high(r.x);
    case 4:
      return low(r.y);
    case 5:
      return high(r.y);
    case 6:
      return low(r.s);
    case 7:
      return high(r.s);
    case 8:
      return low(r.z);
    case 9:
      return high(r.z);
    case 10:
      return r.off;
    case 11:
      return mem_read(r.z + r.off);
    case 12:
      return mem_read(r.z);
    case 13:
      return mem_read(r.x);
    case 14:
      return mem_read(r.y);
    case 15:
      return mem_read(r.s);
    default:
      printf("Bad sel ir 0x%04x\n", r.ir);
      return r.b;
  }
}

void sel_set_value(uint8_t sel, uint8_t val) {
  switch(sel) {
    case 0:
      r.a = val;
      break;
    case 1:
      r.b = val;
      break;
    case 2:
      low(r.x) = val;
      break;
    case 3:
      high(r.x) = val;
      break;
    case 4:
      low(r.y) = val;
      break;
    case 5:
      high(r.y) = val;
      break;
    case 6:
      low(r.s) = val;
      break;
    case 7:
      high(r.s) = val;
      break;
    case 8:
      low(r.z) = val;
      break;
    case 9:
      high(r.z) = val;
      break;
    case 10:
      r.off = val;
      break;
    case 11:
      mem_write(r.z + r.off, val);
      break;
    case 12:
      mem_write(r.z, val);
      break;
    case 13:
      mem_write(r.x, val);
      break;
    case 14:
      mem_write(r.y, val);
      break;
    case 15:
      mem_write(r.s, val);
      break;
    default:
      printf("Bad sel ir 0x%04x\n", r.ir);
      break;
  }
}

void op_nop() {
  
}
void op_seta() {
  r.a = sel_get_value(ARG);  
}
void op_puta() {
  sel_set_value(ARG, r.a);  
}
void op_lit() {
  sel_set_value(ARG, mem_read(r.p));
  r.p++;
}
void op_litw() {
  //endiannes!!

  sel_set_value(ARG | 0x01, mem_read(r.p)); 
  r.p++;
  sel_set_value(ARG, mem_read(r.p)); 
  r.p++;
}
void op_push() {
  mem_write(r.s, sel_get_value(ARG)); 
  r.s--;
}
void op_pop() {
  r.s--;
  sel_set_value(ARG, mem_read(r.s));  
}
void op_pushw() {
  //endiannes!!

  mem_write(r.s, sel_get_value(ARG)); 
  r.s--;
  mem_write(r.s, sel_get_value(ARG | 0x01)); 
  r.s--;

}
void op_popw() {
  //endiannes!!
  r.s--;
  sel_set_value(ARG | 0x01, mem_read(r.s));  
  r.s--;
  sel_set_value(ARG, mem_read(r.s));  

}
void op_alu() {
  uint16_t tmp = aluop();
  r.a = low(tmp);
  r.f = high(tmp);  
}
void op_cmp() {
  uint16_t tmp = aluop();
  r.f = high(tmp);    
}

#define COND_MASK (((!CF) << 3) | (CF << 2) | ((!ZF) << 1) | (ZF))

void op_jmp() {
  uint8_t tmp_h;
  uint8_t tmp_l;

  tmp_h = mem_read(r.p);
  r.p++;
  tmp_l = mem_read(r.p);
  r.p++;

  if(COND_MASK & ARG) {
    low(r.p) = tmp_l;
    high(r.p) = tmp_h;
  }

}
void op_jmpx() {

  if(COND_MASK & ARG) {
    r.p = r.x;
  }
  
}
void op_ret() {
  r.s++;
  high(r.p) = mem_read(r.s);
  r.s++;
  low(r.p) = mem_read(r.s);
}

void op_call() {
  uint8_t tmp_h;
  uint8_t tmp_l;

  tmp_h = mem_read(r.p);
  r.p++;
  tmp_l = mem_read(r.p);
  r.p++;

  mem_write(r.s, low(r.p));
  r.s--;
  mem_write(r.s, high(r.p));
  r.s--;
  
  low(r.p) = tmp_l;
  high(r.p) = tmp_h;
}

void op_ext() {
  r.sect = ARG;
}
void op_err() {
  printf("bad op, sect=0x%02X ir=0x%02X\n", r.sect, r.ir);  
}

void op_sim_halt() {
  printf("Simulation halt instruction!\n");  
  halt = 1;
}

void op_sim_info() {
  printf(">>>>> SIM INFO %d!\n", ARG);  
  r.sect = 0;
}

void (*ops[])(void) = {
  /*sect 00*/
op_nop,   op_seta,  op_puta,  op_lit, op_litw,  op_push,  op_pop,   op_pushw,
op_popw,  op_alu,   op_cmp,   op_jmp, op_jmpx,  op_ret,   op_call,  op_ext,  

  /*sect 01*/
op_err,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_err,
op_err,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_err,

  /*sect 10*/
op_err,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_err,
op_err,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_err,

  /*sect 11*/
op_sim_info,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_err,
op_err,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_sim_halt,

};


void fetch() {
  r.ir = mem_read(r.p);
  r.p++;
}

void execute() {
  uint8_t opcode = (r.ir >> 4) & 0xf;
  opcode |= ((r.sect&0x3) << 4);
  (*ops[opcode])();
}

size_t cycle_counter = 0;
void cycle() {
  fetch();
  execute();
  cycle_counter++;
}

void print_state() {
  printf("==========\n");
  printf("Cycle %zu\n", cycle_counter);
  printf("A:\t0x%02X\t|\t", r.a);
  printf("B:\t0x%02X\t|\t", r.b);
  printf("F:\t0x%02X\n", r.f);
  printf("S:\t0x%04X\t|\t", r.s);
  printf("X:\t0x%04X\t|\t", r.x);
  printf("Y:\t0x%04X\n", r.y);
  printf("P:\t0x%04X\t|\t", r.p);
  printf("Z:\t0x%04X\t|\t", r.z);
  printf("off:\t0x%02X\n", r.off);
  printf("IR:\t0x%02X\t|\t", r.ir);
  printf("sect:\t0x%02X\n", r.sect);
  printf("==========\n");

}

/*

0 lit a 2
2 lit b 3
4 alu sum
5 litw s ff dd
8 call 00 19
11 seta xh
12 puta b
13 seta xl
14 alu sum
15 info 2
17 halt
fn: 19 info 1
21 ret
*/


uint8_t prog[] = {
  0x30, 0x02,
  0x31, 0x03,
  0x90,
  0x46, 0xff, 0xee,
  0xe0, 0x00, 19,
  0x13,
  0x21,
  0x12,
  0x90,
  0xf3, 0x02,
  0xf3, 0xf0,
  0xf3, 0x01,
  0xd0

};


int main() {
  int i;
  memset(memory_image, 0, 64*1024);

  memcpy(memory_image, prog, sizeof(prog));




  r.sect = 0;
  r.p = 0;

  while(!halt) {
    cycle();
    print_state();
    sleep(1);
  }

}