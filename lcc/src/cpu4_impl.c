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
	int need_move = 0;

	if(is_target_reg(p->kids[kid]->target)) {
    need_move = 1;
    if(reg[0] == 'a') {
      tmp_reg = "a";
      need_move = 0;
    } else if(reg[0] == 'b') {
      tmp_reg = "b";
      need_move = 0;
    }
		print("set%s %s%s\n", tmp_reg, reg16_names[get_target(p->kids[kid]->target)], high?"h":"l");
    if(need_move) {
      print("put%s %s\n", tmp_reg, reg);
    }

	} else if(is_target_spill(p->kids[kid]->target)) {
		print("get_rel_sp %s %d\n", reg, get_spill_sp_offset(get_target(p->kids[kid]->target)*2 + high));
	} else {
		fprintf(stderr, "target: 0x%04x\n", p->kids[kid]->target);
		assert(0);
	}


}


void put_kid_to_reg16(Node p, int kid, char * reg) {
	if(is_target_reg(p->kids[kid]->target)) {
    char regl[6];
    char regh[6];

    if(reg[0] == 'a') {
      sprintf(regl, "a");
      sprintf(regh, "b");
    } else {
      sprintf(regl, "%sl", reg);
      sprintf(regh, "%sh", reg);
    }

    put_kid_to_reg8(p, kid, regl, 0);
    put_kid_to_reg8(p, kid, regh, 1);
	} else if(is_target_spill(p->kids[kid]->target)) {
		print("get_rel_sp_w %s %d\n", reg, get_spill_sp_offset(get_target(p->kids[kid]->target)*2));
	} else {
		fprintf(stderr, "target: 0x%04x\n", p->kids[kid]->target);
		assert(0);
	}


}



void put_reg8_to_target(Node p, char * reg, int high) {

	char * tmp_reg = "a";
	int need_move = 0;

	if(is_target_reg(p->target)) {
    need_move = 1;
    if(reg[0] == 'a') {
      tmp_reg = "a";
      need_move = 0;
    } else if(reg[0] == 'b') {
      tmp_reg = "b";
      need_move = 0;
    }

		print("put%s %s%s\n", tmp_reg, reg16_names[get_target(p->target)], high?"h":"l");

    if(need_move) {
      print("set%s %s\n", tmp_reg, reg);
    }

	} else if(is_target_spill(p->target)){
		print("put_rel_sp %s %d\n", reg, get_spill_sp_offset(get_target(p->target)*2 + high));
	} else {
		assert(0);
	}
}

void put_reg16_to_target(Node p, char * reg) {

	if(is_target_reg(p->target)) {
    char regl[6];
    char regh[6];

    if(reg[0] == 'a') {
      sprintf(regl, "a");
      sprintf(regh, "b");
    } else {
      sprintf(regl, "%sl", reg);
      sprintf(regh, "%sh", reg);
    }

    put_reg8_to_target(p, regl, 0);
    put_reg8_to_target(p, regh, 1);
	} else if(is_target_spill(p->target)){
		print("put_rel_sp_w %s %d\n", reg, get_spill_sp_offset(get_target(p->target)*2));
	} else {
		assert(0);
	}
}


void alu(Node p) {

  if( is_target_spill(p->target) &&
      is_target_spill(p->kids[0]->target) &&
      is_target_spill(p->kids[1]->target)) {
    
    int alus_2 = 0;
    char * cmd;
    int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
    int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);
    int off_target = get_spill_sp_offset(get_target(p->target)*2);
    
    if(off_target == off_arg_0) {
      alus_2 = 1;
    }


    switch(generic(p->op)) {
      case BOR:	  cmd = "or";	  break;
      case BAND:	cmd = "and";	break;
      case BXOR:	cmd = "xor";	break;
      case ADD:	  cmd = "add";	break;
      case SUB:	  cmd = "sub";	break;
      default:		assert(0);		break;
    }

    if(alus_2) {
      print("alus2 %s %d %d\n", cmd, off_arg_1, off_arg_0);
    } else {
      print("alus3 %s %d %d %d\n", cmd, off_arg_1, off_arg_0, off_target);
    }

    if(opsize(p->op) == 2) {
      off_arg_0 += 1;
      off_arg_1 += 1;
      off_target += 1;

      switch(generic(p->op)) {
        case BOR:	  cmd = "or";	  break;
        case BAND:	cmd = "and";	break;
        case BXOR:	cmd = "xor";	break;
        case ADD:	  cmd = "adc";	break;
        case SUB:	  cmd = "sbc";	break;
        default:		assert(0);		break;
      }

      if(alus_2) {
        print("alus2 %s %d %d\n", cmd, off_arg_1, off_arg_0);
      } else {
        print("alus3 %s %d %d %d\n", cmd, off_arg_1, off_arg_0, off_target);
      }
    }
  } else {

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
}


