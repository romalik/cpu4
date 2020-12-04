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

char * get_kid_reg_name(Node p, int kid, int high) {
	if(is_target_reg(p->kids[kid]->target)) {
		sprintf(__tmp_reg_name, "%s%s", reg16_names[get_target(p->kids[kid]->target)], high?"h":"l");
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
		print("put_rel_sp %s %d\n", tmp_reg, get_spill_sp_offset(get_target(p->target)*2 + high));
	} else {
		assert(0);
	}
}


void alu(Node p) {
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
  
  if(opsize(p->op) == 2) {
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
}

void alu_unary(Node p) {
	put_kid_to_reg8(p, 0, "a", 0);

	switch(generic(p->op)) {
		case BCOM: 	print("alu not\n"); 	break;
		case NEG: 	print("alu neg\n"); 	break;
		default:		assert(0);				break;
	}

	put_reg8_to_target(p, "a", 0);
  if(opsize(p->op) == 2) {
    put_kid_to_reg8(p, 0, "a", 1);

    switch(generic(p->op)) {
      case BCOM: 	print("alu not\n"); 	break;
      case NEG: 	print("alu neg\n"); 	break;
      default:		assert(0);				break;
    }

    put_reg8_to_target(p, "a", 1);
  }
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
		print("litw a $%s\n", p->syms[0]->x.name);
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

	if ((opsize(p->op)) == 1) { //to size 1
		if (p->syms[0]->u.c.v.i == 1) { //from size 1
			if(p->kids[0]->target == p->target) {
				print("; bypass conversion sz1-sz1\n");
			} else {
        put_kid_to_reg8(p, 0, "a", 0);
        put_reg8_to_target(p, "a", 0);
			}
		}
		else if (p->syms[0]->u.c.v.i == 2) { //from size 2
        put_kid_to_reg8(p, 0, "a", 0);
        put_reg8_to_target(p, "a", 0);
		}
		else {
			not_implemented()
		}
	}
	else if ((opsize(p->op)) == 2) { //to size 2
		if (p->syms[0]->u.c.v.i == 1) { //from size 1
        put_kid_to_reg8(p, 0, "a", 0);
        put_reg8_to_target(p, "a", 0);
        print("lit a 0\n");
        put_reg8_to_target(p, "a", 1);
		}
		else if (p->syms[0]->u.c.v.i == 2) { //from size 2
			if(p->kids[0]->target == p->target) {
				print("; bypass conversion sz1-sz1\n");
			} else {
        put_kid_to_reg8(p, 0, "a", 0);
        put_kid_to_reg8(p, 0, "b", 1);
        put_reg8_to_target(p, "a", 0);
        put_reg8_to_target(p, "b", 1);
			}
		}
		else {
			not_implemented()
		}
	}
	else {
		not_implemented();
	}
}

void arg(Node p) {
	char * reg_name_src;
  reg_name_src = get_kid_reg_name(p, 0, 0);
  if(opsize(p->op) == 1) {
    if(reg_name_src) {
      push(reg_name_src);
    } else {
      put_kid_to_reg8(p, 0, "a", 0);
      push("a");
    }
  } else if(opsize(p->op) == 2) {
    if(reg_name_src) {
      reg_name_src[1] = 0;
      pushw(reg_name_src);
    } else {
      put_kid_to_reg8(p, 0, "a", 0);
      put_kid_to_reg8(p, 0, "b", 1);
      pushw("a");
    }
  } else {
    not_implemented()
  }
}

void indir(Node p) {
  char * reg_name;
  reg_name = get_kid_reg_name(p,0,0);
  if(reg_name) {
    reg_name[1] = 0;
    print("setb m[%s]\n", reg_name);
  } else {
    put_kid_to_reg8(p, 0, "xl", 0);
    put_kid_to_reg8(p, 0, "xh", 1);
    print("setb m[x]\n");
  }

  if(opsize(p->op) == 2) {
    if(reg_name) {
      print("%s++\n", reg_name);
      print("seta m[%s]\n", reg_name);
    } else {
      print("x++\n");
      print("seta m[x]\n");
    }
    put_reg8_to_target(p, "a", 1);
  }

  put_reg8_to_target(p, "b", 0);

}

void jmp(Node p) {
	char * reg_name_src;
  reg_name_src = get_kid_reg_name(p, 0, 0);
  if(reg_name_src) {
    reg_name_src[1] = 0;
    pushw(reg_name_src);
  } else {
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 0, "b", 1);
    pushw("a");
  }

  print("jmps\n");
  current_sp_offset -= 2;
}

