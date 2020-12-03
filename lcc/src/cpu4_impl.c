#include "c.h"
#include "cpu4_impl.h"


void cnst_0(Node p) {

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

}

void nop(Node p) {
d_start();

d_end();
}