void alu4(Node p) {
  int i;
  int alus_2 = 0;
  char * cmd;
  int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
  int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);
  int off_target = get_spill_sp_offset(get_target(p->target)*2);
  
  if(off_target == off_arg_0) {
    alus_2 = 1;
  }


  switch(generic(p->op)) {
    case BOR:	  cmd = "or";	  break;
    case BAND:	cmd = "and";	break;
    case BXOR:	cmd = "xor";	break;
    case ADD:	  cmd = "add";	break;
    case SUB:	  cmd = "sub";	break;
    default:		assert(0);		break;
  }

  if(alus_2) {
    print("alus2 %s %d %d\n", cmd, off_arg_1, off_arg_0);
  } else {
    print("alus3 %s %d %d %d\n", cmd, off_arg_1, off_arg_0, off_target);
  }

  for(i = 0; i<3; i++) {
    off_arg_0 += 1;
    off_arg_1 += 1;
    off_target += 1;

    switch(generic(p->op)) {
      case BOR:	  cmd = "or";	  break;
      case BAND:	cmd = "and";	break;
      case BXOR:	cmd = "xor";	break;
      case ADD:	  cmd = "adc";	break;
      case SUB:	  cmd = "sbc";	break;
      default:		assert(0);		break;
    }

    if(alus_2) {
      print("alus2 %s %d %d\n", cmd, off_arg_1, off_arg_0);
    } else {
      print("alus3 %s %d %d %d\n", cmd, off_arg_1, off_arg_0, off_target);
    }
  }
}


void alu_unary(Node p) {
  if(generic(p->op) == BCOM) {
    put_kid_to_reg8(p, 0, "a", 0);

    print("alu not\n");

    put_reg8_to_target(p, "a", 0);
    if(opsize(p->op) == 2) {
      put_kid_to_reg8(p, 0, "a", 1);

      print("alu not\n");

      put_reg8_to_target(p, "a", 1);
    }
  } else if(generic(p->op) == NEG) {
    print("lit a 0\n");
    put_kid_to_reg8(p, 0, "b", 0);
    print("alu sub\n");

    put_reg8_to_target(p, "a", 0);
    if(opsize(p->op) == 2) {
      print("lit a 0\n");
      put_kid_to_reg8(p, 0, "b", 1);
      print("alu sbc\n");
      put_reg8_to_target(p, "a", 1);
    }

  } else {
    assert(0);
  }
}

