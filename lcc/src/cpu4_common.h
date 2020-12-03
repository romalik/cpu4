extern int current_sp_offset;
extern int current_func_retsize;


#define get_spill_sp_offset(X) (current_sp_offset + 1 + (X))
#define get_local_sp_offset(X) (current_sp_offset + n_spill + 1 + (X))
#define get_arg_sp_offset(X) (current_sp_offset + n_spill + maxoffset + 2 + current_func_retsize + 1 + (X))
#define get_retval_sp_offset() (current_sp_offset + n_spill + maxoffset + 2 + 1)
#define get_retaddr_sp_offset() (current_sp_offset + n_spill + maxoffset + 1)

#define not_implemented()                           \
	fprint(stderr, "; not implemented (%s)\n", opname(p->op)); \
	assert(0);

#define d_start() print("; %s(%s) {\n", \
  opname(p->op), (p->syms) ? ((p->syms[0]) ? (p->syms[0]->x.name ? p->syms[0]->x.name : "") : ("")) : "");
#define d_end() print("; } %s(%s)\n;\n", \
  opname(p->op), (p->syms) ? ((p->syms[0]) ? (p->syms[0]->x.name ? p->syms[0]->x.name : "") : ("")) : "");
