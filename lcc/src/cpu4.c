#include "c.h"
#define I(f) b_##f

static char rcsid[] = "$Id: bytecode.c,v 1.1 2002/08/28 23:12:41 drh Exp $";


int current_virtual_register = 0;

static void I(segment)(int n) {
	static int cseg;

	if (cseg != n)
		switch (cseg = n) {
		case CODE: print(";.text\n"); return;
		case DATA: print(";.data\n"); return;
		case BSS:  print(";.bss\n");  return;
		case LIT:  print(";.lit\n");  return;
		default: assert(0);
		}
}

static void I(address)(Symbol q, Symbol p, long n) {
	q->x.name = stringf("%s%s%D", p->x.name, n > 0 ? "+" : "", n);
}

static void I(defaddress)(Symbol p) {
	print("address %s\n", p->x.name);
}

static void I(defconst)(int suffix, int size, Value v) {
	switch (suffix) {
	case I:
		if (size > sizeof (int))
			print("byte %d %D\n", size, v.i);
		else
			print("byte %d %d\n", size, v.i);
		return;
	case U:
		if (size > sizeof (unsigned))
			print("byte %d %U\n", size, v.u);
		else
			print("byte %d %u\n", size, v.u);
		return;
	case P: print("byte %d %U\n", size, (unsigned long)v.p); return;
	case F:
		if (size == 4) {
			float f = v.d;
			print("byte 4 %u\n", *(unsigned *)&f);
		} else {
			double d = v.d;
			unsigned *p = (unsigned *)&d;
			print("byte 4 %u\n", p[swap]);
			print("byte 4 %u\n", p[1 - swap]);
		}
		return;
	}
	assert(0);
}

static void I(defstring)(int len, char *str) {
	char *s;

	for (s = str; s < str + len; s++)
		print("byte 1 %d\n", (*s)&0377);
}

static void I(defsymbol)(Symbol p) {
	if (p->scope == CONSTANTS)
		switch (optype(ttob(p->type))) {
		case I: p->x.name = stringf("%D", p->u.c.v.i); break;
		case U: p->x.name = stringf("%U", p->u.c.v.u); break;
		case P: p->x.name = stringf("%U", p->u.c.v.p); break;
		default: assert(0);
		}
	else if (p->scope >= LOCAL && p->sclass == STATIC)
		p->x.name = stringf("$%d", genlabel(1));
	else if (p->scope == LABELS || p->generated)
		p->x.name = stringf("$%s", p->name);
	else
		p->x.name = p->name;
}

#define not_implemented() print("; not implemented (%s)\n", opname(p->op)); return no_reg;
#define d_start() print("; %s %s {\n", opname(p->op), (p->syms)?((p->syms[0])?(p->syms[0]->x.name?p->syms[0]->x.name:""):("")):"");
#define d_end()   print("; } %s %s\n;\n", opname(p->op), (p->syms)?((p->syms[0])?(p->syms[0]->x.name?p->syms[0]->x.name:""):("")):"");



#define no_reg 0xff

#define lots_of_regs 0

