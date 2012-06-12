#include <stdio.h>
#include "ast_node.h"
#include "sym_tab.h"
#include "c0.tab.h"
#include "gen.h"
#include "list.h"
#include "error.h"

/* forward decl */
static int gen_leaf(struct ast_node *leaf, int ri);
static void gen_call(struct ast_node *node);
static int gen_exp(struct ast_node *node, int ri);
static int gen_assign(struct ast_node *node);
static void gen_if(struct ast_node *node);
static void gen_for(struct ast_node *node);
static void gen_stmt(struct ast_node *node);
static void gen_block(struct ast_node *block);
static void gen_code(struct sym_tab *ptab);

extern char *strs[80];
extern int strs_count;

static FILE *fp;

static int while_next, while_jmp;
static int func_ret;
static int str_table;
static int get_new_label()
{
	static int n;
	return n++;
}

static void put_label(int e)
{
	fprintf(fp, "_L%d:\n", e);
}

static char *use_label(int e)
{
	static char l[16];
	sprintf(l, "_L%d", e);
	return l;
}

const char *reg_name[32] = {
	"0",
	"at",
	"v0",
	"v1",
	"a0",
	"a1",
	"a2",
	"a3",
	"t0",
	"t1",
	"t2",
	"t3",
	"t4",
	"t5",
	"t6",
	"t7",
	"s0",
	"s1",
	"s2",
	"s3",
	"s4",
	"s5",
	"s6",
	"s7",
	"t8",
	"t9",
	"k0",
	"k1",
	"gp",
	"sp",
	"s8",
	"ra",
};

enum {
	_ZERO = 0,
	_AT,
	_V0,
	_V1,
	_A0,
	_A1,
	_A2,
	_A3,
	_T0,
	_T1,
	_T2,
	_T3,
	_T4,
	_T5,
	_T6,
	_T7,
	_S0,
	_S1,
	_S2,
	_S3,
	_S4,
	_S5,
	_S6,
	_S7,
	_T8,
	_T9,
	_K0,
	_K1,
	_GP,
	_SP,
	_S8,
	_RA,
};

struct reg_struct {
	int dirty;
	int loaded;
	struct sym_entry *sym;
	struct list_head list;
	struct list_head avail_list;
};

static struct reg_struct reg[32];

struct list_head list_reg;
struct list_head avail_reg;


static int cx;
static void gen_r(char *mnemonic, int rd, int rs, int rt)
{
	fprintf(fp, "\t%s\t$%s, $%s, $%s\n", mnemonic, reg_name[rd], reg_name[rs], reg_name[rt]);
	reg[rd].dirty = 1;
	cx++;
}

static void gen_i(char *mnemonic, int rt, int rs, short imm)
{
	fprintf(fp, "\t%s\t$%s, $%s, %hd\n", mnemonic, reg_name[rt], reg_name[rs], imm);
	reg[rt].dirty = 1;
	cx++;
}

static void gen_i2(char *mnemonic, int rt, short imm)
{
	fprintf(fp, "\t%s\t$%s, %hd\n", mnemonic, reg_name[rt], imm);
	reg[rt].dirty = 1;
	cx++;
}

static void gen_j(char *mnemonic, char *label)
{
	fprintf(fp, "\t%s\t%s\n", mnemonic, label);
	cx++;
}

static void gen_jr(char *mnemonic, int r)
{
	fprintf(fp, "\t%s\t$%s\n", mnemonic, reg_name[r]);
	cx++;
}

static void gen_br(char *mnemonic, int rt, int rs, char *label)
{
	fprintf(fp, "\t%s\t$%s, $%s, %s\n", mnemonic, reg_name[rt], reg_name[rs], label);
	cx++;
}

static void gen_ls(char *mnemonic, int rt, short i, int rs)
{
	fprintf(fp, "\t%s\t$%s, %hd($%s)\n", mnemonic, reg_name[rt], i, reg_name[rs]);
	cx++;
}

static void gen_li(int ri, unsigned int num)
{
	if(num > 0xffff)
	{
		gen_i2("lui", ri, num >> 16);
		if(num & 0xffff)
			gen_i("ori", ri, ri, num);
	}
	else
		gen_i("ori", ri, _ZERO, num);
}

