#include "c.h"
#define I(f) b_##f

static char rcsid[] = "$Id: bytecode.c,v 1.1 2002/08/28 23:12:41 drh Exp $";

int current_sp_offset = 0;
int current_func_retsize = 0;
int last_emitted =0;

#define debug_all 0

static void I(segment)(int n)
{
	static int cseg;

	if (cseg != n)
		switch (cseg = n)
		{
		case CODE:
			print(";.text\n");
			return;
		case DATA:
			print(";.data\n");
			return;
		case BSS:
			print(";.bss\n");
			return;
		case LIT:
			print(";.lit\n");
			return;
		default:
			assert(0);
		}
}

static void I(address)(Symbol q, Symbol p, long n)
{
	q->x.name = stringf("%s%s%D", p->x.name, n > 0 ? "+" : "", n);
}

static void I(defaddress)(Symbol p)
{
	print("address %s\n", p->x.name);
}

static void I(defconst)(int suffix, int size, Value v)
{
	switch (suffix)
	{
	case I:
		if (size > sizeof(int))
			print("byte %d %D\n", size, v.i);
		else
			print("byte %d %d\n", size, v.i);
		return;
	case U:
		if (size > sizeof(unsigned))
			print("byte %d %U\n", size, v.u);
		else
			print("byte %d %u\n", size, v.u);
		return;
	case P:
		print("byte %d %U\n", size, (unsigned long)v.p);
		return;
	case F:
		if (size == 4)
		{
			float f = v.d;
			print("byte 4 %u\n", *(unsigned *)&f);
		}
		else
		{
			double d = v.d;
			unsigned *p = (unsigned *)&d;
			print("byte 4 %u\n", p[swap]);
			print("byte 4 %u\n", p[1 - swap]);
		}
		return;
	}
	assert(0);
}

static void I(defstring)(int len, char *str)
{
	char *s;

	print(".ascii \"");
	for (s = str; s < str + len; s++)
	{
		switch (*s)
		{
		case '\"':
			print("\\\"");
			break;
		case '\'':
			print("\\\'");
			break;
		case '\\':
			print("\\\\");
			break;
		case '\a':
			print("\\a");
			break;
		case '\b':
			print("\\b");
			break;
		case '\n':
			print("\\n");
			break;
		case '\t':
			print("\\t");
			break;
		case '\r':
			print("\\r");
			break;
		// and so on
		default:
			if (iscntrl(*s))
				print("\\%d", *s);
			else
				print("%c", *s);
		}
	}

	print("\"\n\n");
}

static void I(defsymbol)(Symbol p)
{
	if (p->scope == CONSTANTS)
		switch (optype(ttob(p->type)))
		{
		case I:
			p->x.name = stringf("%D", p->u.c.v.i);
			break;
		case U:
			p->x.name = stringf("%U", p->u.c.v.u);
			break;
		case P:
			p->x.name = stringf("%U", p->u.c.v.p);
			break;
		default:
			assert(0);
		}
	else if (p->scope >= LOCAL && p->sclass == STATIC)
		p->x.name = stringf("__local_static_%d", genlabel(1));
	else if (p->scope == LABELS || p->generated)
		p->x.name = stringf("__label_%s", p->name);
	else
		p->x.name = p->name;
}

void push(char *arg)
{
	print("push %s\n", arg);
	current_sp_offset++;
	print("; sp +%d\n", current_sp_offset);
}
void pushw(char *arg)
{
	print("pushw %s\n", arg);
	current_sp_offset += 2;
	print("; sp +%d\n", current_sp_offset);
}
void pop(char *arg)
{
	print("pop %s\n", arg);
	current_sp_offset--;
	print("; sp +%d\n", current_sp_offset);
}
void popw(char *arg)
{
	print("popw %s\n", arg);
	current_sp_offset -= 2;
	print("; sp +%d\n", current_sp_offset);
}

void offset_sp(int off)
{
	if(off >= 0) {
		print("; offset sp by %d\n", off);
		print("seta sl\n");
		print("lit b %d\n", off);
		print("alu add\n");
		print("puta sl\n");
		print("seta sh\n");
		print("lit b 0\n");
		print("alu adc\n");
		print("puta sh\n");
		print("; offset end\n");
	} else {
		print("; offset sp by %d\n", off);
		print("seta sl\n");
		print("lit b %d\n", -off);
		print("alu sub\n");
		print("puta sl\n");
		print("seta sh\n");
		print("lit b 0\n");
		print("alu sbc\n");
		print("puta sh\n");
		print("; offset end\n");

	}
}

/*
access locals : off(X) = current_off + n_spill + 1 + X ; m[SP-off]
access args  : off(X) = current_off + n_spill + n_locals + 2(retaddr) + size(retval??) + 1 + X ; m[SP-off]
access retval : off(X) = current_off + n_spill + n_locals + 2(retaddr) + 1
access retaddr: off(X) = current_off + n_spill + n_locals + 1
n_locals = global maxoffset
*/

#define n_spill 4

#define get_local_sp_offset(X) (current_sp_offset + n_spill + 1 + (X))
#define get_arg_sp_offset(X) (current_sp_offset + n_spill + maxoffset + 2 + current_func_retsize + 1 + (X))
#define get_retval_sp_offset() (current_sp_offset + n_spill + maxoffset + 2 + 1)
#define get_retaddr_sp_offset() (current_sp_offset + n_spill + maxoffset + 1)
#define no_reg 0xff


