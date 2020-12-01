#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <termios.h>
#include <pthread.h>

#include "cb.h"

int fl_debug = 0;


uint64_t start_time;

uint64_t gettime_ms() {
    struct timeval te; 
    gettimeofday(&te, NULL);
    uint64_t milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

struct termios oldtermios;

int ttyraw(int fd)
{
    struct termios newtermios;
    if(tcgetattr(fd, &oldtermios) < 0)
        return(-1);
    newtermios = oldtermios;

    newtermios.c_lflag &= ~(ECHO | ICANON | IEXTEN /*| ISIG*/);
    newtermios.c_iflag &= ~(BRKINT /*| ICRNL*/ | INPCK | ISTRIP | IXON);
    newtermios.c_cflag &= ~(CSIZE | PARENB);
    newtermios.c_cflag |= CS8;
    //newtermios.c_oflag &= ~(OPOST);
    newtermios.c_cc[VMIN] = 1;
    newtermios.c_cc[VTIME] = 0;
    if(tcsetattr(fd, TCSAFLUSH, &newtermios) < 0)
        return(-1);
    return(0);
}

int ttyreset(int fd)
{
    if(tcsetattr(fd, TCSAFLUSH, &oldtermios) < 0)
        return(-1);

    return(0);
}

int halt = 0;

//forward
void terminate();

void sigcatch(int sig)
{
    terminate();
    exit(0);
}


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

/* memory map
0x0000 - 0x3FFF ROM
0x4000 - 0x7FFF IO
0x8000 - 0xFFFF RAM
*/

struct Device {
  uint16_t range_start;
  uint16_t range_end;
  uint16_t size;
  void * data;
  void (*write)(struct Device * dev, uint16_t addr, uint8_t val);
  uint8_t (*read)(struct Device * dev, uint16_t addr);  
};




#define N_DEVICES 10
struct Device * devices[N_DEVICES];

struct Device rom;

void memory_device_write(struct Device * dev, uint16_t addr, uint8_t val) {
  ((uint8_t*)(dev->data))[addr - dev->range_start] = val;
}

uint8_t memory_device_read(struct Device * dev, uint16_t addr) {
  return ((uint8_t*)(dev->data))[addr - dev->range_start];
}


struct circular_buffer * uart_cb;
pthread_t uart_thread;
pthread_mutex_t uart_cb_lock;


void * uart_thread_worker(void * args) {
  int c;
  while(!halt) {
    read(0, &c, 1);
    pthread_mutex_lock(&uart_cb_lock);
    cb_push(uart_cb, c);
    pthread_mutex_unlock(&uart_cb_lock);
  }
  return NULL;
}


void uart_device_write(struct Device * dev, uint16_t addr, uint8_t val) {
  printf("%c", val);
  fflush(stdout);
}

uint8_t uart_device_read(struct Device * dev, uint16_t addr) {
  int c = 0;
  pthread_mutex_lock(&uart_cb_lock);
  if(uart_cb->size) {
    c = cb_pop(uart_cb);
  }
  pthread_mutex_unlock(&uart_cb_lock);
  
  return c;
}


struct Device * create_uart() {
  struct Device * uart;
  uart = (struct Device *)malloc(sizeof(struct Device));
  uart->size = 0x0001;
  uart->range_start = 0x4000;
  uart->range_end = 0x4000;
  uart->data = 0;
  uart->write = &uart_device_write;
  uart->read = &uart_device_read;
  
  uart_cb = cb_create(128);

  pthread_mutex_init(&uart_cb_lock, NULL);
  pthread_create(&uart_thread, NULL, uart_thread_worker, NULL);

  return uart;
}

void init_devices() {
  struct Device * ram;
  memset(devices, 0, sizeof(struct Device *) * N_DEVICES);

  rom.size = 0x4000;
  rom.range_start = 0x0000;
  rom.range_end = 0x3FFF;
  rom.data = malloc(rom.size);
  rom.write = &memory_device_write;
  rom.read = &memory_device_read;
  devices[0] = &rom;

  ram = (struct Device *)malloc(sizeof(struct Device));
  ram->size = 0x8000;
  ram->range_start = 0x8000;
  ram->range_end = 0xFFFF;
  ram->data = malloc(ram->size);
  ram->write = &memory_device_write;
  ram->read = &memory_device_read;
  devices[1] = ram;

  devices[2] = create_uart();



}

struct Device * select_device_by_addr(uint16_t addr) {
  int i;
  for(i = 0; i<N_DEVICES; i++) {
    if(devices[i]) {
      if(devices[i]->range_start <= addr && devices[i]->range_end >= addr) {
        return devices[i];
      }
    }
  }
  return NULL;
}

struct regs {
  uint8_t a;
  uint8_t b;
  uint8_t f;

  uint8_t s[2];
  uint8_t x[2];
  uint8_t y[2];
  uint8_t z[2];
  uint8_t p[2];
  uint8_t off;
  
  uint8_t ir;

  uint8_t sect;

};


void inc16(uint8_t r[2]) {
  r[0]++;
  if(!r[0]) {
    r[1]++;
  }
}
void dec16(uint8_t r[2]) {
  r[0]--;
  if(r[0] == 0xff) {
    r[1]--;
  }
}



uint8_t mem_read(uint16_t addr) {
  struct Device * dev;
  dev = select_device_by_addr(addr);
  if(dev) {
    return dev->read(dev, addr);
  }
  return 0;
}

void mem_write(uint16_t addr, uint8_t val) {
  struct Device * dev;
  dev = select_device_by_addr(addr);
  if(dev) {
    dev->write(dev, addr, val);
  }
}

struct regs r;
#define ARG (r.ir & 0xf)
#define low(x) (x[0])
#define high(x) (x[1])
#define val16(x) (x[0] | (x[1]<<8))
#define SF ((r.f & 0x4) >> 2)
#define ZF ((r.f & 0x2) >> 1)
#define CF (r.f & 0x1)

void aluop(uint8_t *res, uint8_t *flags) {


  uint8_t Z;
  uint8_t C;
  uint8_t S;
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
  S = ((tmp & 0x80))?1:0;
  *res = (tmp & 0xff);
  *flags = (S << 2) | (Z << 1) | (C);

  //printf("\n>>>> ALU\ntmp: %d 0x%04x\nZ: %d\nC: %d\nS: %d\nres: %d\nflags: %d\n>>>>\n\n", tmp, tmp, Z, C, S, *res, *flags);

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
      return mem_read(val16(r.s) + r.off);
    case 12:
      return mem_read(val16(r.z));
    case 13:
      return mem_read(val16(r.x));
    case 14:
      return mem_read(val16(r.y));
    case 15:
      return mem_read(val16(r.s));
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
      mem_write(val16(r.s) + r.off, val);
      break;
    case 12:
      mem_write(val16(r.z), val);
      break;
    case 13:
      mem_write(val16(r.x), val);
      break;
    case 14:
      mem_write(val16(r.y), val);
      break;
    case 15:
      mem_write(val16(r.s), val);
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
  sel_set_value(ARG, mem_read(val16(r.p)));
  inc16(r.p);
}
void op_litw() {
  //endiannes!!
  //read low first
  sel_set_value(ARG, mem_read(val16(r.p))); 
  inc16(r.p);
  sel_set_value(ARG | 0x01, mem_read(val16(r.p))); 
  inc16(r.p);
}
void op_push() {
  mem_write(val16(r.s), sel_get_value(ARG)); 
  dec16(r.s);
}
void op_pop() {
  inc16(r.s);
  sel_set_value(ARG, mem_read(val16(r.s)));  
}
void op_pushw() {
  //endiannes!!
  //push high first
  mem_write(val16(r.s), sel_get_value(ARG | 0x01)); 
  dec16(r.s);
  mem_write(val16(r.s), sel_get_value(ARG)); 
  dec16(r.s);

}
void op_popw() {
  //endiannes!!
  inc16(r.s);
  //pop low first
  sel_set_value(ARG, mem_read(val16(r.s)));  
  inc16(r.s);
  sel_set_value(ARG | 0x01, mem_read(val16(r.s)));  

}
void op_alu() {
  aluop(&r.a, &r.f);
}
void op_cmp() {
  uint8_t dummy;
  aluop(&dummy, &r.f);
}

//cond mask : czn

#define IS ((ARG & 0x8)>>3)
#define IC ((ARG & 0x4)>>2)
#define IZ ((ARG & 0x2)>>1)
#define IN ((ARG & 0x1))

void op_jmp() {
  uint8_t tmp_h;
  uint8_t tmp_l;
/*
  uint8_t cond_mask = 
      (((IS&SF)^(CF)) << 2)
    | ((ZF)           << 1)
    | (((IS&SF)^(NF)))
  ;
*/

  //printf("IS : %d\nSF: %d\nCF: %d\nZF: %d\n", IS, SF, CF, ZF);

  uint8_t f1 = ( (IS&SF) | (CF) );
  uint8_t f2 = ZF;

  uint8_t cond_mask = 
      ((f1)        << 2)
    | ((f2)        << 1)
    | (~(f1|f2)&0x01)
  ;


  //printf("cond mask: 0x%02X\n", cond_mask);
  //printf("arg  mask: 0x%02X\n", (ARG&0x7));

  tmp_l = mem_read(val16(r.p));
  inc16(r.p);
  tmp_h = mem_read(val16(r.p));
  inc16(r.p);



  if(cond_mask & (ARG&0x7)) {
    low(r.p) = tmp_l;
    high(r.p) = tmp_h;
  }

}

void op_ret() {
  inc16(r.s);
  low(r.p) = mem_read(val16(r.s));
  inc16(r.s);
  high(r.p) = mem_read(val16(r.s));
}

void op_call() {
  uint8_t tmp_h;
  uint8_t tmp_l;

  tmp_l = mem_read(val16(r.p));
  inc16(r.p);
  tmp_h = mem_read(val16(r.p));
  inc16(r.p);

  mem_write(val16(r.s), high(r.p));
  dec16(r.s);
  mem_write(val16(r.s), low(r.p));
  dec16(r.s);
  
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
  printf("Sim halt!\n");  
  halt = 1;
}

void print_state();

void op_sim_info() {
  printf(">>>>> SIM INFO %d!\n", mem_read(val16(r.p)));
  if(mem_read(val16(r.p)) == 255) { fl_debug = 1; }
  if(mem_read(val16(r.p)) == 254) { fl_debug = 0; }
  print_state();
  inc16(r.p);  
  r.sect = 0;


  sleep(1);
}

void op_x_pp() {
  inc16(r.x);
  r.sect = 0;
}

void op_x_mm() {
  dec16(r.x);
  r.sect = 0;
}

void op_y_pp() {
  inc16(r.y);
  r.sect = 0;
}

void op_y_mm() {
  dec16(r.y);
  r.sect = 0;
}

void op_s_pp() {
  inc16(r.s);
  r.sect = 0;
}

void op_s_mm() {
  dec16(r.s);
  r.sect = 0;
}

void (*ops[])(void) = {
  /*sect 00*/
op_nop,   op_seta,  op_puta,  op_lit, op_litw,  op_push,  op_pop,   op_pushw,
op_popw,  op_alu,   op_cmp,   op_jmp, op_err ,  op_ret,   op_call,  op_ext,  

  /*sect 01*/
op_err,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_err,
op_err,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_err,

  /*sect 10*/
op_err,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_err,
op_err,   op_err,   op_err,   op_err, op_err,   op_err,   op_err,   op_err,

  /*sect 11*/
op_x_pp,   op_x_mm,   op_y_pp,  op_y_mm,  op_s_pp,  op_s_mm,  op_err,      op_err,
op_err,    op_err,    op_err,   op_err,   op_err,   op_err,   op_sim_info, op_sim_halt,

};

char * ops_text[] = {
  /*sect 00*/
"op_nop",   "op_seta",  "op_puta",  "op_lit", "op_litw",  "op_push",  "op_pop",   "op_pushw",
"op_popw",  "op_alu",   "op_cmp",   "op_jmp", "op_err" ,  "op_ret",   "op_call",  "op_ext",  

  /*sect 01*/
"op_err",   "op_err",   "op_err",   "op_err", "op_err",   "op_err",   "op_err",   "op_err",
"op_err",   "op_err",   "op_err",   "op_err", "op_err",   "op_err",   "op_err",   "op_err",

  /*sect 10*/
"op_err",   "op_err",   "op_err",   "op_err", "op_err",   "op_err",   "op_err",   "op_err",
"op_err",   "op_err",   "op_err",   "op_err", "op_err",   "op_err",   "op_err",   "op_err",

  /*sect 11*/ 
"op_x_pp",   "op_x_mm",   "op_y_pp",  "op_y_mm",  "op_s_pp",  "op_s_mm",  "op_err",      "op_err",
"op_err",    "op_err",    "op_err",   "op_err",   "op_err",   "op_err",   "op_sim_info", "op_sim_halt",

};



void fetch() {
  r.ir = mem_read(val16(r.p));
  inc16(r.p);
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
  printf("==========");
  printf("==========");
  printf("==========");
  printf("==========");
  printf("==========");
  printf("==========\n");
  printf("Cycle %zu\n", cycle_counter);
  printf("A:\t0x%02X\t|\t", r.a);
  printf("B:\t0x%02X\t|\t", r.b);
  printf("F:\t0x%02X\n", r.f);
  printf("S:\t0x%04X\t|\t", val16(r.s));
  printf("X:\t0x%04X\t|\t", val16(r.x));
  printf("Y:\t0x%04X\n", val16(r.y));
  printf("P:\t0x%04X\t|\t", val16(r.p));
  printf("Z:\t0x%04X\t|\t", val16(r.z));
  printf("off:\t0x%02X\n", r.off);
  printf("IR:\t0x%02X\t|\t", r.ir);
  printf("sect:\t0x%02X\t|\t", r.sect);
  printf("op: %s\n", ops_text[((r.ir >> 4)&0xf)|(r.sect << 4)]);

  
  printf("==========");
  printf("==========");
  printf("==========");
  printf("==========");
  printf("==========");
  printf("==========\n");

}

void init_signals() {
    signal(SIGINT,sigcatch);
    signal(SIGQUIT,sigcatch);
    signal(SIGTERM,sigcatch);
}


void terminate() {
  uint64_t end_time = gettime_ms();
  uint64_t elapsed = end_time - start_time;

  double ips = 1000 * (double)cycle_counter / elapsed;

  uint64_t ns_per_instruction = elapsed * 1000000 / cycle_counter;

  ttyreset(0);


  printf("cycles: %zu\nips: %f\nns per instr: %zu\n", cycle_counter, ips, ns_per_instruction);
}

int main(int argc, char ** argv) {
  int i;
  FILE * fp = fopen(argv[1], "r");
  if(argc > 2) {
    if(!strcmp(argv[2], "-d")) {
      fl_debug = 1;
    }
  }

  init_signals();
  ttyraw(0);

  init_devices();

  memset(rom.data, 0, rom.size);

  fread(rom.data, 1, rom.size, fp);

  fclose(fp);


  r.sect = 0;
  low(r.p) = 0;
  high(r.p) = 0;

  start_time = gettime_ms();

  while(!halt) {
    if(fl_debug) {
      print_state();
      usleep(250*1000);
      start_time += 250*1000;
    }
    cycle();
  }

  terminate();

  return 0;

}
