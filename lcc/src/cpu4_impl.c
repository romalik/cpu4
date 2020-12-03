#include "c.h"
#include "cpu4_impl.h"


static char * reg8_names[] = {
	"xl",
	"yl"
};

static char * reg16_names[] = {
	"x",
	"y"
};

char __tmp_reg_name[5];
char * get_target_reg_name(Node p, int high) {
	if(is_target_reg(p->target)) {
		sprintf(__tmp_reg_name, "%s%s", reg16_names[get_target(p->target)], high?"h":"l");
		return __tmp_reg_name;
	} else {
		return NULL;
	}
}

void put_kid_to_reg8(Node p, int kid, char * reg, int high) {
	char * tmp_reg = "a";
	int need_move = 1;
	if(reg[0] == 'a') {
		tmp_reg = "a";
		need_move = 0;
	} else if(reg[0] == 'b') {
		tmp_reg = "b";
		need_move = 0;
	}

	if(is_target_reg(p->kids[kid]->target)) {
		print("set%s %s%s\n", tmp_reg, reg16_names[get_target(p->kids[kid]->target)], high?"h":"l");
	} else if(is_target_spill(p->kids[kid]->target)) {
		print("get_rel_sp %s %d\n", tmp_reg, get_spill_sp_offset(get_target(p->kids[kid]->target)*2 + high));
	} else {
		fprintf(stderr, "target: 0x%04x\n", p->kids[kid]->target);
		assert(0);
	}
	if(need_move) {
		print("put%s %s\n", tmp_reg, reg);
	}
}

void put_reg8_to_target(Node p, char * reg, int high) {
	char * tmp_reg = "a";
	int need_move = 1;
	if(reg[0] == 'a') {
		tmp_reg = "a";
		need_move = 0;
	} else if(reg[0] == 'b') {
		tmp_reg = "b";
		need_move = 0;
	}

	if(need_move) {
		print("set%s %s\n", tmp_reg, reg);
	}
	if(is_target_reg(p->target)) {
		print("put%s %s%s\n", tmp_reg, reg16_names[get_target(p->target)], high?"h":"l");
	} else if(is_target_spill(p->target)){
		print("set_rel_sp %s %d\n", tmp_reg, get_spill_sp_offset(get_target(p->target)*2 + high));
	} else {
		assert(0);
	}
}

void alu_1(Node p) {
	put_kid_to_reg8(p, 1, "b", 0);
	put_kid_to_reg8(p, 0, "a", 0);
	switch(generic(p->op)) {
		case BOR: 	print("alu or\n"); 	break;
		case BAND: 	print("alu and\n"); 	break;
		case BXOR: 	print("alu xor\n"); 	break;
		case ADD:	print("alu add\n");	break;
		case SUB:	print("alu sub\n");	break;
		default:		assert(0);				break;
	}
	put_reg8_to_target(p, "a", 0);
}

void alu_2(Node p) {
	put_kid_to_reg8(p, 1, "b", 0);
	put_kid_to_reg8(p, 0, "a", 0);

	switch(generic(p->op)) {
		case BOR:	print("alu or\n");	break;
		case BAND:	print("alu and\n");	break;
		case BXOR:	print("alu xor\n");	break;
		case ADD:	print("alu add\n");	break;
		case SUB:	print("alu sub\n");	break;
		default:		assert(0);				break;
	}

	put_reg8_to_target(p, "a", 0);

	put_kid_to_reg8(p, 1, "b", 1);
	put_kid_to_reg8(p, 0, "a", 1);

	switch(generic(p->op)) {
		case BOR:	print("alu or\n");	break;
		case BAND:	print("alu and\n");	break;
		case BXOR:	print("alu xor\n");	break;
		case ADD:	print("alu adc\n");	break;
		case SUB:	print("alu sbc\n");	break;
		default:		assert(0);				break;
	}

	put_reg8_to_target(p, "a", 1);

}


void cnst(Node p) {
	char * reg_name;

	if ((opsize(p->op)) == 1)
	{
		reg_name = get_target_reg_name(p, 0);
		if(reg_name) {
			print("lit %s %s\n", reg_name, p->syms[0]->x.name);
		} else {
			print("lit a %s\n", p->syms[0]->x.name);
			put_reg8_to_target(p, "a", 0);
		}
	}
	else if (opsize(p->op) == 2)
	{
		reg_name = get_target_reg_name(p, 0);
		if(reg_name) {
			print("litw %c %s\n", reg_name[0], p->syms[0]->x.name);
		} else {
			print("litw a %s\n", p->syms[0]->x.name);
			put_reg8_to_target(p, "a", 0);
			put_reg8_to_target(p, "b", 1);
		}
	}
	else
	{
		not_implemented()
	}
}


void addrg(Node p) {
	char * reg_name;

	reg_name = get_target_reg_name(p, 0);
	if(reg_name) {
		print("litw %c $%s\n", reg_name[0], p->syms[0]->x.name);
	} else {
		print("litw a %s\n", p->syms[0]->x.name);
		put_reg8_to_target(p, "a", 0);
		put_reg8_to_target(p, "b", 1);
	}
}

void addrl(Node p) {
	print("lit off %d\n", get_local_sp_offset(atoi(p->syms[0]->x.name)));
	print("seta sol\n");
	put_reg8_to_target(p, "a", 0);
	print("seta soh\n");
	put_reg8_to_target(p, "a", 1);
}

void addrf(Node p) {
	print("lit off %d\n", get_arg_sp_offset(atoi(p->syms[0]->x.name)));
	print("seta sol\n");
	put_reg8_to_target(p, "a", 0);
	print("seta soh\n");
	put_reg8_to_target(p, "a", 1);
}

void label(Node p) {
	print("%s:\n", p->syms[0]->x.name);
}

void cv(Node p) {
	char * reg_name_target;
	char * reg_name_src;

	if ((opsize(p->op)) == 1)
	{ //to size 1
		if (p->syms[0]->u.c.v.i == 1)
		{ //from size 1
			if(p->kids[0]->target == p->target) {
				print("; bypass conversion sz1-sz1\n");
			} else {
				reg_name_target = get_target_reg_name(p, 0);
				reg_name_src = get_target_reg_name(p, 0);
				if(reg_name_target) {
					
				}
			}
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


}
void nop(Node p) {
d_start();

d_end();
}