#if lots_of_regs
#define nreg16 4
#define nreg8 8
static unsigned char reg16[] = {0,0,0,0,0,0,0};
static unsigned char reg8[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static char * reg16names[] = {
	"x","y","r1","r2"//,"r3","r4","r5"
};
static char * reg16memnames[] = {
	"m[x]","m[y]","m[r1]","m[r2]"//,"m[r3]","m[r4]","m[r5]"
};
static char * reg8names[] = {
	"xl","xh","yl","yh","r1l","r1h","r2l","r2h"//,"r3l","r3h","r4l","r4h","r5l","r5h"
};
#else
#define nreg16 2
#define nreg8 4
static unsigned char reg16[nreg16] = {0,0};
static unsigned char reg8[nreg8] = {0,0,0,0};
static char * reg16names[] = {
	"x","y"
};
static char * reg16memnames[] = {
	"m[x]","m[y]"
};
static char * reg8names[] = {
	"xl","xh","yl","yh"
};
#endif


static char * no_reg_name = "no_reg";

char * get8name(unsigned char reg) {
	if(reg == no_reg) return no_reg_name;
	return reg8names[reg];
}

char * get16name(unsigned char reg) {
	if(reg == no_reg) return no_reg_name;
	return reg16names[reg];
}

char * get16name_low(unsigned char reg) {
	if(reg == no_reg) return no_reg_name;
	return reg8names[reg*2];
}

char * get16name_high(unsigned char reg) {
	if(reg == no_reg) return no_reg_name;
	return reg8names[reg*2 + 1];
}

char * get16memreg(unsigned char reg) {
	if(reg == no_reg) return no_reg_name;
	return reg16memnames[reg];
}


void dump_reg_alloc() {
	int i;
/*
	print("; reg16: ");
	for(i = 0; i<nreg16; i++) {
		print(" [%s]%s", reg16[i]?"x":" ", get16name(i));
	}
*/
	print("; reg8: \n;");
	for(i = 0; i<nreg8; i++) {
		print("\t%s", get8name(i));
	}
	print("\n;");
	for(i = 0; i<nreg8; i++) {
		print("\t[%s]", reg8[i]?"x":" ");
	}
	print("\n");
}

unsigned char alloc_reg8() {
	unsigned char i;
	for(i = 0; i<nreg8; i++) {
		if(reg8[i] == 0) {
			reg8[i] = 1;
			reg16[i/2] = 1;
			print("; alloc reg8 %s\n", get8name(i));
			dump_reg_alloc();
			return i;
		}
	}
	print("; alloc reg8 %s\n", get8name(no_reg));
	dump_reg_alloc();
	return no_reg;
}
void free_reg8(unsigned char reg) {
	print("; free reg8 %s\n", get8name(reg));
	if(reg == no_reg) return;

	assert(reg8[reg] == 1);

	reg8[reg] = 0;

	if(reg % 2) { //reg high
		if(reg8[reg-1] == 0) {
			reg16[(reg-1)/2] = 0;
		}
	} else { //reg low
		if(reg8[reg+1] == 0) {
			reg16[reg/2] = 0;
		}
	}
	dump_reg_alloc();
}

unsigned char alloc_reg16() {
	unsigned char i;
	for(i = 0; i<nreg16; i++) {
		if(reg16[i] == 0) {
			if((reg8[i*2] == 0) && (reg8[i*2+1] == 0)) {
				reg16[i] = 1;
				reg8[i*2] = 1;
				reg8[i*2 + 1] = 1;
				print("; alloc reg16 %s\n", get16name(i));
				dump_reg_alloc();
				return i;
			}
		}
	}
	print("; alloc reg16 %s\n", get16name(no_reg));
	dump_reg_alloc();
	return no_reg;
}
void spill_reg16(unsigned char reg) {
	print("; spilling reg %s\n", get16name(reg));
	dump_reg_alloc();
	print("pushw %s\n", get16name(reg));
}

void unspill_reg16(unsigned char reg) {
	print("; unspilling reg %s\n", get16name(reg));
	dump_reg_alloc();
	print("popw %s\n", get16name(reg));
}

void free_reg16(unsigned char reg) {
	print("; free reg16 %s\n", get16name(reg));
	if(reg == no_reg)	return;
	assert(reg16[reg] == 1);

	reg16[reg] = 0;
	reg8[reg*2] = 0;
	reg8[reg*2+1] = 0;
	dump_reg_alloc();

}
int dump_tree = 0;
int ident = 0;
static unsigned char dumptree(Node p) {
	unsigned char target_reg;
	unsigned char reg_addr, reg_val;
	unsigned char reg_arg_1, reg_arg_2;
	unsigned char size_from, size_to;
	unsigned char spilled = 0;
	int i;

	if(dump_tree) {

		for(i = 0; i<ident; i++) print(" ");
		print("-> %s %s count: %d\n", 
			opname(p->op), (p->syms)?((p->syms[0])?(p->syms[0]->x.name?p->syms[0]->x.name:""):("")):"", p->count);

		ident += 3;
		if(generic(p->op) == CALL) {
			for(i = 0; i<ident-3; i++) print(" ");
			print(" |_ args:\n");
			for(i = 0; i<15; i++) {
				assert(p->args);
				if(!p->args[i]) break;
				dumptree(p->args[i]);
			}
			for(i = 0; i<ident-3; i++) print(" ");
			print(" |_ address:\n");
		}

		if(p->kids[0]) dumptree(p->kids[0]);
		if(p->kids[1]) dumptree(p->kids[1]);
		ident -= 3;
		return 0;
	}


	switch (specific(p->op)) {
	case ASGN+B:
	/*
		assert(p->kids[0]);
		assert(p->kids[1]);
		assert(p->syms[0]);
		dumptree(p->kids[0]);
		dumptree(p->kids[1]);
		print("%s %d\n", opname(p->op), p->syms[0]->u.c.v.u);
		return;
	*/
		not_implemented()
	case RET+V:
	/*
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		print("%s\n", opname(p->op));
		return;
	*/
		not_implemented()
	}

	switch (generic(p->op)) {
	case CNST: 
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0] && p->syms[0]->x.name);

		d_start();
		if((opsize(p->op)) == 1) {
			target_reg = alloc_reg8();
			if(target_reg != no_reg) {
				print("lit %s %s\n", get8name(target_reg), p->syms[0]->x.name);
			} else {
				print("lit a %s\n", p->syms[0]->x.name);
				print("push a\n");
			}
		} else if(opsize(p->op) == 2) {
			target_reg = alloc_reg16();
			if(target_reg != no_reg) {
				print("litw %s %s\n", get16name(target_reg), p->syms[0]->x.name);
			} else {
				print("litw a %s\n", p->syms[0]->x.name);
				print("pushw a\n");
			}
		} else {
			not_implemented()
		}
		d_end();
		return target_reg;
	case ADDRG: 
		not_implemented()
	case ADDRF: 
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0] && p->syms[0]->x.name);
		//assert(opsize(p->op) == 2);
		
		d_start();
		reg_addr = alloc_reg16();

		print("seta zl\n");
		print("lit b %s\n", p->syms[0]->x.name);
		print("alu sub\n");
		if(reg_addr == no_reg) {
			print("push a\n");
		} else {
			print("puta %s\n", get16name_low(reg_addr));
		}

		print("seta zh\n");
		print("lit b 0\n");
		print("alu sbc\n");
		if(reg_addr == no_reg) {
			print("push a\n");
		} else {
			print("puta %s\n", get16name_high(reg_addr));
		}

		d_end();

		return reg_addr;
	case ADDRL: 
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0] && p->syms[0]->x.name);
		//assert(opsize(p->op) == 2);
		
		d_start();
		
		reg_addr = alloc_reg16();

		print("seta zl\n");
		print("lit b %s\n", p->syms[0]->x.name);
		print("alu add\n");
		if(reg_addr == no_reg) {
			print("push a\n");
		} else {
			print("puta %s\n", get16name_low(reg_addr));
		}

		print("seta zh\n");
		print("lit b 0\n");
		print("alu adc\n");
		if(reg_addr == no_reg) {
			print("push a\n");
		} else {
			print("puta %s\n", get16name_high(reg_addr));
		}

		d_end();

		return reg_addr;
	case LABEL:
		not_implemented()