void alu_unary4(Node p) {
  int i;
  char * cmd;
  int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
  int off_target = get_spill_sp_offset(get_target(p->target)*2);
  
  if(off_target != off_arg_0) {
    fprintf(stderr, "alu_unary_4 src != dst\n");
    assert(0);
  }

  if(generic(p->op) == BCOM) {

    print("alus1 not %d\n", off_arg_0);

    for(i = 0; i<3; i++) {
      off_arg_0 += 1;
      off_target += 1;
      print("alus1 not %d\n", off_arg_0);
    }
  } else if(generic(p->op) == NEG) {
    print("lit a 0\n");
    put_kid_to_reg8(p, 0, "b", 0);
    print("alu sub\n");

    put_reg8_to_target(p, "a", 0);
    for(i = 0; i<3; i++) {
      print("lit a 0\n");
      put_kid_to_reg8(p, 0, "b", 1+i);
      print("alu sbc\n");
      put_reg8_to_target(p, "a", 1+i);
    }
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
			put_reg16_to_target(p, "a");
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
		put_reg16_to_target(p, "a");
	}
}
void iaddrg(Node p) {
  print("litw x $%s\n", p->custom_data);
  print("seta m[x]\n");

  if(opsize(p->op) == 2) {
    print("x++\n");
    print("setb m[x]\n");
    put_reg16_to_target(p, "a");
  } else {
    put_reg8_to_target(p, "a", 0);
  }
}
void asgng(Node p) {
  print("litw x $%s\n", p->custom_data);
  if(opsize(p->op) == 1) {
    put_kid_to_reg8(p, 1, "a", 0);
    print("puta m[x]\n");
  } else if(opsize(p->op) == 2) {
    put_kid_to_reg16(p, 1, "a");
    print("puta m[x]\n");
    print("x++\n");
    print("putb m[x]\n");
  }
}

void addrl(Node p) {
  
  char * target_reg = get_target_reg_name(p, 0);
  if(target_reg) {
    target_reg[1] = 0;
  	print("adjust_sp %s %d\n", target_reg, get_local_sp_offset(calculate_offset_sum(p->syms[0]->x.name)));
  } else {
  	print("adjust_sp a %d\n", get_local_sp_offset(calculate_offset_sum(p->syms[0]->x.name)));
  	put_reg16_to_target(p, "a");

  }
}

void iaddrl(Node p) {
  if(opsize(p->op) == 2) {
    print("get_rel_sp_w a %d\n", get_local_sp_offset(calculate_offset_sum(p->custom_data)));
    put_reg16_to_target(p, "a");
  } else {
    print("get_rel_sp a %d\n", get_local_sp_offset(calculate_offset_sum(p->custom_data)));
    put_reg8_to_target(p, "a", 0);
  }
}
void asgnl(Node p) {
  if(opsize(p->op) == 2) {
    put_kid_to_reg16(p, 1, "a");
    print("put_rel_sp_w a %d\n", get_local_sp_offset(calculate_offset_sum(p->custom_data)));
  } else {
    put_kid_to_reg8(p, 1, "a", 0);
    print("put_rel_sp a %d\n", get_local_sp_offset(calculate_offset_sum(p->custom_data)));
  }
}


void addrf(Node p) {
  char * target_reg = get_target_reg_name(p, 0);
  if(target_reg) {
    target_reg[1] = 0;
  	print("adjust_sp %s %d\n", target_reg, get_arg_sp_offset(calculate_offset_sum(p->syms[0]->x.name)));
  } else {
  	print("adjust_sp a %d\n", get_arg_sp_offset(calculate_offset_sum(p->syms[0]->x.name)));
  	put_reg16_to_target(p, "a");

  }
}

void iaddrf(Node p) {
  if(opsize(p->op) == 2) {
    print("get_rel_sp_w a %d\n", get_arg_sp_offset(calculate_offset_sum(p->custom_data)));
    put_reg16_to_target(p, "a");
  } else {
    print("get_rel_sp a %d\n", get_arg_sp_offset(calculate_offset_sum(p->custom_data)));
    put_reg8_to_target(p, "a", 0);
  }
}

void asgnf(Node p) {
  if(opsize(p->op) == 2) {
    put_kid_to_reg16(p, 1, "a");
    print("put_rel_sp_w a %d\n", get_arg_sp_offset(calculate_offset_sum(p->custom_data)));
  } else {
    put_kid_to_reg8(p, 1, "a", 0);
    print("put_rel_sp a %d\n", get_arg_sp_offset(calculate_offset_sum(p->custom_data)));
  }
}

void label(Node p) {
	print("%s:\n", p->syms[0]->x.name);
}