void ret(Node p) {
	char * reg_name_src;
  reg_name_src = get_kid_reg_name(p, 0, 0);
  if(opsize(p->op) == 0) {
    //do nothing
  } else if(opsize(p->op) == 1) {
    if(reg_name_src) {
  		print("put_rel_sp %s %d\n", reg_name_src, get_retval_sp_offset());
    } else {
      put_kid_to_reg8(p, 0, "a", 0);
  		print("put_rel_sp a %d\n", get_retval_sp_offset());
    }
  } else if(opsize(p->op) == 2) {
    if(reg_name_src) {
      reg_name_src[1] = 0;
  		print("put_rel_sp %sl %d\n", reg_name_src, get_retval_sp_offset());
  		print("put_rel_sp %sh %d\n", reg_name_src, get_retval_sp_offset()+1);
    } else {
      put_kid_to_reg8(p, 0, "a", 0);
      put_kid_to_reg8(p, 0, "b", 1);
  		print("put_rel_sp a %d\n", get_retval_sp_offset());
  		print("put_rel_sp b %d\n", get_retval_sp_offset()+1);
    }
  } else {
    not_implemented()
  }

  print("; create instruction for this !\n");
	print("lit off %d\n", get_retaddr_sp_offset()-1);
	print("seta sol\n");
  print("setb soh\n");
  print("puta sl\n");
  print("putb sh\n");
  print(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
  print("ret\n");
}

void call_op(Node p) {
  char * reg_name_src;
  char * reg_name_target;
  int total_arg_size = 0;
  int i;

  for (i = 0; i < 15; i++) {
    assert(p->args);
    if (!p->args[i])
      break;
    total_arg_size += (opsize(p->args[i]->op));
  }

  print("; reserve retval\n");
  for(i = 0; i<(opsize(p->op)); i++) {
    print("s--\n");
    current_sp_offset++;
  }

  reg_name_src = get_kid_reg_name(p, 0, 0);
  if(reg_name_src) {
    reg_name_src[1] = 0;
    pushw(reg_name_src);
  } else {
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 0, "b", 1);
    pushw("a");
  }

  print("calls\n");
  current_sp_offset -= 2;

  reg_name_target = get_target_reg_name(p, 0);

  if(reg_name_target) {
    if(opsize(p->op) == 0) {
    } else if(opsize(p->op) == 1) {
      pop(reg_name_target);
    } else if(opsize(p->op) == 2) {
      reg_name_target[1] = 0;
      popw(reg_name_target);
    } else {
      not_implemented()
    }
  } else {
    if(opsize(p->op) == 0) {
    } else if(opsize(p->op) == 1) {
      pop("a");
      put_reg8_to_target(p, "a", 0);
    } else if(opsize(p->op) == 2) {
      popw("a");
      put_reg8_to_target(p, "a", 0);
      put_reg8_to_target(p, "b", 1);
    } else {
      not_implemented()
    }
  }

  print("; create instruction for this !\n");
	print("lit off %d\n", total_arg_size);
	print("seta sol\n");
  print("setb soh\n");
  print("puta sl\n");
  print("putb sh\n");
  print(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
  current_sp_offset -= total_arg_size;
}

void asgn_op(Node p) {
  char * reg_name_addr;
  char * reg_name_src;
  
  char reg_mem[5];
  reg_name_addr = get_kid_reg_name(p, 0, 0);
  reg_name_src = get_kid_reg_name(p, 1, 0);

  if(reg_name_addr) {
    reg_name_addr[1] = 0;
    sprintf(reg_mem, "m[%s]", reg_name_addr);
    put_kid_to_reg8(p, 1, reg_mem, 0);
  } else {
    put_kid_to_reg8(p, 0, "xl", 0);
    put_kid_to_reg8(p, 0, "xh", 1);
    put_kid_to_reg8(p, 1, "m[x]", 0);
  }

  if(opsize(p->op) == 2) {
    if(reg_name_addr) {
      print("%s++\n", reg_name_addr);
      put_kid_to_reg8(p, 1, reg_mem, 1);
    } else {
      print("x++\n");
      put_kid_to_reg8(p, 1, "m[x]", 1);
    }
  }
}


void cond_br(Node p) {
  char * cmd;
  char is_signed;
  int i;

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

  if(opsize(p->op) == 1) {
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 1, "b", 0);
    print("cmp sub\n");
    print("jmp %s %s $%s\n", (is_signed?"s":""), cmd, p->syms[0]->x.name);

  } else if(opsize(p->op) == 2) {
    // compare high bytes, if equal - jump to low byte comparison
    put_kid_to_reg8(p, 0, "a", 1);
    put_kid_to_reg8(p, 1, "b", 1);

    print("cmp sub\n");

    i = genlabel(1);

    print("jmp e $__jump_label__%d\n", i);
    if ((generic(p->op)) != EQ)
      print("jmp %s %s $%s\n", (is_signed?"s":""), cmd, p->syms[0]->x.name);
    print("jmp $__jump_label_2__%d\n", i);
    print("__jump_label__%d:\n", i);

    // compare low bytes if high equal
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 1, "b", 0);

    print("cmp sub\n");
    print("jmp %s %s $%s\n", (is_signed?"s":""), cmd, p->syms[0]->x.name);

    print("__jump_label_2__%d:\n", i);
  }
}