static void gen_nop()
{
	gen_i("sll", _ZERO, _ZERO, 0);
}

static void reg_init()
{
	int i;

	for(i = 0; i < 32; i++)
	{
		reg[i].loaded = 0;
		reg[i].dirty = 0;
		INIT_LIST_HEAD(&reg[i].avail_list);
		INIT_LIST_HEAD(&reg[i].list);
		reg[i].sym = NULL;
	}
	
	INIT_LIST_HEAD(&avail_reg);
	list_add_tail(&reg[_T0].avail_list, &avail_reg);
	list_add_tail(&reg[_T1].avail_list, &avail_reg);
	list_add_tail(&reg[_T2].avail_list, &avail_reg);
	list_add_tail(&reg[_T3].avail_list, &avail_reg);
	list_add_tail(&reg[_T4].avail_list, &avail_reg);
	list_add_tail(&reg[_T5].avail_list, &avail_reg);
	list_add_tail(&reg[_T6].avail_list, &avail_reg);
	list_add_tail(&reg[_T7].avail_list, &avail_reg);
	list_add_tail(&reg[_T8].avail_list, &avail_reg);

	INIT_LIST_HEAD(&list_reg);
	list_add_tail(&reg[_T0].list, &list_reg);
	list_add_tail(&reg[_T1].list, &list_reg);
	list_add_tail(&reg[_T2].list, &list_reg);
	list_add_tail(&reg[_T3].list, &list_reg);
	list_add_tail(&reg[_T4].list, &list_reg);
	list_add_tail(&reg[_T5].list, &list_reg);
	list_add_tail(&reg[_T6].list, &list_reg);
	list_add_tail(&reg[_T7].list, &list_reg);
	list_add_tail(&reg[_T8].list, &list_reg);
}

static void reg_wb(struct reg_struct *r)
{
	if(r->sym)
	{
		if(r->sym->kind == SYM_VAR && r->dirty)
		{
			if(r->sym->tab->uplink)
				gen_ls("sw",
				       r - reg,
				       -(r->sym->svar.offset)*4, _S8);
			else
				gen_ls("sw",
				       r - reg,
				       (r->sym->svar.offset)*4,
				       _GP);
		}
		r->sym->gen_data = NULL;
		r->sym = NULL;
		printf("wb %s\n", reg_name[r - reg]);
		list_add_tail(&r->avail_list, &avail_reg);
		r->dirty = 0;
		r->loaded = 0;
	}
}

static void reg_wb_all()
{
	struct reg_struct *r;
	list_for_each_entry(r, &list_reg, list)
		reg_wb(r);
}

static void load(struct sym_entry *var)
{
	struct reg_struct *r;
	if(var->kind != SYM_VAR || !var->gen_data)
	{
		fprintf(stderr, "load\n");
		exit(1);
	}
	r = var->gen_data;
	if(r->dirty)
	{
		r->loaded = 1;
	}
	if(!r->loaded)
	{
		if(var->tab->uplink)
			gen_ls("lw",
			       r - reg,
			       -(r->sym->svar.offset)*4, _S8);
		else
			gen_ls("lw",
			       r - reg,
			       (r->sym->svar.offset)*4, _GP);
		gen_nop();
		r->loaded = 1;
	}
}

static int get_reg(struct sym_entry *var)
{
	struct reg_struct *r;
	if(var->gen_data)
	{
		r = var->gen_data;
		return r - reg;
	}
	
	if(!list_empty(&avail_reg))
	{
		r = list_entry(avail_reg.next, struct reg_struct, avail_list);
		list_del(avail_reg.next);
		r->dirty = 0;
	}
	else
	{
		/*list_for_each_entry(r, &list_reg, list)
		{
			if(!r->dirty)
				goto found;
				}*/
		list_for_each_entry(r, &list_reg, list)
		{
			reg_wb(r);
			list_del(&r->avail_list);
			goto found;
		}
	}
found:
	r->sym = var;
	r->loaded = 0;
	var->gen_data = r;
	return r - reg;
}