void cv(Node p) {
  int is_signed;
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
		} else if (p->syms[0]->u.c.v.i == 4) { //from size 4, same
        put_kid_to_reg8(p, 0, "a", 0);
        put_reg8_to_target(p, "a", 0);
		} else {
			not_implemented()
		}
	}
	else if ((opsize(p->op)) == 2) { //to size 2
		if (p->syms[0]->u.c.v.i == 1) { //from size 1
        put_kid_to_reg8(p, 0, "a", 0);
        put_reg8_to_target(p, "a", 0);

        //sign extension
        if(is_signed) {
          int i = genlabel(1);
          print("lit b 0x80\n");
          print("cmp and\n");
          print("lit b 0\n");
          print("jmp z $__label_%d\n", i);
          print("lit b 0xff\n");
          print("__label_%d:\n", i);
          put_reg8_to_target(p, "b", 1);

        } else {
          print("lit b 0\n");
          put_reg8_to_target(p, "b", 1);

        }

		}
		else if (p->syms[0]->u.c.v.i == 2) { //from size 2
			if(p->kids[0]->target == p->target) {
				print("; bypass conversion sz1-sz1\n");
			} else {
        put_kid_to_reg16(p, 0, "a");
        put_reg16_to_target(p, "a");
			}
		} else if (p->syms[0]->u.c.v.i == 4) { //from size 4, same
        put_kid_to_reg16(p, 0, "a");
        put_reg16_to_target(p, "a");
		} else {
			not_implemented()
		}
	}
	else if ((opsize(p->op)) == 4) { //to size 4
		if (p->syms[0]->u.c.v.i == 1) { //from size 1
        put_kid_to_reg8(p, 0, "a", 0);
        put_reg8_to_target(p, "a", 0);

        //sign extension
        if(is_signed) {
          int i = genlabel(1);
          print("lit b 0x80\n");
          print("cmp and\n");
          print("lit b 0\n");
          print("jmp z $__label_%d\n", i);
          print("lit b 0xff\n");
          print("__label_%d:\n", i);
          put_reg8_to_target(p, "b", 1);
          put_reg8_to_target(p, "b", 2);
          put_reg8_to_target(p, "b", 3);

        } else {
          print("lit b 0\n");
          put_reg8_to_target(p, "b", 1);
          put_reg8_to_target(p, "b", 2);
          put_reg8_to_target(p, "b", 3);

        }

		}
		else if (p->syms[0]->u.c.v.i == 2) { //from size 2
        put_kid_to_reg16(p, 0, "a");
        put_reg16_to_target(p, "a");

        //sign extension
        if(is_signed) {
          int i = genlabel(1);
          print("lit a 0x80\n");
          print("cmp and\n");
          print("lit a 0\n");
          print("jmp z $__label_%d\n", i);
          print("lit a 0xff\n");
          print("__label_%d:\n", i);
          put_reg8_to_target(p, "a", 2);
          put_reg8_to_target(p, "a", 3);

        } else {
          print("lit a 0\n");
          put_reg8_to_target(p, "a", 2);
          put_reg8_to_target(p, "a", 3);

        }
		} else if (p->syms[0]->u.c.v.i == 4) { //from size 4, same
			if(p->kids[0]->target == p->target) {
				print("; bypass conversion sz4-sz4\n");
			} else {
        print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[0]->target)*2));
        print("put_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->target)*2));
        print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[0]->target)*2) + 2);
        print("put_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->target)*2) + 2);
			}
		} else {
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
      put_kid_to_reg16(p, 0, "a");
      pushw("a");
    }
  } else if(opsize(p->op) == 4) {
    print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[0]->target)*2) + 2);
    pushw("a");
    print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[0]->target)*2));
    pushw("a");
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
    put_kid_to_reg16(p, 0, "x");
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

void indir4(Node p) {
  char * reg_name;
  reg_name = get_kid_reg_name(p,0,0);
  if(reg_name) {
    reg_name[1] = 0;
  } else {
    put_kid_to_reg16(p, 0, "x");
    reg_name = "x";
  }

  print("seta m[%s]\n", reg_name);
  print("put_rel_sp a %d\n" ,get_spill_sp_offset(get_target(p->target)*2));
  print("%s++\n", reg_name);
  print("seta m[%s]\n", reg_name);
  print("put_rel_sp a %d\n" ,get_spill_sp_offset(get_target(p->target)*2) + 1);
  print("%s++\n", reg_name);
  print("seta m[%s]\n", reg_name);
  print("put_rel_sp a %d\n" ,get_spill_sp_offset(get_target(p->target)*2) + 2);
  print("%s++\n", reg_name);
  print("seta m[%s]\n", reg_name);
  print("put_rel_sp a %d\n" ,get_spill_sp_offset(get_target(p->target)*2) + 3);
}