/*
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0] && p->syms[0]->x.name);
		print("%s %s\n", opname(p->op), p->syms[0]->x.name);
		return;
*/
	case CVF: 
	case CVI: 
	case CVP: 
	case CVU:
		assert(p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0]);


		reg_val = dumptree(p->kids[0]);

		d_start();

		if((opsize(p->op)) == 1) { //to size 1
			if(p->syms[0]->u.c.v.i == 1) { //from size 1
				print("; bypass conversion sz1-sz1\n");
				target_reg = reg_val;

			} else if(p->syms[0]->u.c.v.i == 2) { //from size 2

				if(reg_val == no_reg) {
					print("popw a\n");
				} else {
					print("seta %s\n", get16name_low(reg_val));
				}

				free_reg16(reg_val);
				target_reg = alloc_reg8();

				if(target_reg == no_reg) {
					print("push a\n");
				} else {
					print("puta %s\n", get8name(target_reg));
				}

			}
		} else if((opsize(p->op)) == 2) { //to size 2
			if(p->syms[0]->u.c.v.i == 1) { //from size 1
				if(reg_val == no_reg) {
					print("pop a\n");
				} else {
					print("seta %s\n", get8name(reg_val));
				}
				print("lit b 0\n");

				free_reg8(reg_val);

				target_reg = alloc_reg16();

				if(target_reg == no_reg) {
					print("pushw a\n");
				} else {
					print("puta %s\n", get16name_low(target_reg));
					print("seta b\n");
					print("puta %s\n", get16name_high(target_reg));
				}


			} else if(p->syms[0]->u.c.v.i == 2) { //from size 2
				print("; bypass conversion sz2-sz2\n");
				target_reg = reg_val;

			}
		} else {
			not_implemented();
		}
		d_end();
		return target_reg;
