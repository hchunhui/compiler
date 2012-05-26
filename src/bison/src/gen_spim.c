#include <stdio.h>
#include "ast_node.h"
#include "sym_tab.h"
#include "c0.tab.h"
#include "gen.h"
#include "list.h"

/* forward decl */
static int gen_leaf(struct ast_node *leaf, int ri);
static void gen_call(struct ast_node *node);
static int gen_exp(struct ast_node *node, int ri);
static void gen_cond(struct ast_node *node);
static void gen_assign(struct ast_node *node);
static void gen_if(struct ast_node *node);
static void gen_while(struct ast_node *node);
static void gen_stmt(struct ast_node *node);
static void gen_block(struct ast_node *block);
static int gen_code(struct sym_tab *ptab);


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

static int lcount;
static FILE *fp;

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
		if(r->sym->type == SYM_VAR && r->dirty)
		{
			if(r->sym->tab->uplink)
				gen_ls("sw", r - reg, -(r->sym->svar.offset+1)*4, _S8);
			else
				gen_ls("sw", r - reg, (r->sym->svar.offset)*4, _GP);
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
	if(var->type != SYM_VAR || !var->gen_data)
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
			gen_ls("lw", r - reg, -(r->sym->svar.offset+1)*4, _S8);
		else
			gen_ls("lw", r - reg, (r->sym->svar.offset)*4, _GP);
		gen_nop();
		r->loaded = 1;
	}
}