void jmp(Node p) {
	char * reg_name_src;
  reg_name_src = get_kid_reg_name(p, 0, 0);
  if(reg_name_src) {
    reg_name_src[1] = 0;
    pushw(reg_name_src);
  } else {
    put_kid_to_reg16(p, 0, "a");
    pushw("a");
  }

  print("jmps\n");
  current_sp_offset -= 2;
}

void jmpc(Node p) {
  print("jmp $%s\n", p->custom_data);
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
  		print("put_rel_sp_w %s %d\n", reg_name_src, get_retval_sp_offset());
    } else {
      put_kid_to_reg16(p, 0, "a");
  		print("put_rel_sp_w a %d\n", get_retval_sp_offset());
    }
  } else if(opsize(p->op) == 4) {
    print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[0]->target)*2 + 2));
    print("put_rel_sp_w a %d\n", get_retval_sp_offset() + 2);
    print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[0]->target)*2));
    print("put_rel_sp_w a %d\n", get_retval_sp_offset());
  } else {
    not_implemented()
  }

	print("adjust_sp s %d\n", get_retaddr_sp_offset()-1);
/*
  print("; create instruction for this !\n");
	print("lit off %d\n", get_retaddr_sp_offset()-1);
	print("seta sol\n");
  print("setb soh\n");
  print("puta sl\n");
  print("putb sh\n");
  print(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
*/
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
    put_kid_to_reg16(p, 0, "a");
    pushw("a");
  }

  print("calls\n");
  current_sp_offset -= 2;

  if(is_target(p->target)) {
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
        put_reg16_to_target(p, "a");
      } else {
        not_implemented()
      }
    }

    print("adjust_sp s %d\n", total_arg_size);
    current_sp_offset -= total_arg_size;
  } else {
    //unused retval 
    //discard it
    print("adjust_sp s %d\n", total_arg_size+opsize(p->op));
    current_sp_offset -= (total_arg_size+opsize(p->op));
  }
}

void callc(Node p) {
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

  print("call $%s\n", p->custom_data);

  if(is_target(p->target)) {
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
        put_reg16_to_target(p, "a");
      } else if(opsize(p->op) == 4) {
        popw("a");
        print("put_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->target)*2));
        popw("a");
        print("put_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->target)*2) + 2);      } else {
      }
    }

    print("adjust_sp s %d\n", total_arg_size);
    current_sp_offset -= total_arg_size;
  } else {
    //unused retval 
    //discard it
    print("adjust_sp s %d\n", total_arg_size+opsize(p->op));
    current_sp_offset -= (total_arg_size+opsize(p->op));
  }
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
    put_kid_to_reg16(p, 0, "x");
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

void asgn4(Node p) {

  print("get_rel_sp_w x %d\n", get_spill_sp_offset(get_target(p->kids[0]->target)*2));

  print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[1]->target)*2));
  print("puta m[x]\n");
  print("x++\n");
  print("putb m[x]\n");
  print("x++\n");
  print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[1]->target)*2 + 2));
  print("puta m[x]\n");
  print("x++\n");
  print("putb m[x]\n");

}