/*
		assert(p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0]);
		dumptree(p->kids[0]);
		print("%s %d\n", opname(p->op), p->syms[0]->u.c.v.i);
		return;
*/
	case ARG: 
		assert(p->kids[0]);
		assert(!p->kids[1]);

		reg_val = dumptree(p->kids[0]);
		d_start();
		if(reg_val == no_reg) {
			d_end();
			return no_reg;
		}

		if(opsize(p->op) == 1) {
			print("push %s\n", get8name(reg_val));
			free_reg8(reg_val);
		} else if(opsize(p->op) == 2) {
			print("pushw %s\n", get16name(reg_val));
			free_reg16(reg_val);
		}
		d_end();
		return no_reg;
	case BCOM: 
		not_implemented()
	case NEG: 
		not_implemented()
	case INDIR: 
		assert(p->kids[0]);
		assert(!p->kids[1]);
		reg_addr = dumptree(p->kids[0]);

		d_start();
		if(reg_addr == no_reg) {
			reg_addr = alloc_reg16();
			if(reg_addr == no_reg) {
				print("; failed to alloc reg16\n");
				assert(0);
			}
			print("popw %s\n", get16name(reg_addr));
		}

		if(opsize(p->op) == 1) {
			print("seta %s\n", get16memreg(reg_addr));
			free_reg16(reg_addr);

			target_reg = alloc_reg8();
			if(target_reg != no_reg) {
				print("puta %s\n", get8name(target_reg));
			} else {
				print("push a\n");
			}

		} else if(opsize(p->op) == 2) {
			print("seta %s\n", get16memreg(reg_addr));
			print("puta b\n");
			print("%s++\n", get16name(reg_addr));
			print("seta %s\n", get16memreg(reg_addr));

			//a - second
			//b - first

			free_reg16(reg_addr);

			target_reg = alloc_reg16();
			if(target_reg == no_reg) {
				print("push b\n");
				print("push a\n");
			} else {
				print("puta %s\n", get16name_high(target_reg));
				print("seta b\n");
				print("puta %s\n", get16name_low(target_reg));
			}


		} else {
			not_implemented()
		}
		d_end();
		return target_reg;
	case JUMP: 
		not_implemented()
	case RET:
		not_implemented()
/*
		assert(p->kids[0]);
		assert(!p->kids[1]);
		dumptree(p->kids[0]);
		print("%s\n", opname(p->op));
		return;
		*/
	case CALL:
		assert(p->kids[0]);
		assert(!p->kids[1]);
		assert(optype(p->op) != B);

		for(i = 0; i<15; i++) {
			assert(p->args);
			if(!p->args[i]) break;
			dumptree(p->args[i]);
		}

		if(generic(p->kids[0]->op) == ADDRG) {
			//embed address to code
			d_start();
			print("call $%s\n", p->kids[0]->syms[0]->x.name);
		} else {
			reg_addr = dumptree(p->kids[0]);

			d_start();
			if(reg_addr != no_reg) {
				print("pushw %s\n", get16name(reg_addr));
				free_reg16(reg_addr);
				print("call_st ; !!!! assume call-from-stack\n");
			}
		}

		if((opsize(p->op)) == 1) {
			target_reg = alloc_reg8();
			if(target_reg != no_reg) {
				print("pop %s\n", get8name(target_reg));
			} else {
				print("; cleanup problem! No free reg\n");
				assert(0);
			}
		} else if((opsize(p->op)) == 2){
			target_reg = alloc_reg16();
			if(target_reg != no_reg) {
				print("popw %s\n", get16name(target_reg));
			} else {
				print("; cleanup problem! No free reg\n");
				assert(0);
			}
		} else {
			not_implemented()
		}

		print("!!! cleanup after call !!!\n");
		d_end();
		return target_reg;