static int gen_call_parm(struct ast_node *list)
{
	int r;
	int i;
	struct ast_node *exp, *tmp;
	i = 0;
	if(list)
		list_for_each_entry_rev(exp, tmp, &list->chlds, sibling)
		{
			i++;
			r = gen_exp(exp, _V1);
			gen_ls("sw", r, -4*i, _SP);
		}
	gen_i("addiu", _SP, _SP, -4*i);
	return i;
}

static void gen_call(struct ast_node *node)
{
	int count;
	struct ast_node *l, *r;
	struct sym_entry *e;
	get_lr_child(node, &l, &r);
	e = l->pval;
	gen_ls("sw", _V1, -4, _SP);
	gen_ls("sw", _T9, -8, _SP);
	gen_i("addiu", _SP, _SP, -8);
	count = gen_call_parm(r);
	reg_wb_all();
	gen_j("jal", e->name);
	gen_nop(); //分支延迟槽
	gen_i("addiu", _SP, _SP, count*4);
	gen_ls("lw", _T9, 0, _SP);
	gen_i("addiu", _SP, _SP, 8);
	gen_ls("lw", _V1, -4, _SP);
	gen_nop();
}

static int gen_assign(struct ast_node *node)
{
	struct ast_node *id, *exp, *l, *r;
	struct sym_entry *e;
	int ri, rj;
	int lev;

	get_lr_child(node, &id, &exp);
	rj = gen_exp(exp, _V0);
	if(id->id == 'I')
	{
		e = id->pval;
		ri = get_reg(e);
		if(ri != rj)
			gen_r("addu", ri, _ZERO, rj);
		return ri;
	}
	else
	{
		get_lr_child(id, &l, &r);
		ri = gen_exp(r, _AT);
		if(ri != _AT)
			gen_r("addu", _AT, _ZERO, ri);
		gen_i("sll", _AT, _AT, 2);
		e = l->pval;
		if(e->tab->uplink)
		{
			gen_r("addu", _AT, _AT, _S8);
			gen_ls("sw",
			       rj,
			       -(e->svar.offset)*4, _AT);
		}
		else
		{
			gen_r("addu", _AT, _AT, _GP);
			gen_ls("sw",
			       rj,
			       (e->svar.offset)*4, _AT);
		}
		return rj;
	}
}