void cond_g(Node p, int is_signed) {
  int i;
  if(opsize(p->op) == 1) {
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 1, "b", 0);
    print("cmp sub\n");
    print("jmp %s $%s\n", is_signed?"g":"a", p->syms[0]->x.name);

  } else if(opsize(p->op) == 2) {

    assert( is_target_spill(p->kids[0]->target) &&
            is_target_spill(p->kids[1]->target));
      
      int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
      int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);

    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp %s $__jmp_label_false_%d\n", is_signed?"l":"b", i);

    print("cmps2 sub %d %d\n", off_arg_1, off_arg_0);
    print("jmp a $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);

  } else if(opsize(p->op) == 4) {

    assert( is_target_spill(p->kids[0]->target) &&
        is_target_spill(p->kids[1]->target));
      
    int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
    int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);
    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+3, off_arg_0+3);
    print("jmp %s $__jmp_label_false_%d\n", is_signed?"l":"b", i);

    print("cmps2 sub %d %d\n", off_arg_1+2, off_arg_0+2);
    print("jmp b $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp b $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+0, off_arg_0+0);
    print("jmp a $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);
  }

}
void cond_ge(Node p, int is_signed) {
  int i;
  if(opsize(p->op) == 1) {
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 1, "b", 0);
    print("cmp sub\n");
    print("jmp %s $%s\n", is_signed?"ge":"ae", p->syms[0]->x.name);

  } else if(opsize(p->op) == 2) {

    assert( is_target_spill(p->kids[0]->target) &&
            is_target_spill(p->kids[1]->target));
      
      int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
      int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);

    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp %s $__jmp_label_false_%d\n", is_signed?"l":"b", i);

    print("cmps2 sub %d %d\n", off_arg_1, off_arg_0);
    print("jmp ae $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);

  } else if(opsize(p->op) == 4) {

    assert( is_target_spill(p->kids[0]->target) &&
        is_target_spill(p->kids[1]->target));
      
    int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
    int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);
    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+3, off_arg_0+3);
    print("jmp %s $__jmp_label_false_%d\n", is_signed?"l":"b", i);

    print("cmps2 sub %d %d\n", off_arg_1+2, off_arg_0+2);
    print("jmp b $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp b $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+0, off_arg_0+0);
    print("jmp ae $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);
  }

  
}
void cond_l(Node p, int is_signed) {
  int i;
  if(opsize(p->op) == 1) {
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 1, "b", 0);
    print("cmp sub\n");
    print("jmp %s $%s\n", is_signed?"l":"b", p->syms[0]->x.name);

  } else if(opsize(p->op) == 2) {

    assert( is_target_spill(p->kids[0]->target) &&
            is_target_spill(p->kids[1]->target));
      
      int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
      int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);

    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp %s $__jmp_label_false_%d\n", is_signed?"g":"a", i);

    print("cmps2 sub %d %d\n", off_arg_1, off_arg_0);
    print("jmp b $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);

  } else if(opsize(p->op) == 4) {

    assert( is_target_spill(p->kids[0]->target) &&
        is_target_spill(p->kids[1]->target));
      
    int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
    int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);
    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+3, off_arg_0+3);
    print("jmp %s $__jmp_label_false_%d\n", is_signed?"g":"a", i);

    print("cmps2 sub %d %d\n", off_arg_1+2, off_arg_0+2);
    print("jmp a $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp a $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+0, off_arg_0+0);
    print("jmp b $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);
  }
  
}
void cond_le(Node p, int is_signed) {
  int i;
  if(opsize(p->op) == 1) {
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 1, "b", 0);
    print("cmp sub\n");
    print("jmp %s $%s\n", is_signed?"le":"be", p->syms[0]->x.name);

  } else if(opsize(p->op) == 2) {

    assert( is_target_spill(p->kids[0]->target) &&
            is_target_spill(p->kids[1]->target));
      
      int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
      int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);

    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp %s $__jmp_label_false_%d\n", is_signed?"g":"a", i);

    print("cmps2 sub %d %d\n", off_arg_1, off_arg_0);
    print("jmp be $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);

  } else if(opsize(p->op) == 4) {

    assert( is_target_spill(p->kids[0]->target) &&
        is_target_spill(p->kids[1]->target));
      
    int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
    int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);
    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+3, off_arg_0+3);
    print("jmp %s $__jmp_label_false_%d\n", is_signed?"g":"a", i);

    print("cmps2 sub %d %d\n", off_arg_1+2, off_arg_0+2);
    print("jmp a $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp a $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+0, off_arg_0+0);
    print("jmp be $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);
  }
  
}
void cond_eq(Node p, int is_signed) {
  int i;
  if(opsize(p->op) == 1) {
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 1, "b", 0);
    print("cmp sub\n");
    print("jmp %s $%s\n", "e", p->syms[0]->x.name);

  } else if(opsize(p->op) == 2) {

    assert( is_target_spill(p->kids[0]->target) &&
            is_target_spill(p->kids[1]->target));
      
      int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
      int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);

    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp ne $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1, off_arg_0);
    print("jmp e $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);

  } else if(opsize(p->op) == 4) {

    assert( is_target_spill(p->kids[0]->target) &&
        is_target_spill(p->kids[1]->target));
      
    int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
    int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);
    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+3, off_arg_0+3);
    print("jmp ne $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+2, off_arg_0+2);
    print("jmp ne $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp ne $__jmp_label_false_%d\n", i);

    print("cmps2 sub %d %d\n", off_arg_1+0, off_arg_0+0);
    print("jmp e $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);
  }
  
}
void cond_ne(Node p, int is_signed) {
  int i;
  if(opsize(p->op) == 1) {
    put_kid_to_reg8(p, 0, "a", 0);
    put_kid_to_reg8(p, 1, "b", 0);
    print("cmp sub\n");
    print("jmp %s $%s\n", "ne", p->syms[0]->x.name);

  } else if(opsize(p->op) == 2) {

    assert( is_target_spill(p->kids[0]->target) &&
            is_target_spill(p->kids[1]->target));
      
      int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
      int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);

    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp ne $%s\n", p->syms[0]->x.name);

    print("cmps2 sub %d %d\n", off_arg_1, off_arg_0);
    print("jmp ne $%s\n", p->syms[0]->x.name);


  } else if(opsize(p->op) == 4) {

    assert( is_target_spill(p->kids[0]->target) &&
        is_target_spill(p->kids[1]->target));
      
    int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
    int off_arg_1 = get_spill_sp_offset(get_target(p->kids[1]->target)*2);
    i = genlabel(1);

    print("cmps2 sub %d %d\n", off_arg_1+3, off_arg_0+3);
    print("jmp ne $%s\n", p->syms[0]->x.name);

    print("cmps2 sub %d %d\n", off_arg_1+2, off_arg_0+2);
    print("jmp ne $%s\n", p->syms[0]->x.name);

    print("cmps2 sub %d %d\n", off_arg_1+1, off_arg_0+1);
    print("jmp ne $%s\n", p->syms[0]->x.name);

    print("cmps2 sub %d %d\n", off_arg_1+0, off_arg_0+0);
    print("jmp ne $%s\n", p->syms[0]->x.name);

    print("__jmp_label_false_%d:\n", i);
  }
  
}