#define nreg16 2
#define nreg8 4
static unsigned char reg16[nreg16] = {0, 0};
static unsigned char reg8[nreg8] = {0, 0, 0, 0};
static char *reg16names[] = {
		"x", "y",
		"so" //inaccessible
};
static char *reg16memnames[] = {
		"m[x]", "m[y]",
		"m[so]"};
static char *reg8names[] = {
		"xl", "xh", "yl", "yh",
		"sol", "soh" //inaccessible
};
#define reg_so 2

static char *no_reg_name = "no_reg";

char *get8name(unsigned char reg)
{
	if (reg == no_reg)
		return no_reg_name;
	return reg8names[reg];
}

char *get16name(unsigned char reg)
{
	if (reg == no_reg)
		return no_reg_name;
	return reg16names[reg];
}

char *get16name_low(unsigned char reg)
{
	if (reg == no_reg)
		return no_reg_name;
	return reg8names[reg * 2];
}

char *get16name_high(unsigned char reg)
{
	if (reg == no_reg)
		return no_reg_name;
	return reg8names[reg * 2 + 1];
}

char *get16memreg(unsigned char reg)
{
	if (reg == no_reg)
		return no_reg_name;
	return reg16memnames[reg];
}

void dump_reg_alloc()
{
	int i;
	/*
	print("; reg16: ");
	for(i = 0; i<nreg16; i++) {
		print(" [%s]%s", reg16[i]?"x":" ", get16name(i));
	}
*/
	print("; reg8: \n;");
	for (i = 0; i < nreg8; i++)
	{
		print("\t%s", get8name(i));
	}
	print("\n;");
	for (i = 0; i < nreg8; i++)
	{
		print("\t[%s]", reg8[i] ? "x" : " ");
	}
	print("\n");
}

unsigned char alloc_reg8()
{
	unsigned char i;
	for (i = 0; i < nreg8; i++)
	{
		if (reg8[i] == 0)
		{
			reg8[i] = 1;
			reg16[i / 2] = 1;
#if debug_all
			print("; alloc reg8 %s\n", get8name(i));
			dump_reg_alloc();
#endif
			return i;
		}
	}
#if debug_all
	print("; alloc reg8 %s\n", get8name(no_reg));
	dump_reg_alloc();
#endif
	return no_reg;
}
void free_reg8(unsigned char reg)
{
#if debug_all
	print("; free reg8 %s\n", get8name(reg));
#endif
	if (reg == no_reg)
		return;
	if (reg >= nreg8)
		return; //unfreeable

	assert(reg8[reg] == 1);

	reg8[reg] = 0;

	if (reg % 2)
	{ //reg high
		if (reg8[reg - 1] == 0)
		{
			reg16[(reg - 1) / 2] = 0;
		}
	}
	else
	{ //reg low
		if (reg8[reg + 1] == 0)
		{
			reg16[reg / 2] = 0;
		}
	}
#if debug_all
	dump_reg_alloc();
#endif
}

unsigned char alloc_reg16()
{
	unsigned char i;
	for (i = 0; i < nreg16; i++)
	{
		if (reg16[i] == 0)
		{
			if ((reg8[i * 2] == 0) && (reg8[i * 2 + 1] == 0))
			{
				reg16[i] = 1;
				reg8[i * 2] = 1;
				reg8[i * 2 + 1] = 1;
#if debug_all
				print("; alloc reg16 %s\n", get16name(i));
				dump_reg_alloc();
#endif
				return i;
			}
		}
	}
#if debug_all
	print("; alloc reg16 %s\n", get16name(no_reg));
	dump_reg_alloc();
#endif
	return no_reg;
}
void spill_reg16(unsigned char reg)
{
	print("; spilling reg %s\n", get16name(reg));
	dump_reg_alloc();
	pushw(get16name(reg));
}

void unspill_reg16(unsigned char reg)
{
	print("; unspilling reg %s\n", get16name(reg));
	dump_reg_alloc();
	popw(get16name(reg));
}

void free_reg16(unsigned char reg)
{
#if debug_all
	print("; free reg16 %s\n", get16name(reg));
#endif
	if (reg == no_reg)
		return;
	if (reg >= nreg16)
		return; //unfreeable
	assert(reg16[reg] == 1);

	reg16[reg] = 0;
	reg8[reg * 2] = 0;
	reg8[reg * 2 + 1] = 0;
#if debug_all
	dump_reg_alloc();
#endif
}



#define not_implemented()                           \
	print("; not implemented (%s)\n", opname(p->op)); \
	return no_reg;
#define d_start() print("; %s %s {\n", opname(p->op), (p->syms) ? ((p->syms[0]) ? (p->syms[0]->x.name ? p->syms[0]->x.name : "") : ("")) : "");
#define d_end() print("; } %s(%s) -> %s\n;\n", \
  opname(p->op), (p->syms) ? ((p->syms[0]) ? (p->syms[0]->x.name ? p->syms[0]->x.name : "") : ("")) : "",\
  (opsize(p->op)) == 1 ? \
		get8name(target_reg) :\
		(\
			(opsize(p->op)) == 2 ?\
			  get16name(target_reg):\
				"32 bit\n"\
		)	   \
	);



int dump_tree = 0;
int ident = 0;

#define USE_SO_NO 0
#define USE_SO_LOCAL 1
#define USE_SO_ARG 2