static int gen_exp(struct ast_node *node, int ri)
{
	struct ast_node *l, *r;
	struct sym_entry *e;
	int lab;
	char *func;
	int rl, rr;
	lab = get_new_label();
	if(!node)return _ZERO;
	if(node->type == NT_NUL) return _ZERO;
	switch(node->id)
	{
	case '+': func = "addu";break;
	case '-': func = "subu";break;
	case '*': func = "mul";break;
	case '/': func = "div";break;
	case AND:
		get_lr_child(node, &l, &r);
		rl = gen_exp(l, _V1);
		gen_br("beq", rl, _ZERO, use_label(lab));
		gen_r("addu", ri, _ZERO, _ZERO);
		gen_nop();
		rl = gen_exp(r, _V1);
		gen_br("beq", rl, _ZERO, use_label(lab));
		gen_nop();
		gen_i("xori", ri, ri, 1);
		put_label(lab);
		return ri;
	case OR:
		get_lr_child(node, &l, &r);
		rl = gen_exp(l, _V1);
		gen_br("bne", rl, _ZERO, use_label(lab));
		gen_i("addiu", ri, _ZERO, 1);
		gen_nop();
		rl = gen_exp(r, _V1);
		gen_br("bne", rl, _ZERO, use_label(lab));
		gen_nop();
		gen_i("xori", ri, ri, 1);
		put_label(lab);
		return ri;
	case NOT:
		get_lr_child(node, &l, &r);
		rl = gen_exp(l, ri);
		gen_br("bne", rl, _ZERO, use_label(lab));
		gen_i("addu", ri, _ZERO, _ZERO);
		gen_i("xori", ri, ri, 1);
		put_label(lab);
		return ri;
	case 'i':
	case 'b':
		if(node->ival == 0)
			return _ZERO;
		gen_li(ri, node->ival);
		return ri;
	case 'f':
		new_error(1,0,0,"不支持浮点数\n");
		return 0;
	case 'I':
		e = node->pval;
		rl = get_reg(e);
		load(e);
		return rl;
	case 'A':
		get_lr_child(node, &l, &r);
		rr = gen_exp(r, _AT);
		if(rr != _AT)
			gen_r("addu", _AT, _ZERO, rr);
		gen_i("sll", _AT, _AT, 2);
		e = l->pval;
		if(e->tab->uplink)
		{
			gen_r("addu", _AT, _AT, _S8);
			gen_ls("lw",
			       ri,
			       -(e->svar.offset)*4, _AT);
		}
		else
		{
			gen_r("addu", _AT, _AT, _GP);
			gen_ls("lw",
			       ri,
			       (e->svar.offset)*4, _AT);
		}
		gen_nop();
		return ri;
	case '=':
		return gen_assign(node);
	case 'F':
		gen_call(node);
		gen_r("addu", ri, _ZERO, _V0);
		return ri;
	}
	get_lr_child(node, &l, &r);
	if(r)
	{
		if(node->id == '*')
		{
			if(l->id == 'i' && l->ival == 2)
			{
				rr = gen_exp(r, _T9);
				gen_i("sll", ri, rr, 1);
				return ri;
			} else if(r->id == 'i' && r->ival == 2)
			{
				rl = gen_exp(l, _V1);
				gen_i("sll", ri, rl, 1);
				return ri;
			}
		}
		if(node->id == '/')
		{
			if(r->id == 'i' && r->ival == 2)
			{
				rl = gen_exp(l, _V1);
				gen_i("sra", ri, rl, 1);
				return ri;
			}
		}
		rl = gen_exp(l, _V1);
		rr = gen_exp(r, _T9);
		switch(node->id)
		{
		case EQ_OP:
			gen_br("bne", rl, rr, use_label(lab));
			gen_r("addu", ri, _ZERO, _ZERO);
			gen_i("addiu", ri, ri, 1);
			put_label(lab);
			break;
		case NE_OP:
			gen_br("beq", rl, rr, use_label(lab));
			gen_r("addu", ri, _ZERO, _ZERO);
			gen_i("addiu", ri, ri, 1);
			put_label(lab);
			break;
		case '<':
			gen_r("slt", ri, rl, rr);
			break;
		case GE_OP:
			gen_r("slt", ri, rl, rr);
			gen_i("xori", ri, ri, 1);
			break;
		case '>':
			gen_r("slt", ri, rr, rl);
			break;
		case LE_OP:
			gen_r("slt", ri, rr, rl);
			gen_i("xori", ri, ri, 1);
			break;
		case '/':
			gen_r("div", _ZERO, rl, rr);
			fprintf(fp, "\tmflo $%s\n", reg_name[ri]);
			reg[ri].dirty = 1;
			break;
		case '%':
			gen_r("div", _ZERO, rl, rr);
			fprintf(fp, "\tmfhi $%s\n", reg_name[ri]);
			reg[ri].dirty = 1;
			break;
		default:
			gen_r(func, ri, rl, rr);
			break;
		}
	}
	else
	{
		rl = gen_exp(l, _V1);
		if(func == "subu")
			gen_r("subu", ri, _ZERO, rl);
	}
	return ri;
}

static void gen_if(struct ast_node *node)
{
	int i;
	struct ast_node *p;
	int l_else, l_next;
	int rl;

	l_else = get_new_label();
	l_next = get_new_label();
	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		switch(i)
		{
		case 0: /* cond */
			rl = gen_exp(p, _V0);
			reg_wb_all();
			gen_br("beq", rl, _ZERO, use_label(l_else));
			gen_nop(); /* 分支延迟槽 */
			break;
		case 1: /* then */
			gen_stmt(p);
			reg_wb_all();
			gen_br("beq", _ZERO, _ZERO, use_label(l_next));
			gen_nop(); /* 分支延迟槽 */
			break;
		case 2: /* else */
			put_label(l_else);
			gen_stmt(p);
			reg_wb_all();
			break;
		}
		i++;
	}
	if(i == 2)
		put_label(l_else);
	put_label(l_next);
}