void cond_br(Node p) {
  char * cmd_32_high;
  char * cmd_32_mid;
  char * cmd_32_low;
  char * cmd_8;

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
    cond_eq(p, is_signed);
    break;
  case NE:
    cond_ne(p, is_signed);
    break;
  case GT:
    cond_g(p, is_signed);
    break;		
  case GE:
    cond_ge(p, is_signed);
    break;
  case LE:
    cond_le(p, is_signed);
    break;
  case LT:
    cond_l(p, is_signed);
    break;
  default:
    assert(0);
  }
}

void shl(Node p) {
  int n;
  int i;

  if (generic(p->kids[1]->op) == CNST) {
    n = atoi(p->kids[1]->syms[0]->x.name);
  } else {
    intr_fn(p);
    return;
  }

  for(i = 0; i<n; i++) {
    if(i == 0) {
      put_kid_to_reg8(p, 0, "a", 0);
    } else {
      print("seta xl\n");
    }
  	print("alu shl\n");
    

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

      print("alu shlc\n");

      if(i == n-1) {
        put_reg8_to_target(p, "a", 1);
      } else {
        print("puta xh\n");
      }

    }
  }
}

void shl4(Node p) {
  int n;
  int i;
  int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
  int off_target = get_spill_sp_offset(get_target(p->target)*2);

  if (generic(p->kids[1]->op) == CNST) {
    n = atoi(p->kids[1]->syms[0]->x.name);
  } else {
    intr_fn(p);
    return;
  }

  for(i = 0; i<n; i++) {
  	print("alus1 shl %d\n", off_arg_0);
  	print("alus1 shlc %d\n", off_arg_0+1);
  	print("alus1 shlc %d\n", off_arg_0+2);
  	print("alus1 shlc %d\n", off_arg_0+3);
  }

  if(off_arg_0 != off_target) {
    print("get_rel_sp_w a %d\n", off_arg_0);
    print("put_rel_sp_w a %d\n", off_target);
    print("get_rel_sp_w a %d\n", off_arg_0+2);
    print("put_rel_sp_w a %d\n", off_target+2);
  }

}