static int get_reg(struct sym_entry *var)
{
	struct reg_struct *r;
	if(var->type != SYM_VAR)
	{
		fprintf(stderr, "get_reg\n");
		exit(1);
	}
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

static int gen_leaf(struct ast_node *node, int ri)
{
	int r;
	struct sym_entry *entry;
	switch(node->type)
	{
	case NT_NUMBER:
		if(node->ival == 0)
			return _ZERO;
		gen_li(ri, node->ival);
		return ri;
	case NT_IDENT:
		entry = node->pval;
		switch(entry->type)
		{
		case SYM_CONST:
			if(entry->sconst.value == 0)
				return _ZERO;
			gen_li(ri, entry->sconst.value);
			return ri;
		case SYM_VAR:
			r = get_reg(entry);
			load(entry);
			return r;
		case SYM_FUNC:
			printf("func error\n");
			exit(1);
			break;
		}
		break;
	default:
		printf("gen_leaf error\n");
		exit(1);
	}
	return 0;
}

static void gen_call(struct ast_node *node)
{
	struct ast_node *p;
	struct sym_entry *e;
	p = get_child(node);
	e = p->pval;
	if(e->type != SYM_FUNC) {
		printf("func call error.\n");
		exit(1);
	}
	reg_wb_all();
	gen_j("jal", e->name);
	gen_nop(); //分支延迟槽
}

static int gen_exp(struct ast_node *node, int ri)
{
	struct ast_node *l, *r;
	char *func;
	int rl, rr;
	switch(node->id)
	{
		//case UMINUS: func = "subu";break;
	case '+': func = "addu";break;
	case '-': func = "subu";break;
	case '*': func = "mul";break;
	case '/': func = "div";break;
	case 'N':
	case 'I': return gen_leaf(get_child(node), ri);
	default: printf("gen_exp error\n");exit(1);
	}
	//if(node->id == UMINUS)
	//gen_r(func, ri, _ZERO, gen_exp(get_child(node), _V1));
	//else
	{
		get_lr_child(node, &l, &r);
		rl = gen_exp(l, _V1);
		rr = gen_exp(r, _T9);
		if(node->id == '/')
		{
			gen_r(func, _ZERO, rl, rr);
			fprintf(fp, "\tmflo $%s\n", reg_name[ri]);
			reg[ri].dirty = 1;
		}
		else gen_r(func, ri, rl, rr);
		
	}
	return ri;
}

static void gen_assign(struct ast_node *node)
{
	struct ast_node *id, *exp;
	struct sym_entry *e;
	int ri, rj;
	int lev;

	get_lr_child(node, &id, &exp);
	e = id->pval;
	if(e->type != SYM_VAR)
	{
		exit(2);
	}
	ri = get_reg(e);
	rj = gen_exp(exp, ri);
	if(ri != rj)
		gen_r("addu", ri, _ZERO, rj);
}

static void gen_if(struct ast_node *node)
{
	struct ast_node *cond, *stmt, *l, *r;
	int rl, rr;
	struct sym_entry *e;
	char label[16];
	sprintf(label, "_L%d", lcount++);
	
	get_lr_child(node, &cond, &stmt);

	reg_wb_all();
	
	if(cond->id == '%')
	{
		gen_i("andi", _V0, gen_exp(get_child(cond), _V0), 1);
		gen_br("beq", _V0, _ZERO, label);
	}
	else
	{
		get_lr_child(cond, &l, &r);
		rl = gen_exp(l, _V0);
		rr = gen_exp(r, _V1);
		switch(cond->id)
		{
		case EQ_OP:
			gen_br("bne", rl, rr, label);
			break;
		case NE_OP:
			gen_br("beq", rl, rr, label);
			break;
		case '<':
			gen_r("slt", _V0, rl, rr);
			gen_br("beq", _V0, _ZERO, label);
			break;
		case GE_OP:
			gen_r("slt", _V0, rl, rr);
			gen_br("bne", _V0, _ZERO, label);
			break;
		case '>':
			gen_r("slt", _V0, rr, rl);
			gen_br("beq", _V0, _ZERO, label);
			break;
		case LE_OP:
			gen_r("slt", _V0, rr, rl);
			gen_br("bne", _V0, _ZERO, label);
			break;
		default: return;
		}
	}
	gen_nop(); //分支延迟槽
	gen_stmt(stmt);
	reg_wb_all();
	fprintf(fp, "%s:\n", label);
}

static void gen_while(struct ast_node *node)
{
	struct ast_node *cond, *stmt, *l, *r;
	int rl, rr;
	struct sym_entry *e;
	char lbeg[16];
	char lab[16];
	sprintf(lbeg, "_L%d", lcount++);
	sprintf(lab , "_L%d", lcount++);
	
	reg_wb_all();
	fprintf(fp, "%s:\n", lbeg);
	get_lr_child(node, &cond, &stmt);
	if(cond->id == '%')
	{
		gen_i("andi", _V0, gen_exp(get_child(cond), _V0), 1);
		gen_br("beq", _V0, _ZERO, lab);
	}
	else
	{
		
		get_lr_child(cond, &l, &r);
		rl = gen_exp(l, _V0);
		rr = gen_exp(r, _V1);
		switch(cond->id)
		{
		case EQ_OP:
			gen_br("bne", rl, rr, lab);
			break;
		case NE_OP:
			gen_br("beq", rl, rr, lab);
			break;
		case '<':
			gen_r("slt", _V0, rl, rr);
			gen_br("beq", _V0, _ZERO, lab);
			break;
		case GE_OP:
			gen_r("slt", _V0, rl, rr);
			gen_br("bne", _V0, _ZERO, lab);
			break;
		case '>':
			gen_r("slt", _V0, rr, rl);
			gen_br("beq", _V0, _ZERO, lab);
			break;
		case LE_OP:
			gen_r("slt", _V0, rr, rl);
			gen_br("bne", _V0, _ZERO, lab);
			break;
		default: return;
		}
	}
	gen_nop();  //分支延迟槽
	reg_wb_all();
	gen_stmt(stmt);
	reg_wb_all();
	gen_j("j", lbeg);
	gen_nop(); //分支延迟槽
	fprintf(fp, "%s:\n", lab);
}

static void gen_stmt(struct ast_node *node)
{
	switch(node->type)
	{
	case NT_CALL:	
		gen_call(node);
		break;
	case NT_EXP:
		gen_exp(node, _V0);
		break;
	case NT_ASSIGN:
		gen_assign(node);
		break;
	case NT_IF:
		gen_if(node);
		break;
	case NT_WHILE:
		gen_while(node);
		break;
	case NT_BLOCK:
		gen_block(node);
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


static int gen_code(struct sym_tab *ptab)
{
	int offset;
	int cx_func;
	int cx1 = 0;
	struct sym_entry *entry;
	struct sym_entry *e;
	offset = 0;
	
	fprintf(fp, "\t.data\n\t.align 2\n");
	list_for_each_entry(entry, &ptab->order, order)
		if(entry->type == SYM_VAR)
		{
			entry->svar.offset = offset++;
			fprintf(fp, "%s:\n\t.word\t%d\n",
				entry->name,
				0);
		}
	
	fprintf(fp, "\t.text\n\t.align 2\n\t.globl main\n");
	list_for_each_entry(entry, &ptab->order, order)
		if(entry->type == SYM_FUNC)
		{
			cx_func = cx;
			printf("function %s is at %d\n",
			       entry->name,
			       cx_func);
			
			entry->sfunc.addr = cx_func;
			fprintf(fp, "%s:\n", entry->name);
			offset = 0;
			list_for_each_entry(
				e,
				&entry->sfunc.sym->order,
				order)
				if(e->type == SYM_VAR)
					e->svar.offset = offset++;
			gen_ls("sw", _RA, -4, _SP);
			gen_ls("sw", _S8, -8, _SP);
			gen_i("addiu", _S8, _SP, -8);
			gen_i("addiu", _SP, _SP, (-offset-2)*4);
			if(strcmp(entry->name, "main") == 0)
				gen_i2("lui", _GP, 0x1000);
			gen_block(entry->sfunc.stmts);
			reg_wb_all();
			gen_ls("lw", _RA, 4, _S8);
			gen_ls("lw", _S8, 0, _S8);  //加载延迟槽
			gen_jr("jr", _RA);
			gen_i("addiu", _SP, _SP, (offset+2)*4); //分支延迟槽
		}
	return cx1;
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
	lcount = 0;
	reg_init();
	gen_code(ptab);
	fclose(fp);
}

struct gen_info gen_spim = {
	.name = "spim",
	.gen_code = gen_code_all,
	.info = "C0到SPIM"
};