static void gen_for(struct ast_node *node)
{
	int i;
	struct ast_node *p;
	int l_head, l_next, l_jmp, save_next, save_jmp;
	int rl;
	struct sym_entry *e;

	l_head = get_new_label();
	l_next = get_new_label();
	l_jmp = get_new_label();
	save_jmp = while_jmp;
	while_jmp = l_jmp;
	save_next = while_next;
	while_next = l_next;
	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		switch(i)
		{
		case 0: /* init */
			gen_stmt(p);
			reg_wb_all();
			break;
		case 1: /* cond */
			put_label(l_head);
			rl = gen_exp(p, _V0);
			reg_wb_all();
			gen_br("beq", rl, _ZERO, use_label(l_next));
			gen_nop(); /* 分支延迟槽 */
			break;
		case 2: /* stmt */
			gen_stmt(p);
			reg_wb_all();
			break;
		case 3: /* inc */
			put_label(l_jmp);
			gen_stmt(p);			
			reg_wb_all();
			gen_br("beq", _ZERO, _ZERO, use_label(l_head));
			gen_nop(); /* 分支延迟槽 */
			put_label(l_next);
			break;
		}
		i++;
	}
	
	while_next = save_next;
	while_jmp = save_jmp;
}

static void gen_read(struct ast_node *list)
{
	struct ast_node *exp, *l, *r;
	struct sym_entry *e;
	int ri;
	if(list)
		list_for_each_entry(exp, &list->chlds, sibling)
		{
			gen_i("addiu", _V0, _ZERO, 5);
			fprintf(fp, "\tsyscall\n");

			if(exp->id == 'I')
			{
				e = exp->pval;
				ri = get_reg(e);
				gen_r("addu", ri, _ZERO, _V0);
			}
			else if(exp->id == 'A')
			{
				get_lr_child(exp, &l, &r);
				ri = gen_exp(r, _AT);
				if(ri != _AT)
					gen_r("addu", _AT, _ZERO, ri);
				gen_i("sll", _AT, _AT, 2);
				e = l->pval;
				if(e->tab->uplink)
				{
					gen_r("addu", _AT, _AT, _S8);
					gen_ls("sw",
					       _V0,
					       -(e->svar.offset)*4, _AT);
				}
				else
				{
					gen_r("addu", _AT, _AT, _GP);
					gen_ls("sw",
					       _V0,
					       (e->svar.offset)*4, _AT);
				}
			}
		}
	reg_wb_all();
}

static void gen_stmt(struct ast_node *node)
{
	int rl;
	switch(node->type)
	{
	case NT_EXP:
		gen_exp(node, _V0);
		break;
	case NT_IF:
		gen_if(node);
		break;
	case NT_FOR:
		gen_for(node);
		break;
	case NT_NUL:
		break;
	case NT_BLOCK:
		gen_block(node);
		break;
	case NT_BREAK:
		reg_wb_all();
		gen_br("beq", _ZERO, _ZERO, use_label(while_next));
		gen_nop();
		break;
	case NT_CONTINUE:
		reg_wb_all();
		gen_br("beq", _ZERO, _ZERO, use_label(while_jmp));
		gen_nop();
		break;
	case NT_RETURN:
		if(node->id)
		{
			rl = gen_exp(get_child(node), _V0);
			if(rl != _V0)
				gen_r("addu", _V0, _ZERO, rl);
		}
		reg_wb_all();
		gen_br("beq", _ZERO, _ZERO, use_label(func_ret));
		gen_r("addu", _V0, _ZERO, rl);
		break;
	case NT_WRITEE:
		rl = gen_exp(get_child(node), _A0);
		if(rl != _A0)
			gen_r("addu", _A0, _ZERO, rl);
		gen_i("addiu", _V0, _ZERO, 1);
		fprintf(fp, "\tsyscall\n");
		break;
	case NT_READ:
		gen_read(get_child(node));
		break;
	case NT_WRITES:
		gen_ls("lw", _A0, 4*(node->id+str_table), _GP);
		gen_i("addiu", _V0, _ZERO, 4);
		fprintf(fp, "\tsyscall\n");
		break;
	default:
		printf("unkwon stmt\n");
		exit(1);
	}
}

static void gen_block(struct ast_node *block)
{
	struct ast_node *stmt;
	struct sym_entry *entry;
	list_for_each_entry(stmt, &block->chlds, sibling)
	{
		gen_stmt(stmt);
	}
}