void shl(Node p) {
  int n;
  int i;

  if (generic(p->kids[1]->op) == CNST) {
    n = atoi(p->kids[1]->syms[0]->x.name);
  } else {
    not_implemented()
  }

  for(i = 0; i<n; i++) {
    if(i == 0) {
      put_kid_to_reg8(p, 0, "a", 0);
    } else {
      print("seta xl\n");
    }
  	print("alu shl\n"); break;
    

    if(i == n-1) {
  	  put_reg8_to_target(p, "a", 0);
    } else {
      print("puta xl\n");
    }

    if(opsize(p->op) == 2) {
      if(i == 0) {
        put_kid_to_reg8(p, 0, "a", 1);
      } else {
        print("seta xh\n");
      }

      print("alu shlc\n"); break;

      if(i == n-1) {
        put_reg8_to_target(p, "a", 1);
      } else {
        print("puta xh\n");
      }

    }
  }
}

void shr(Node p) {
  int n;
  int i;

  if (generic(p->kids[1]->op) == CNST) {
    n = atoi(p->kids[1]->syms[0]->x.name);
  } else {
    not_implemented()
  }

  for(i = 0; i<n; i++) {


    if(opsize(p->op) == 2) {
      if(i == 0) {
        put_kid_to_reg8(p, 0, "a", 1);
      } else {
        print("seta xh\n");
      }

      print("alu shr\n");

      if(i == n-1) {
        put_reg8_to_target(p, "a", 1);
      } else {
        print("puta xh\n");
      }

    }



    if(i == 0) {
      put_kid_to_reg8(p, 0, "a", 0);
    } else {
      print("seta xl\n");
    }
  	print("alu shrc\n");

    if(i == n-1) {
  	  put_reg8_to_target(p, "a", 0);
    } else {
      print("puta xl\n");
    }

  }
}

void nop(Node p) {
  fprintf(stderr, "Not implemented %s\n", opname(p->op));
  assert(0);
}