/*
		assert(p->kids[0]);
		assert(!p->kids[1]);
		assert(optype(p->op) != B);
		dumptree(p->kids[0]);
		print("%s\n", opname(p->op));
		return;
*/
	case ASGN: 
		assert(p->kids[0]);
		assert(p->kids[1]);
		reg_val = dumptree(p->kids[1]);
		reg_addr = dumptree(p->kids[0]);

		d_start();
		if((opsize(p->op)) == 1) {
			if(reg_val == no_reg) {
				print("pop a\n");
			} else {
				print("seta %s\n", get8name(reg_val));
			}

			if(reg_addr == no_reg) {
				reg_addr = alloc_reg16();
				if(reg_addr == no_reg) {
					print("shit, it happened, unable to alloc tmp reg for addr\n");
					assert(0);
				}
				print("popw %s\n", get16name(reg_addr));
			}

			print("puta %s\n", get16memreg(reg_addr));
			free_reg8(reg_val);
			free_reg16(reg_addr);
		} else if(opsize(p->op) == 2) {
			if(reg_val == no_reg) {
				print("popw a\n");

				if(reg_addr == no_reg) {
					reg_addr = alloc_reg16();
					if(reg_addr == no_reg) {
						print("shit, it happened, unable to alloc tmp reg for addr\n");
						assert(0);
					}
					print("popw %s\n", get16name(reg_addr));
				}

				print("puta %s ;check endianness\n", get16memreg(reg_addr));
				print("seta b\n");
				print("%s++\n", get16name(reg_addr));
				print("puta %s ;check endianness\n", get16memreg(reg_addr));
				free_reg16(reg_addr);
				free_reg16(reg_val);
			} else {

				if(reg_addr == no_reg) {
					reg_addr = alloc_reg16();
					if(reg_addr == no_reg) {
						print("shit, it happened, unable to alloc tmp reg for addr\n");
						assert(0);
					}
					print("popw %s\n", get16name(reg_addr));
				}

				print("seta %s\n", get16name_low(reg_val));
				print("puta %s ;check endianness\n", get16memreg(reg_addr));
				print("seta %s\n", get16name_high(reg_val));
				print("%s++\n", get16name(reg_addr));
				print("puta %s ;check endianness\n", get16memreg(reg_addr));

				free_reg16(reg_addr);
				free_reg16(reg_val);

			}
		} else {
			not_implemented()
		}
		d_end();
		return no_reg;
	case RSH: 
		not_implemented()
	case LSH:
		not_implemented()
	case BOR: 
	case BAND: 
	case BXOR: 
	case ADD: 
	case SUB: 
		assert(p->kids[0]);
		assert(p->kids[1]);
		reg_arg_1 = dumptree(p->kids[0]);
		reg_arg_2 = dumptree(p->kids[1]);
		
		d_start();
		if(opsize(p->op) == 1) {
			if(reg_arg_2 == no_reg) {
				print("pop b\n");
			} else {
				print("seta %s\n", get8name(reg_arg_2));
				print("puta b\n");
			}
			if(reg_arg_1 == no_reg) {
				print("pop a\n");
			} else {
				print("seta %s\n", get8name(reg_arg_1));
			}

			switch(specific(p->op)) {
				case BOR:
					print("alu or\n");
					break;
				case BAND: 
					print("alu and\n");
					break;
				case BXOR: 
					print("alu xor\n");
					break;
				case ADD: 
					print("alu add\n");
					break;
				case SUB: 
					print("alu sub\n");
					break;
				default:
					break;				
			}

			target_reg = alloc_reg8();
			if(target_reg == no_reg) {
				print("push a\n");
			} else {
				print("puta %s\n", get8name(target_reg));
			}
			free_reg8(reg_arg_1);
			free_reg8(reg_arg_2);
		} else if(opsize(p->op) == 2) {
			if(reg_arg_1 == no_reg && reg_arg_2 == no_reg) {
				print("; both sources for alu16 are on stack, fail\n");
				assert(0);
			}


			if(reg_arg_2 == no_reg) {
				reg_arg_2 = alloc_reg16();
				if(reg_arg_2 == no_reg) {
					print("; failed to alloc reg16 for alu operation\n");
					//assert(0);
				}
				print("popw %s\n", get16name(reg_arg_2));
			}
			if(reg_arg_1 == no_reg) {
				reg_arg_1 = alloc_reg16();
				if(reg_arg_1 == no_reg) {
					print("; failed to alloc reg16 for alu operation\n");
					//assert(0);
				}
				print("popw %s\n", get16name(reg_arg_1));
			}


			target_reg = alloc_reg16();

			if(reg_arg_2 == no_reg) {
				print("pop b ; assume popped low!!\n");
			} else {
				print("seta %s\n", get16name_low(reg_arg_2));
				print("puta b\n");
			}

			if(reg_arg_1 == no_reg) {
				print("pop a ; assume popped low!!\n");
			} else {
				print("seta %s\n", get16name_low(reg_arg_1));
				print("puta b\n");
			}

			print("alu add ;!!!!!\n");
			
			if(target_reg == no_reg) {
				print("push a\n");
			} else {
				print("puta %s\n", get16name_low(target_reg));
			}

			if(reg_arg_2 == no_reg) {
				print("pop b ; assume popped high!!\n");
			} else {
				print("seta %s\n", get16name_high(reg_arg_2));
				print("puta b\n");
			}

			if(reg_arg_1 == no_reg) {
				print("pop a ; assume popped high!!\n");
			} else {
				print("seta %s\n", get16name_high(reg_arg_1));
			}

			print("alu adc ;!!!!!\n");
			
			if(target_reg == no_reg) {
				print("push a\n");
			} else {
				print("puta %s\n", get16name_high(target_reg));
			}

			free_reg16(reg_arg_1);
			free_reg16(reg_arg_2);
		} else {
			not_implemented()
		}

		d_end();
		return target_reg;


	case DIV: 
		not_implemented()
	case MUL: 
		not_implemented()
	case MOD:
		not_implemented()