static void gen_initexp(struct sym_entry *e)
{
	struct ast_node *node, *p;
	int i;
	int r, base;
	int sign;
	node = e->svar.iexp;
	sign = e->tab->uplink ? -1 : 1;
	if(e->tab->uplink)
		base = _S8;
	else
		base = _GP;
	if(node->type == NT_EXP)
	{
		r = gen_exp(node, _V0);
		gen_ls("sw", r, sign*e->svar.offset*4, base);
	}
	else
	{
		i = 0;
		list_for_each_entry(p, &node->chlds, sibling)
		{
			r = gen_exp(p, _V0);
			gen_ls("sw", r, (i+sign*e->svar.offset)*4, base);
			i++;
		}
	}
}

static void gen_code(struct sym_tab *ptab)
{
	int i;
	int offset, parm_offset, len;
	int cx_func;
	struct sym_entry *entry;
	struct sym_entry *e;
	offset = 0;
	parm_offset = -2;
	
	fprintf(fp, "\t.data\n\t.align 2\n");
	list_for_each_entry(entry, &ptab->order, order)
		if(entry->kind == SYM_VAR)
		{
			if(type_is_float(entry->type))
				new_error(1,0,0,"暂不支持浮点数\n");
			len = type_len(entry->type);
			entry->svar.offset = offset;
			offset += len;
			fprintf(fp, "%s:\n", entry->name);
			fprintf(fp, "\t.space\t%d\n", len*4);
		}
	str_table = offset;
	fprintf(fp,"\nstrs:\n");
	for(i = 0; i < strs_count; i++)
		fprintf(fp, "\t.word\t_S%d\n", i);
	for(i = 0; i < strs_count; i++)
		fprintf(fp, "_S%d:\t.asciiz \"%s\"\n", i, strs[i]);
	fprintf(fp, "\n");
	fprintf(fp, "\t.text\n\t.align 2\n\t.globl main\n");
	list_for_each_entry(entry, &ptab->order, order)
		if(entry->kind == SYM_FUNC)
		{
			cx_func = cx;
			func_ret = get_new_label();
			printf("function %s is at %d\n",
			       entry->name,
			       cx_func);
			
			entry->sfunc.addr = cx_func;
			fprintf(fp, "%s:\n", entry->name);
			offset = 0;
			parm_offset = -2;
			list_for_each_entry(
				e,
				&entry->sfunc.sym->order,
				order)
				if(e->kind == SYM_VAR)
				{
					len = type_len(e->type);
					if(e->svar.is_param)
					{
						e->svar.offset = parm_offset;
						parm_offset -= len;
					}
					else
					{
						offset += len;
						e->svar.offset = offset;
					}
				}
			gen_ls("sw", _RA, -4, _SP);
			gen_ls("sw", _S8, -8, _SP);
			gen_i("addiu", _S8, _SP, -8);
			gen_i("addiu", _SP, _SP, (-offset-2)*4);
			if(strcmp(entry->name, "main") == 0)
			{
				gen_i2("lui", _GP, 0x1000);
				list_for_each_entry(e, &ptab->order, order)
					if(e->kind == SYM_VAR)
						if(e->svar.iexp)
							gen_initexp(e);
			}
			list_for_each_entry(
				e,
				&entry->sfunc.sym->order,
				order)
				if(e->kind == SYM_VAR)
					if(e->svar.iexp)
						gen_initexp(e);
			gen_block(entry->sfunc.stmts);
			reg_wb_all();
			put_label(func_ret);
			gen_ls("lw", _RA, 4, _S8);
			gen_ls("lw", _S8, 0, _S8);  //加载延迟槽
			gen_jr("jr", _RA);
			gen_i("addiu", _SP, _SP, (offset+2)*4); //分支延迟槽
		}
}


static void gen_code_all(struct sym_tab *ptab, char *name)
{
	fp = fopen(name, "w");
	if(!fp)
	{
		fprintf(stderr,
			"Warning: Can't open %s .\n",
			name);
		return;
	}

	cx = 0;
	reg_init();
	gen_code(ptab);
	fclose(fp);
}

struct gen_info gen_spim = {
	.name = "spim",
	.gen_code = gen_code_all,
	.info = "C0到SPIM"
};
