#include "c.h"
#include "cpu4_impl.h"
#define I(f) b_##f

static char rcsid[] = "$Id: bytecode.c,v 1.1 2002/08/28 23:12:41 drh Exp $";

int current_sp_offset = 0;
int current_func_retsize = 0;
int last_emitted =0;

#define debug_all 1

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
	print(".word $%s\n", p->x.name);
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

int n_spill = 0;

#define no_reg 0xff

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




#define spill_mask_size 32
int spill_mask[spill_mask_size]; //32 2-byte spill

void clear_spill_mask() {
	memset(spill_mask, 0, spill_mask_size * sizeof(int));
}

int alloc_spill() {
	int i = 0;
	for(i = 0; i<spill_mask_size; i++) {
		if(spill_mask[i] == 0) { 
			spill_mask[i] = 1;
			return i;
		}
	}
	fprintf(stderr, "spill overflow!\n");
	assert(0);
}

void free_spill(int i) {
	assert(i<spill_mask_size);
	spill_mask[i] = 0;

}


#define BIT_ISREG    0x2000
#define BIT_ISSPILL  0x4000
#define BIT_ISTARGET 0x8000
#define is_target_reg(x) 		( ((x) & BIT_ISTARGET) && ((x) & BIT_ISREG) )
#define is_target_spill(x) 	( ((x) & BIT_ISTARGET) && ((x) & BIT_ISSPILL) )
#define is_target(x) 		 	(  (x) & BIT_ISTARGET  )   
#define get_target(x)			(  (x) & 0xff          )


unsigned int create_reg_target(int x) {
	return BIT_ISTARGET | BIT_ISREG | (unsigned int)x;
}

unsigned int create_spill_target(int x) {
	return BIT_ISTARGET | BIT_ISSPILL | (unsigned int)x;

}

unsigned int create_no_target() {
	return 0;
}

struct op_processors {
	int generic_op;
	void (*fn[4])(Node p);
};



struct op_processors procs[] = {
//name	sz:	 	0			 		1			 		2					4
	{CNST,				{nop,			cnst,			cnst,			cnst}},
	{ADDRG, 				{nop,			nop,			addrg,		nop}},
	{ADDRF,				{nop,			nop,			addrf,		nop}},
	{ADDRL,				{nop,			nop,			addrl,		nop}},
	{LABEL,				{label,		nop,			nop,			nop}},
	{CVF,					{nop,			cv,			cv,			nop}},
	{CVI,					{nop,			cv,			cv,			nop}},
	{CVP,					{nop,			cv,			cv,			nop}},
	{CVU,					{nop,			cv,			cv,			nop}},
	{ARG, 				{nop,			nop,			nop,			nop}},
	{BCOM, 				{nop,			nop,			nop,			nop}},
	{NEG, 				{nop,			nop,			nop,			nop}},
	{INDIR, 				{nop,			nop,			nop,			nop}},
	{JUMP, 				{nop,			nop,			nop,			nop}},
	{RET,					{nop,			nop,			nop,			nop}},
	{CALL,				{nop,			nop,			nop,			nop}},
	{ASGN, 				{nop,			nop,			nop,			nop}},
	{BOR, 				{nop,			alu_1,		alu_2,		nop}},
	{BAND, 				{nop,			alu_1,		alu_2,		nop}},
	{BXOR, 				{nop,			alu_1,		alu_2,		nop}},
	{RSH, 				{nop,			nop,			nop,			nop}},
	{LSH,					{nop,			nop,			nop,			nop}},
	{ADD, 				{nop,			alu_1,		alu_2,		nop}},
	{SUB, 				{nop,			alu_1,		alu_2,		nop}},
	{DIV, 				{nop,			nop,			nop,			nop}},
	{MUL, 				{nop,			nop,			nop,			nop}},
	{MOD,					{nop,			nop,			nop,			nop}},
	{EQ, 					{nop,			nop,			nop,			nop}},
	{NE, 					{nop,			nop,			nop,			nop}},
	{GT, 					{nop,			nop,			nop,			nop}},
	{GE, 					{nop,			nop,			nop,			nop}},
	{LE, 					{nop,			nop,			nop,			nop}},
	{LT,				  	{nop,			nop,			nop,			nop}},

	0, {0,0,0,0}
};



static void dump_op(int generic_op, Node p) {
	int i = 0;
	int sz;
	switch(opsize(p->op)) {
		case 0:
			sz = 0;
		break;
		case 1:
			sz = 1;
		break;
		case 2:
			sz = 2;
		break;
		case 4:
			sz = 3;
		break;
	}
	while(1) {
		if(procs[i].generic_op == 0) {
			print("WTF %s\n", opname(generic_op));
			assert(0);
		}
		if(procs[i].generic_op == generic_op) {
			procs[i].fn[sz](p);
			return;
		}
		i++;
	}
}


int ident = 0;
int print_tree = 0;