static unsigned char dumptree(Node p)
{
	unsigned char target_reg = no_reg;
	unsigned char reg_addr, reg_val;
	unsigned char reg_arg_1, reg_arg_2;
	unsigned char size_from, size_to;
	unsigned char spilled = 0;
	unsigned char use_so = 0;
	unsigned char total_arg_size = 0;
	char need_swap = 0;
	char *cmd;
	int i;

	char is_signed = optype(p->op);
	switch (optype(p->op)) {
	case I:
		is_signed = 1;
		break;
	case P:
	case U:
		is_signed = 0;
		break;
	case F:
	case V:
	case B:
		is_signed = 0;
		break;
	default:
	   assert(0);
	}

	last_emitted = generic(p->op);

	if (dump_tree)
	{

		for (i = 0; i < ident; i++)
			print(" ");
		print("-> %s %s count: %d\n",
					opname(p->op), (p->syms) ? ((p->syms[0]) ? (p->syms[0]->x.name ? p->syms[0]->x.name : "") : ("")) : "", p->count);

		ident += 3;
		if (generic(p->op) == CALL)
		{
			for (i = 0; i < ident - 3; i++)
				print(" ");
			print(" |_ args:\n");
			for (i = 0; i < 15; i++)
			{
				assert(p->args);
				if (!p->args[i])
					break;
				dumptree(p->args[i]);
			}
			for (i = 0; i < ident - 3; i++)
				print(" ");
			print(" |_ address:\n");
		}

		if (p->kids[0])
			dumptree(p->kids[0]);
		if (p->kids[1])
			dumptree(p->kids[1]);
		ident -= 3;
		return 0;
	}

	switch (specific(p->op))
	{
	case ASGN + B:
		/*
		assert(p->kids[0]);
		assert(p->kids[1]);
		assert(p->syms[0]);
		dumptree(p->kids[0]);
		dumptree(p->kids[1]);
		print("%s %d\n", opname(p->op), p->syms[0]->u.c.v.u);
		return;
	*/
		not_implemented() case RET + V : assert(!p->kids[0]);
		assert(!p->kids[1]);
		print("ret\n");
		return no_reg;
	}

	switch (generic(p->op))
	{
	case CNST:
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0] && p->syms[0]->x.name);

		d_start();
		if ((opsize(p->op)) == 1)
		{
			target_reg = alloc_reg8();
			if (target_reg != no_reg)
			{
				print("lit %s %s\n", get8name(target_reg), p->syms[0]->x.name);
			}
			else
			{
				print("lit a %s\n", p->syms[0]->x.name);
				push("a");
			}
		}
		else if (opsize(p->op) == 2)
		{
			target_reg = alloc_reg16();
			if (target_reg != no_reg)
			{
				print("litw %s %s\n", get16name(target_reg), p->syms[0]->x.name);
			}
			else
			{
				print("litw a %s\n", p->syms[0]->x.name);
				pushw("a");
			}
		}
		else
		{
			not_implemented()
		}
		d_end();
		return target_reg;
	case ADDRG:
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0] && p->syms[0]->x.name);
		//assert(opsize(p->op) == 2);

		d_start();
		reg_addr = alloc_reg16();

		if (reg_addr == no_reg)
		{
			print("litw a $%s\n", p->syms[0]->x.name);
			pushw("a");
		}
		else
		{
			print("litw %s $%s\n", get16name(reg_addr), p->syms[0]->x.name);
		}
		d_end();
		return reg_addr;
	case ADDRF:
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0] && p->syms[0]->x.name);
		//assert(opsize(p->op) == 2);

		d_start();
		reg_addr = alloc_reg16();


		print("; >> manual arg address calculation!\n");
		print("seta sl\n");
		print("lit b %d\n", get_arg_sp_offset(atoi(p->syms[0]->x.name)));

		print("alu add\n");
		if (reg_addr == no_reg)
		{
			//endianness
			//pushing low
			push("a");
		}
		else
		{
			print("puta %s\n", get16name_low(reg_addr));
		}

		print("seta sh\n");
		print("lit b 0\n");
		print("alu adc\n");
		if (reg_addr == no_reg)
		{
			//endianness
			//pushing high
			push("a");
		}
		else
		{
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

		print("; >> manual local address calculation!\n");
		print("seta sl\n");
		print("lit b %d\n", get_local_sp_offset(atoi(p->syms[0]->x.name)));
		print("alu add\n");
		if (reg_addr == no_reg)
		{
			//endianness
			//pushing low
			push("a");
		}
		else
		{
			print("puta %s\n", get16name_low(reg_addr));
		}

		print("seta sh\n");
		print("lit b 0\n");
		print("alu adc\n");
		if (reg_addr == no_reg)
		{
			//endianness
			//pushing high
			push("a");
		}
		else
		{
			print("puta %s\n", get16name_high(reg_addr));
		}

		d_end();

		return reg_addr;
	case LABEL:
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0] && p->syms[0]->x.name);
		d_start();
		print("%s:\n", p->syms[0]->x.name);
		d_end();
		return no_reg;
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

		if ((opsize(p->op)) == 1)
		{ //to size 1
			if (p->syms[0]->u.c.v.i == 1)
			{ //from size 1
				print("; bypass conversion sz1-sz1\n");
				target_reg = reg_val;
			}
			else if (p->syms[0]->u.c.v.i == 2)
			{ //from size 2

				if (reg_val == no_reg)
				{
					popw("a");
				}
				else
				{
					print("seta %s\n", get16name_low(reg_val));
				}

				free_reg16(reg_val);
				target_reg = alloc_reg8();

				if (target_reg == no_reg)
				{
					push("a");
				}
				else
				{
					print("puta %s\n", get8name(target_reg));
				}
			}
			else
			{
				not_implemented()
			}
		}
		else if ((opsize(p->op)) == 2)
		{ //to size 2
			if (p->syms[0]->u.c.v.i == 1)
			{ //from size 1
				if (reg_val == no_reg)
				{
					pop("a");
				}
				else
				{
					print("seta %s\n", get8name(reg_val));
				}
				print("lit b 0\n");

				free_reg8(reg_val);

				target_reg = alloc_reg16();

				if (target_reg == no_reg)
				{
					pushw("a");
				}
				else
				{
					print("puta %s\n", get16name_low(target_reg));
					print("seta b\n");
					print("puta %s\n", get16name_high(target_reg));
				}
			}
			else if (p->syms[0]->u.c.v.i == 2)
			{ //from size 2
				print("; bypass conversion sz2-sz2\n");
				target_reg = reg_val;
			}
			else
			{
				not_implemented()
			}
		}
		else
		{
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
		if (reg_val == no_reg)
		{
			d_end();
			return no_reg;
		}

		if (opsize(p->op) == 1)
		{
			push(get8name(reg_val));
			free_reg8(reg_val);
		}
		else if (opsize(p->op) == 2)
		{
			pushw(get16name(reg_val));
			free_reg16(reg_val);
		}
		d_end();
		return no_reg;

	case INDIR:
		assert(p->kids[0]);
		assert(!p->kids[1]);

		if ((generic(p->kids[0]->op)) == ADDRF)
		{
			use_so = USE_SO_ARG;
			cmd = p->kids[0]->syms[0]->x.name;
			reg_addr = no_reg;
			//print("lit off %d\n", get_arg_sp_offset(atoi(p->kids[0]->syms[0]->x.name)));
		}
		else if ((generic(p->kids[0]->op)) == ADDRL)
		{
			use_so = USE_SO_LOCAL;
			cmd = p->kids[0]->syms[0]->x.name;
			reg_addr = no_reg;
			//print("lit off %d\n", get_local_sp_offset(atoi(p->kids[0]->syms[0]->x.name)));
		}
		else
		{
			use_so = USE_SO_NO;
			reg_addr = dumptree(p->kids[0]);
		}

		d_start();
		if ((reg_addr == no_reg) && (use_so == USE_SO_NO))
		{
			reg_addr = alloc_reg16();
			if (reg_addr == no_reg)
			{
				print("; failed to alloc reg16 for reg_addr in indir\n");
				assert(0);
			}
			popw(get16name(reg_addr));
		}

		if (opsize(p->op) == 1)
		{
			if (use_so == USE_SO_ARG)
			{
				print("lit off %d\n", get_arg_sp_offset(atoi(cmd)));
				print("seta m[so]\n");
			}
			else if (use_so == USE_SO_LOCAL)
			{
				print("lit off %d\n", get_local_sp_offset(atoi(cmd)));
				print("seta m[so]\n");
			}
			else
			{
				print("seta %s\n", get16memreg(reg_addr));
			}
			free_reg16(reg_addr);

			target_reg = alloc_reg8();
			if (target_reg != no_reg)
			{
				print("puta %s\n", get8name(target_reg));
			}
			else
			{
				push("a");
			}
		}
		else if (opsize(p->op) == 2)
		{

			//endianness
			//m[X] = xl
			//m[X+1] = xh
			if (use_so == USE_SO_ARG)
			{
				print("lit off %d\n", get_arg_sp_offset(atoi(cmd)));
				print("seta m[so]\n");
				print("puta b\n");
				print("lit off %d\n", get_arg_sp_offset(atoi(cmd)) + 1);
				print("seta m[so]\n");
			}
			else if (use_so == USE_SO_LOCAL)
			{
				print("lit off %d\n", get_local_sp_offset(atoi(cmd)));
				print("seta m[so]\n");
				print("puta b\n");
				print("lit off %d\n", get_local_sp_offset(atoi(cmd)) + 1);
				print("seta m[so]\n");
			}
			else
			{
				print("seta %s\n", get16memreg(reg_addr));
				print("puta b\n");
				print("%s++\n", get16name(reg_addr));
				print("seta %s\n", get16memreg(reg_addr));
			}
			//a - m[x+1]
			//b - m[x]

			free_reg16(reg_addr);

			target_reg = alloc_reg16();
			if (target_reg == no_reg)
			{
				//endianness

				push("b"); //m[x+1]
				push("a"); //m[x]
			}
			else
			{
				print("puta %s\n", get16name_high(target_reg));
				print("seta b\n");
				print("puta %s\n", get16name_low(target_reg));
			}
		}
		else
		{
			not_implemented()
		}
		d_end();
		return target_reg;
	case JUMP:
		assert(p->kids[0]);
		assert(!p->kids[1]);

		if ((generic(p->kids[0]->op)) == ADDRG)
		{ //put address as immediate literal
			d_start();
			print("jmp $%s\n", p->kids[0]->syms[0]->x.name);
		}
		else
		{
			reg_addr = dumptree(p->kids[0]);
			d_start();
			if (reg_addr != no_reg)
			{
				pushw(get16name(reg_addr));
				free_reg16(reg_addr);
			}
			print("jmps ; !!!! assume jump_from_stack!\n");
		}

		d_end();
		return no_reg;
	case RET:
		assert(p->kids[0]);
		assert(!p->kids[1]);

		reg_val = dumptree(p->kids[0]);
		d_start();

		print("; >> Access frame!\n");
		if ((opsize(p->op) == 1))
		{
			print("; store %s to frame as retval\n", get8name(reg_val));
			if (reg_val == no_reg)
			{
				pop("a");
			}
			else
			{
				print("seta %s\n", get8name(reg_val));
			}
			print("lit off %d\n", get_retval_sp_offset());
			print("puta m[so]\n");
			free_reg8(reg_val);
		}
		else if ((opsize(p->op) == 2))
		{
			print("; store %s to frame as retval\n", get16name(reg_val));
			if (reg_val == no_reg)
			{
				//endianness
				popw("a");
				print("lit off %d\n", get_retval_sp_offset());
				print("puta m[so]\n");
				print("lit off %d\n", get_retval_sp_offset() + 1);
				print("seta b\n");
				print("puta m[so]\n");
			}
			else
			{
				//endianness
				print("lit off %d\n", get_retval_sp_offset());
				print("seta %s\n", get16name_low(reg_val));
				print("puta m[so]\n");
				print("lit off %d\n", get_retval_sp_offset() + 1);
				print("seta %s\n", get16name_high(reg_val));
				print("puta m[so]\n");
			}
			free_reg16(reg_val);
		}
		else
		{
			not_implemented()
		}
		offset_sp(get_retaddr_sp_offset()-1);
		print("ret\n");
		d_end();
		return no_reg;
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

		total_arg_size = 0;

		for (i = 0; i < 15; i++)
		{
			assert(p->args);
			if (!p->args[i])
				break;
			dumptree(p->args[i]);
			total_arg_size += (opsize(p->args[i]->op));
		}

		if (generic(p->kids[0]->op) == ADDRG)
		{
			//embed address to code
			d_start();

			print("; reserve retval\n");
			for(i = 0; i<(opsize(p->op)); i++) {
				print("s--\n");
				current_sp_offset++;
			}


			print("call $%s\n", p->kids[0]->syms[0]->x.name);
		}
		else
		{
			reg_addr = dumptree(p->kids[0]);

			d_start();
			if (reg_addr != no_reg)
			{
				pushw(get16name(reg_addr));
				free_reg16(reg_addr);

				print("; reserve retval\n");
				for(i = 0; i<(opsize(p->op)); i++) {
					print("s--\n");
					current_sp_offset++;
				}

				print("calls ; !!!! assume call-from-stack\n");
			}
		}

		if ((opsize(p->op)) == 0) {
			//nothing to do
		} else if ((opsize(p->op)) == 1)
		{
			target_reg = alloc_reg8();
			if (target_reg != no_reg)
			{
				pop(get8name(target_reg));
			}
			else
			{
				print("; cleanup problem! No free reg\n");
				assert(0);
			}
		}
		else if ((opsize(p->op)) == 2)
		{
			target_reg = alloc_reg16();
			if (target_reg != no_reg)
			{
				popw(get16name(target_reg));
			}
			else
			{
				print("; cleanup problem! No free reg\n");
				assert(0);
			}
		}
		else
		{
			target_reg = no_reg;
			not_implemented()
		}

		offset_sp(total_arg_size);
		current_sp_offset -= total_arg_size;

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

		if ((generic(p->kids[0]->op)) == ADDRF)
		{
			use_so = USE_SO_ARG;
			cmd = p->kids[0]->syms[0]->x.name;
			reg_addr = no_reg;
			//print("lit off %d\n", get_arg_sp_offset(atoi(p->kids[0]->syms[0]->x.name)));
		}
		else if ((generic(p->kids[0]->op)) == ADDRL)
		{
			use_so = USE_SO_LOCAL;
			cmd = p->kids[0]->syms[0]->x.name;
			reg_addr = no_reg;
			//print("lit off %d\n", get_local_sp_offset(atoi(p->kids[0]->syms[0]->x.name)));
		}
		else
		{
			use_so = USE_SO_NO;
			reg_addr = dumptree(p->kids[0]);
		}

		d_start();
		if ((opsize(p->op)) == 1)
		{
			if ((reg_addr == no_reg) && (use_so == USE_SO_NO))
			{
				reg_addr = alloc_reg16();
				if (reg_addr == no_reg)
				{
					print("shit, it happened, unable to alloc tmp reg for addr\n");
					assert(0);
				}
				popw(get16name(reg_addr));
			}

			if (reg_val == no_reg)
			{
				pop("a");
			}
			else
			{
				print("seta %s\n", get8name(reg_val));
			}
			if (use_so == USE_SO_LOCAL)
			{
				print("lit off %d\n", get_local_sp_offset(atoi(cmd)));
				print("puta m[so]\n");
			}
			else if (use_so == USE_SO_ARG)
			{
				print("lit off %d\n", get_arg_sp_offset(atoi(cmd)));
				print("puta m[so]\n");
			}
			else
			{
				print("puta %s\n", get16memreg(reg_addr));
				free_reg16(reg_addr);
			}
			free_reg8(reg_val);
		}
		else if (opsize(p->op) == 2)
		{
			if (reg_val == no_reg)
			{
				if ((reg_addr == no_reg) && (use_so == USE_SO_NO))
				{
					reg_addr = alloc_reg16();
					if (reg_addr == no_reg)
					{
						print("shit, it happened, unable to alloc tmp reg for addr\n");
						assert(0);
					}
					popw(get16name(reg_addr));
				}

				popw("a");
				if (use_so == USE_SO_LOCAL)
				{
					//endianness

					print("lit off %d\n", get_local_sp_offset(atoi(cmd))); //m[arrd] = xl
					print("puta m[so]\n");
					print("seta b\n");
					print("lit off %d\n", get_local_sp_offset(atoi(cmd)) + 1); //m[addr+1] = xh
					print("puta m[so]\n");
				}
				else if (use_so == USE_SO_ARG)
				{
					//endianness
					print("lit off %d\n", get_arg_sp_offset(atoi(cmd)));//m[arrd] = xl
					print("puta m[so]\n");
					print("seta b\n");
					print("lit off %d\n", get_arg_sp_offset(atoi(cmd)) + 1);//m[addr+1] = xh
					print("puta m[so]\n");
				}
				else
				{
					//endianness
					//m[addr] = xl
					//m[addr+1] = xh
					print("puta %s ;check endianness\n", get16memreg(reg_addr));
					print("seta b\n");
					print("%s++\n", get16name(reg_addr));
					print("puta %s ;check endianness\n", get16memreg(reg_addr));
					free_reg16(reg_addr);
				}
				free_reg16(reg_val);
			}
			else
			{

				if ((reg_addr == no_reg) && (use_so == USE_SO_NO))
				{
					reg_addr = alloc_reg16();
					if (reg_addr == no_reg)
					{
						print("shit, it happened, unable to alloc tmp reg for addr\n");
						assert(0);
					}
					popw(get16name(reg_addr));
				}

				if (use_so == USE_SO_LOCAL)
				{
					print("lit off %d\n", get_local_sp_offset(atoi(cmd)));
					print("seta %s\n", get16name_low(reg_val));
					print("puta m[so]\n");
					print("seta %s\n", get16name_high(reg_val));
					print("lit off %d\n", get_local_sp_offset(atoi(cmd)) + 1);
					print("puta m[so]\n");
				}
				else if (use_so == USE_SO_ARG)
				{
					print("lit off %d\n", get_arg_sp_offset(atoi(cmd)));
					print("seta %s\n", get16name_low(reg_val));
					print("puta m[so]\n");
					print("seta %s\n", get16name_high(reg_val));
					print("lit off %d\n", get_arg_sp_offset(atoi(cmd)) + 1);
					print("puta m[so]\n");
				}
				else
				{
					print("seta %s\n", get16name_low(reg_val));
					print("puta %s ;check endianness\n", get16memreg(reg_addr));
					print("seta %s\n", get16name_high(reg_val));
					print("%s++\n", get16name(reg_addr));
					print("puta %s ;check endianness\n", get16memreg(reg_addr));
					free_reg16(reg_addr);
				}

				free_reg16(reg_val);
			}
		}
		else
		{
			not_implemented()
		}
		d_end();
		return no_reg;
	case RSH:not_implemented() 
	case LSH : not_implemented() 
	case BCOM : not_implemented() 
	case NEG : not_implemented() 
	case BOR : 
	case BAND : 
	case BXOR : 
	case ADD : 
	case SUB : 
		assert(p->kids[0]);
		assert(p->kids[1]);
		reg_arg_1 = dumptree(p->kids[0]);
		reg_arg_2 = dumptree(p->kids[1]);

		d_start();
		print("; generic p->op = %d\n", (generic(p->op)));
		switch (generic(p->op))
		{
		case BOR:
			cmd = "or";
			break;
		case BAND:
			cmd = "and";
			break;
		case BXOR:
			cmd = "xor";
			break;
		case ADD:
			cmd = "add";
			break;
		case SUB:
			cmd = "sub";
			break;
		default:
			assert(0);
			break;
		}

		print("; cmd: %s\n", cmd);

		if (opsize(p->op) == 1)
		{
			if (reg_arg_2 == no_reg)
			{
				pop("b");
			}
			else
			{
				print("seta %s\n", get8name(reg_arg_2));
				print("puta b\n");
			}
			if (reg_arg_1 == no_reg)
			{
				pop("a");
			}
			else
			{
				print("seta %s\n", get8name(reg_arg_1));
			}

			print("alu %s\n", cmd);

			target_reg = alloc_reg8();
			if (target_reg == no_reg)
			{
				push("a");
			}
			else
			{
				print("puta %s\n", get8name(target_reg));
			}
			free_reg8(reg_arg_1);
			free_reg8(reg_arg_2);
		}
		else if (opsize(p->op) == 2)
		{

			if (reg_arg_2 == no_reg)
			{
				reg_arg_2 = alloc_reg16();
				if (reg_arg_2 == no_reg)
				{
					print("; failed to alloc reg16 for alu operation\n");
					//assert(0);
				}
				else
				{
					popw(get16name(reg_arg_2));
				}
			}
			if (reg_arg_1 == no_reg)
			{
				reg_arg_1 = alloc_reg16();
				if (reg_arg_1 == no_reg)
				{
					print("; failed to alloc reg16 for alu operation\n");
					//assert(0);
				}
				else
				{
					popw(get16name(reg_arg_1));
				}
			}

			target_reg = alloc_reg16();

			if ((
							((reg_arg_1 == no_reg) ? 1 : 0) +
							((reg_arg_2 == no_reg) ? 1 : 0) +
							((target_reg == no_reg) ? 1 : 0)) > 1)
			{
				print("; more than one of reg16 are on stack for alu16, fail\n");
				assert(0);
			}

			if (reg_arg_2 == no_reg)
			{
				pop("b");
			}
			else
			{
				print("seta %s\n", get16name_low(reg_arg_2));
				print("puta b\n");
			}

			if (reg_arg_1 == no_reg)
			{
				pop("a");
			}
			else
			{
				print("seta %s\n", get16name_low(reg_arg_1));
			}

			print("alu %s\n", cmd);

			if (target_reg == no_reg)
			{
				//endianness
				//push low
				push("a");
			}
			else
			{
				print("puta %s\n", get16name_low(target_reg));
			}

			if (reg_arg_2 == no_reg)
			{
				pop("b");
			}
			else
			{
				print("seta %s\n", get16name_high(reg_arg_2));
				print("puta b\n");
			}

			if (reg_arg_1 == no_reg)
			{
				pop("a");
			}
			else
			{
				print("seta %s\n", get16name_high(reg_arg_1));
			}

			switch (generic(p->op))
			{
			case ADD:
				print("alu adc\n");
				break;
			case SUB:
				print("alu sbc\n");
				break;
			default:
				print("alu %s\n", cmd);
				break;
			}

			if (target_reg == no_reg)
			{
				//endianness
				//reverse low and high
				pop("b");

				//push high
				push("a");
				//push low
				push("b");
			}
			else
			{
				print("puta %s\n", get16name_high(target_reg));
			}

			free_reg16(reg_arg_1);
			free_reg16(reg_arg_2);
		}
		else
		{
			not_implemented()
		}

		d_end();
		return target_reg;

	case DIV:
		not_implemented() 
	case MUL : 
	  not_implemented() 
	case MOD : 
	  not_implemented()
/*
		assert(p->kids[0]);
		assert(p->kids[1]);
		dumptree(p->kids[0]);
		dumptree(p->kids[1]);
		print("%s\n", opname(p->op));
		return;
*/
	case EQ : 
	case NE : 
	case GT : 
	case GE : 
	case LE : 
	case LT :

		assert(p->kids[0]);
		assert(p->kids[1]);
		assert(p->syms[0]);
		assert(p->syms[0]->x.name);

		reg_arg_1 = dumptree(p->kids[0]);
		reg_arg_2 = dumptree(p->kids[1]);

		d_start();

		switch (generic(p->op))
		{
		case EQ:
			cmd = "e";
			break;
		case NE:
			cmd = "ne";
			break;
		case GT:
			cmd = "g";
			break;		
		case GE:
			cmd = "ge";
			break;
		case LE:
			cmd = "le";
			break;
		case LT:
			cmd = "l";
			break;
		default:
			assert(0);
		}

		if ((opsize(p->op)) == 1)
		{
			if (reg_arg_2 == no_reg)
			{
				pop("b");
			}
			else
			{
				print("seta %s\n", get8name(reg_arg_2));
				print("puta b\n");
				free_reg8(reg_arg_2);
			}

			if (reg_arg_1 == no_reg)
			{
				pop("a");
			}
			else
			{
				print("seta %s\n", get8name(reg_arg_1));
				free_reg8(reg_arg_1);
			}


			print("cmp sub\n");
			print("jmp %s %s $%s\n", (is_signed?"s":""), cmd, p->syms[0]->x.name);
		}
		else if ((opsize(p->op)) == 2)
		{
			if (reg_arg_2 == no_reg)
			{
				reg_arg_2 = alloc_reg16();
				if (reg_arg_2 == no_reg)
				{
					print("; can not alloc reg16 for branch instruction\n");
					assert(0);
				}
				popw(get16name(reg_arg_2));
			}

			if (reg_arg_1 == no_reg)
			{
				reg_arg_1 = alloc_reg16();
				if (reg_arg_1 == no_reg)
				{
					print("; can not alloc reg16 for branch instruction\n");
					assert(0);
				}
				popw(get16name(reg_arg_1));
			}

			// compare high bytes, if equal - jump to low byte comparison
			print("seta %s\n", get16name_high(reg_arg_2));
			print("puta b\n");
			print("seta %s\n", get16name_high(reg_arg_1));


			print("cmp sub\n");

			i = genlabel(1);

			print("jmp e $__jump_label__%d\n", i);
			if ((generic(p->op)) != EQ)
				print("jmp %s %s $%s\n", (is_signed?"s":""), cmd, p->syms[0]->x.name);
			print("jmp $__jump_label_2__%d\n", i);
			print("__jump_label__%d:\n", i);

			// compare low bytes if high equal

			print("seta %s\n", get16name_low(reg_arg_2));
			print("puta b\n");
			print("seta %s\n", get16name_low(reg_arg_1));


			print("cmp sub\n");
			print("jmp %s %s $%s\n", (is_signed?"s":""), cmd, p->syms[0]->x.name);

			print("__jump_label_2__%d:\n", i);

			free_reg16(reg_arg_1);
			free_reg16(reg_arg_2);
		}
		else
		{
			not_implemented()
		}

		d_end();

		return no_reg;

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
static void I(emit)(Node p)
{
	for (; p; p = p->link)
	{
		if (p->count)
		{
			if ((generic(p->op) == CALL) || (generic(p->op) == ARG))
			{
				continue;
			}
			else
			{
				print("deleted something important!\n");
				assert(0);
			}
		}
		dumptree(p);
		if (generic(p->op) == CALL)
		{
			//print("DISCARD%s%d\n", " "/*suffixes[optype(p->op)]*/, opsize(p->op));
		}
	}
}

static void I(export)(Symbol p)
{
	print(";.export %s\n", p->x.name);
}

static void I(function)(Symbol f, Symbol caller[], Symbol callee[], int ncalls)
{
	int i;

	(*IR->segment)(CODE);
	offset = 0;
	for (i = 0; caller[i] && callee[i]; i++)
	{
		offset = roundup(offset, caller[i]->type->align);
		caller[i]->x.name = callee[i]->x.name = stringf("%d", offset);
		caller[i]->x.offset = callee[i]->x.offset = offset;
		offset += caller[i]->type->size;
	}
	maxargoffset = maxoffset = argoffset = offset = 0;
	gencode(caller, callee);
	//wierd f->type->type->size to get return size
	current_func_retsize = f->type->type->size;
	current_sp_offset = 0;
	print("; function %s [%d] %d %d\n", f->x.name, f->type->type->size, maxoffset, maxargoffset);
	print("%s:\n", f->x.name);
	print("; alloc %d for locals, %d for spill\n", maxoffset, n_spill);
	offset_sp(-(maxoffset + n_spill));
	emitcode();
	print("; end function %s\n", f->x.name);
	if(last_emitted != RET) {
		offset_sp(get_retaddr_sp_offset()-1);
		print("ret\n");
	}
}

static void gen02(Node p)
{
	assert(p);
	if (generic(p->op) == ARG)
	{
		assert(p->syms[0]);

		/*		argoffset += (p->syms[0]->u.c.v.i < 4 ? 4 : p->syms[0]->u.c.v.i);   */
		argoffset += (p->syms[0]->u.c.v.i);
	}
	else if (generic(p->op) == CALL)
	{
		maxargoffset = (argoffset > maxargoffset ? argoffset : maxargoffset);
		argoffset = 0;
	}
}

static void gen01(Node p)
{
	if (p)
	{
		gen01(p->kids[0]);
		gen01(p->kids[1]);
		gen02(p);
	}
}
#define max_args 15
void assign_args(Node p)
{
	int i = 0;
	Node q;
	assert(p);
	p->args = (Node *)malloc(sizeof(Node) * (max_args + 1));
	memset(p->args, 0, sizeof(Node) * (max_args + 1));
	for (q = p->back_link; q; q = q->back_link)
	{
		if (generic(q->op) == CALL)
		{
			return;
		}
		if (generic(q->op) == ARG)
		{
			assert(i < max_args);
			q->count = 1;
			p->args[i] = q;
			i++;
		}
	}
}

static Node I(gen)(Node p)
{
	Node q;
	Node prev = NULL;
	assert(p);
	for (q = p; q; q = q->link)
	{
		q->back_link = prev;
		prev = q;
		gen01(q);
		if (generic(q->op) == CALL)
		{
			assign_args(q);
		}
		else
		{
			q->args = NULL;
		}
	}
	return p;
}

static void I(global)(Symbol p)
{
	//print("align %d\n", p->type->align > 4 ? 4 : p->type->align);
	print("%s:\n", p->x.name);
}

static void I(import)(Symbol p)
{
	print(";.import %s\n", p->x.name);
}

static void I(local)(Symbol p)
{
	offset = roundup(offset, p->type->align);
	p->x.name = stringf("%d", offset);
	p->x.offset = offset;
	offset += p->type->size;
}

static void I(progbeg)(int argc, char *argv[]) {}

static void I(progend)(void) {}

static void I(space)(int n)
{
	print(".skip %d\n", n);
}

static void I(stabline)(Coordinate *cp)
{
	static char *prevfile;
	static int prevline;

	if (cp->file && (prevfile == NULL || strcmp(prevfile, cp->file) != 0))
	{
		print("; file \"%s\"\n", prevfile = cp->file);
		prevline = 0;
	}
	if (cp->y != prevline)
		print("; line %d\n", prevline = cp->y);
}

#define b_blockbeg blockbeg
#define b_blockend blockend

Interface cpu4IR = {
		1, 1, 0, /* char */
		2, 1, 0, /* short */
		2, 1, 0, /* int */
		4, 1, 0, /* long */
		4, 1, 0, /* long long */
		2, 1, 1, /* float */
		2, 1, 1, /* double */
		2, 1, 1, /* long double */
		2, 1, 0, /* T* */
		0, 1, 0, /* struct */
		0,			 /* little_endian */
		0,			 /* mulops_calls */
		0,			 /* wants_callb */
		0,			 /* wants_argb */
		1,			 /* left_to_right */
		1,			 /* wants_dag */
		0,			 /* unsigned_char */
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
		0, /* I(stabblock) */
		0, /* I(stabend) */
		0, /* I(stabfend) */
		0, /* I(stabinit) */
		I(stabline),
		0, /* I(stabsym) */
		0, /* I(stabtype) */
};