/*
		assert(p->kids[0]);
		assert(p->kids[1]);
		dumptree(p->kids[0]);
		dumptree(p->kids[1]);
		print("%s\n", opname(p->op));
		return;
*/
	case EQ: 
		not_implemented()
	case NE: 
		not_implemented()
	case GT: 
		not_implemented()
	case GE: 
		not_implemented()
	case LE: 
		not_implemented()
	case LT:
		not_implemented()
/*
		assert(p->kids[0]);
		assert(p->kids[1]);
		assert(p->syms[0]);
		assert(p->syms[0]->x.name);
		dumptree(p->kids[0]);
		dumptree(p->kids[1]);
		print("%s %s\n", opname(p->op), p->syms[0]->x.name);
		return;
*/
	}
	assert(0);
}
/*
static void I(asmcode)(char * s , Symbol ss[] ) {
  
  write(2, "EMIT ASM", strlen("EMIT ASM"));
  
  print("asm\n");
}
*/
static void I(emit)(Node p) {
	for (; p; p = p->link) {
		if(p->count) {
			if ((generic(p->op) == CALL) || (generic(p->op) == ARG)) {
				continue;
			} else {
				print("deleted something important!\n");
				assert(0);
			}
		}
		dumptree(p);
		if (generic(p->op) == CALL) {
			//print("DISCARD%s%d\n", " "/*suffixes[optype(p->op)]*/, opsize(p->op));
		}
	}
}

static void I(export)(Symbol p) {
	print("export %s\n", p->x.name);
}

static void I(function)(Symbol f, Symbol caller[], Symbol callee[], int ncalls) {
	int i;

	(*IR->segment)(CODE);
	offset = 0;
	for (i = 0; caller[i] && callee[i]; i++) {
		offset = roundup(offset, caller[i]->type->align);
		caller[i]->x.name = callee[i]->x.name = stringf("%d", offset);
		caller[i]->x.offset = callee[i]->x.offset = offset;
		offset += caller[i]->type->size;
	}
	maxargoffset = maxoffset = argoffset = offset = 0;
	gencode(caller, callee);
	print("proc %s %d %d\n", f->x.name, maxoffset, maxargoffset);
	emitcode();
	print("endproc %s %d %d\n", f->x.name, maxoffset, maxargoffset);

}