static void dumptree_dbg(Node p)
{
	int i;
	char * target_type;

	if(!is_target(p->target)) {
		target_type = "NO TARGET";
	} else if(is_target_reg(p->target)) {
		target_type = "REG";
	} else if(is_target_spill(p->target)) {
		target_type = "SPILL";
	}

	for (i = 0; i < ident; i++) print(" ");

	print("-> %s%s %s count: %d depth: %d target: %s %d\n\n",
				p->emitted?"[EMITTED PREVIOUSLY] ":"",
				opname(p->op), (p->syms) ? ((p->syms[0]) ? (p->syms[0]->x.name ? p->syms[0]->x.name : "") : ("")) : "", 
				p->count,
				p->depth,
				target_type,
				get_target(p->target));

	p->emitted = 1;


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
			dumptree_dbg(p->args[i]);
		}
		for (i = 0; i < ident - 3; i++)
			print(" ");
		print(" |_ address:\n");
	}

	if (p->kids[0])
		dumptree_dbg(p->kids[0]);
	if (p->kids[1])
		dumptree_dbg(p->kids[1]);
	ident -= 3;
	return;

}
static void dumptree(Node p)
{
	int i;
	char * target_type;



	if(p->emitted) return;


	ident += 3;


	
	if (p->kids[0])
		dumptree(p->kids[0]);
	if (p->kids[1])
		dumptree(p->kids[1]);

/////
	if(!is_target(p->target)) {
		target_type = "NO TARGET";
	} else if(is_target_reg(p->target)) {
		target_type = "REG";
	} else if(is_target_spill(p->target)) {
		target_type = "SPILL";
	}

	for (i = 0; i < ident; i+=3) print(" - ");

	print("-> %s%s %s count: %d depth: %d target: %s %d\n\n",
				p->emitted?"[EMITTED PREVIOUSLY] ":"",
				opname(p->op), (p->syms) ? ((p->syms[0]) ? (p->syms[0]->x.name ? p->syms[0]->x.name : "") : ("")) : "", 
				p->count,
				p->depth,
				target_type,
				get_target(p->target));



/////

	print("opname : %s\n", opname(p->op));
	dump_op(generic(p->op), p);
	p->emitted = 1;
	ident -= 3;
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
				fprintf(stderr, "Deleted something from tree: %s\n",opname(p->op));
				continue;
			}
		}
		dumptree(p);
		if (generic(p->op) == CALL)
		{
			//print("DISCARD%s%d\n", " "/*suffixes[optype(p->op)]*/, opsize(p->op));
		}
	}
}

static void I(emit_tree)(Node p)
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
				fprintf(stderr, "Deleted something from tree: %s\n",opname(p->op));
				continue;
			}
		}
		dumptree_dbg(p);
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
	clear_spill_mask();
	n_spill = 0;
	gencode(caller, callee);
	//wierd f->type->type->size to get return size
	current_func_retsize = f->type->type->size;
	current_sp_offset = 0;
	print("; function %s [%d] %d %d\n", f->x.name, f->type->type->size, maxoffset, maxargoffset);
	print("%s:\n", f->x.name);
	print("; alloc %d for locals\n", maxoffset);
	print("; n_spill = %d\n", n_spill);
	//offset_sp(-(maxoffset + n_spill));
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

int mandatory_spill(Node p) {
	switch(generic(p->op)) {
		case BOR:
		case BAND:
		case BXOR:
		case ADD:
		case SUB:
			return 1;
		default:
			return 0;
	}
}


static void gen_node(Node p, int * depth, unsigned int target)
{
	int i;
	int d = 0;
	int n_kids = 0;

	int spill_for_kid = 0;
	unsigned int target_kid_0 = 0;
	unsigned int target_kid_1 = 0;

	int kid_0_result_preserved = 0;
	int kid_1_result_preserved = 0;

	//calculate_depth
	if (p)
	{
		for(i = 0; i<ident; i++) print(" ");
		print("gen %s count %d\n", opname(p->op), p->count);
		ident+=3;
		n_kids = ((p->kids[0])?1:0) + ((p->kids[1])?1:0);

		if(p->kids[0]) {
			if(!p->kids[0]->generated) {
				if(p->kids[0]->count > 1) {
					kid_0_result_preserved = 1;
				}
				//put result to spill in second kid present, or if intermediate to be used later
				if(n_kids > 1 || kid_0_result_preserved) {
					target_kid_0 = create_spill_target(alloc_spill());
				} else {
					target_kid_0 = create_reg_target(0);
				}
				gen_node(p->kids[0], &d, target_kid_0);
			}
		}

		if(p->kids[1]) {
			if(!p->kids[1]->generated) {
				if(p->kids[1]->count > 1) {
					kid_1_result_preserved = 1;
				}
				//put result to spill if intermediate to be used later
				if(kid_1_result_preserved) {
					target_kid_1 = create_spill_target(alloc_spill());
				} else {
					target_kid_1 = create_reg_target(1);
				}
				gen_node(p->kids[1], &d, target_kid_1);
			}
		}

		ident-=3;
		if(is_target_spill(target_kid_0)) {
			if(!kid_0_result_preserved) {
				free_spill(get_target(target_kid_0));
			}
		}

		if(is_target_spill(target_kid_1)) {
			if(!kid_1_result_preserved) {
				free_spill(get_target(target_kid_1));
			}
		}

		d++;


		if(is_target_reg(target)) {
			if(p->count > 1) {
				target = create_spill_target(alloc_spill());
			}
		}

		if(!is_target(target)) {
			if(generic(p->op) == CALL) {
				target = create_spill_target(alloc_spill());
			}
		}

		p->target = target;

		if(is_target_spill(target_kid_0)) {
			if(n_spill < get_target(target_kid_0) + 1) n_spill = get_target(target_kid_0) + 1;
		}
		if(is_target_spill(target_kid_1)) {
			if(n_spill < get_target(target_kid_1) + 1) n_spill = get_target(target_kid_1) + 1;
		}
		if(is_target_spill(target)) {
			if(n_spill < get_target(target) + 1) n_spill = get_target(target) + 1;
		}

		p->generated = 1;
		p->depth = d;
		*depth += d;
	}
}

static void gen_root(Node p) {
	int depth = 0;
	gen_node(p, &depth,create_no_target());
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
		gen_root(q);
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

Interface cpu4_treeIR = {
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
		I(emit_tree),
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