void shr(Node p) {
  int n;
  int i;

  if (generic(p->kids[1]->op) == CNST) {
    n = atoi(p->kids[1]->syms[0]->x.name);
  } else {
    intr_fn(p);
    return;
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

void shr4(Node p) {
  int n;
  int i;
  int off_arg_0 = get_spill_sp_offset(get_target(p->kids[0]->target)*2);
  int off_target = get_spill_sp_offset(get_target(p->target)*2);

  if (generic(p->kids[1]->op) == CNST) {
    n = atoi(p->kids[1]->syms[0]->x.name);
  } else {
    intr_fn(p);
    return;
  }

  for(i = 0; i<n; i++) {
  	print("alus1 shr %d\n", off_arg_0+3);
  	print("alus1 shrc %d\n", off_arg_0+2);
  	print("alus1 shrc %d\n", off_arg_0+1);
  	print("alus1 shrc %d\n", off_arg_0);
  }

  if(off_arg_0 != off_target) {
    print("get_rel_sp_w a %d\n", off_arg_0);
    print("put_rel_sp_w a %d\n", off_target);
    print("get_rel_sp_w a %d\n", off_arg_0+2);
    print("put_rel_sp_w a %d\n", off_target+2);
  }

}



void intr_fn(Node p) {
  int i;
  int args_sz = 0;
  char * reg_name_target;
  if(opsize(p->op) == 1) {
    if(p->kids[0]) {
      if(is_target_spill(p->kids[0]->target)) {
        put_kid_to_reg8(p, 0, "xl", 0);        
      }
      push("xl");
      args_sz++;
    }
    if(p->kids[1]) {
      if(is_target_spill(p->kids[1]->target)) {
        put_kid_to_reg8(p, 0, "yl", 0);        
      }
      push("yl");
      args_sz++;
    }
  } else if(opsize(p->op) == 2) {
    if(p->kids[0]) {
      if(is_target_spill(p->kids[0]->target)) {
        put_kid_to_reg16(p, 0, "x");        
      }
      pushw("x");
      args_sz+=2;
    }
    if(p->kids[1]) {
      if(is_target_spill(p->kids[1]->target)) {
        put_kid_to_reg16(p, 0, "y");        
      }
      pushw("y");
      args_sz+=2;
    }
  } else if(opsize(p->op) == 4) {
    if(p->kids[0]) {
      print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[0]->target)*2) + 2);
      pushw("a");
      print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[0]->target)*2));
      pushw("a");

      args_sz+=4;
    }
    if(p->kids[1]) {
      print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[1]->target)*2) + 2);
      pushw("a");
      print("get_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->kids[1]->target)*2));
      pushw("a");

      args_sz+=4;
    }
  }

  print("; reserve retval\n");

  for(i = 0; i<(opsize(p->op)); i++) {
    print("s--\n");
    current_sp_offset++;
  }



  print("call $__crt_%s\n", opname(p->op));

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
      put_reg16_to_target(p, "a");
    } else if(opsize(p->op) == 4) {
      popw("a");
      print("put_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->target)*2));
      popw("a");
      print("put_rel_sp_w a %d\n", get_spill_sp_offset(get_target(p->target)*2) + 2);
    } else {
      not_implemented()
    }
  }
	print("adjust_sp s %d\n", args_sz);

  current_sp_offset -= args_sz;

}


void nop(Node p) {
  fprintf(stderr, "Not implemented %s\n", opname(p->op));
  assert(0);
}