static void gen02(Node p) {
	assert(p);
	if (generic(p->op) == ARG) {
		assert(p->syms[0]);

/*		argoffset += (p->syms[0]->u.c.v.i < 4 ? 4 : p->syms[0]->u.c.v.i);   */
		argoffset += (p->syms[0]->u.c.v.i);

	} else if (generic(p->op) == CALL) {
		maxargoffset = (argoffset > maxargoffset ? argoffset : maxargoffset);
		argoffset = 0;
	}
}

static void gen01(Node p) {
	if (p) {
		gen01(p->kids[0]);
		gen01(p->kids[1]);
		gen02(p);
	}
}
#define max_args 15
void assign_args(Node p) {
	int i=0;
	Node q;
	assert(p);
	p->args = (Node *)malloc(sizeof(Node) * (max_args+1));
	memset(p->args, 0, sizeof(Node)*(max_args+1));
	for (q = p->back_link; q; q = q->back_link) {
		if(generic(q->op) == CALL) {
			return;
		}
		if(generic(q->op) == ARG) {
			assert(i<max_args); 
			q->count = 1;
			p->args[i] = q;
			i++;
		}
	}
}

static Node I(gen)(Node p) {
	Node q;
	Node prev = NULL;
	assert(p);
	for (q = p; q; q = q->link) {
		q->back_link = prev;
		prev = q;
		gen01(q);
		if(generic(q->op) == CALL) {
			assign_args(q);
		} else {
			q->args = NULL;
		}
  }
	return p;
}

static void I(global)(Symbol p) {
	print("align %d\n", p->type->align > 4 ? 4 : p->type->align);
	print("LABELV %s\n", p->x.name);
}

static void I(import)(Symbol p) {
	print("import %s\n", p->x.name);
}

static void I(local)(Symbol p) {
	offset = roundup(offset, p->type->align);
	p->x.name = stringf("%d", offset);
	p->x.offset = offset;
	offset += p->type->size;
}

static void I(progbeg)(int argc, char *argv[]) {}

static void I(progend)(void) {}

static void I(space)(int n) {
	print(".skip %d\n", n);
}

static void I(stabline)(Coordinate *cp) {
	static char *prevfile;
	static int prevline;

	if (cp->file && (prevfile == NULL || strcmp(prevfile, cp->file) != 0)) {
		print("; file \"%s\"\n", prevfile = cp->file);
		prevline = 0;
	}
	if (cp->y != prevline)
		print("; line %d\n", prevline = cp->y);
}

#define b_blockbeg blockbeg
#define b_blockend blockend

Interface cpu4IR = {
	1, 1, 0,	/* char */
	2, 1, 0,	/* short */
	2, 1, 0,	/* int */
	2, 1, 0,	/* long */
	2, 1, 0,	/* long long */
	2, 1, 1,	/* float */
	2, 1, 1,	/* double */
	2, 1, 1,	/* long double */
	2, 1, 0,	/* T* */
	0, 1, 0,	/* struct */
	0,		/* little_endian */
	0,		/* mulops_calls */
	0,		/* wants_callb */
	0,		/* wants_argb */
	1,		/* left_to_right */
	1,		/* wants_dag */
	0,		/* unsigned_char */
	I(address),
	I(blockbeg),
	I(blockend),
	0, //I(asmcode),
	I(defaddress),
	I(defconst),
	I(defstring),
	I(defsymbol),
	I(emit),
	I(export),
	I(function),
	I(gen),
	I(global),
	I(import),
	I(local),
	I(progbeg),
	I(progend),
	I(segment),
	I(space),
	0,		/* I(stabblock) */
	0,		/* I(stabend) */
	0,		/* I(stabfend) */
	0,		/* I(stabinit) */
	I(stabline),
	0,		/* I(stabsym) */
	0,		/* I(stabtype) */
};
