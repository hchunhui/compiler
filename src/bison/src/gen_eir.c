#include <stdio.h>
#include <string.h>
#include "ast_node.h"
#include "sym_tab.h"
#include "c0.tab.h"
#include "gen.h"
#include "error.h"
#include "eir_inst.h"

#define CXMAX 5000
static instruction code[CXMAX];
static struct sym_entry *refill[CXMAX];

static int curr_parm_len;
static int cx;


extern char *strs[80];
extern int strs_count;

static struct sym_tab *global_tab;
static struct sym_entry *while_next, *while_jmp;
static struct sym_entry *get_new_label()
{
	static int n;
	char l[16];
	sprintf(l, "%d", n++);
	return symtab_enter(global_tab,
			    l,
			    get_type(TYPE_LABEL, 0, NULL, NULL));
}

static void put_label(struct sym_entry *e)
{
	e->gen_data = (void *)(long)cx;
}

static void use_label(struct sym_entry *e)
{
	refill[cx] = e;
}

const char *fctt[] =
{
	"lit", "opr", "lod", "sto", "cal", "Int", "jmp", "jpc", "lar", "sar", "jpe" ,"init"       // functions
};

const char *opcc[] =
{
	"ret", "neg", "add", "minuss", "mult", "divv", "mod", "andand", "oror", "eq", "neq", "lt", "lte", "gt", "gte", "readd", "writee" , "writes", "notnot"
};

void listcode()
{
	int i;
	for(i = 0; i < cx; i++)
	{
		fprintf(stderr,
			"%d\t%s\t%d",
			i,
			fctt[code[i].f],
			code[i].l);
		switch(code[i].t)
		{
		case _INT:
			fprintf(stderr, "\t%d\tint", code[i].v.i);
			if(refill[i])
				fprintf(stderr, "\tsym: %s%s\n",
					type_is_label(refill[i]->type)?".L":"",
					refill[i]->name);
			else
				fprintf(stderr, "\n");
			break;
		case _FLOAT:
			fprintf(stderr, "\t%.3f\tfloat\n", code[i].v.d);
			break;
		case _BOOL:
			fprintf(stderr, "\t%d\tbool\n", code[i].v.b);
			break;
		case _OPC:
			fprintf(stderr, "\t%s\topc\n", opcc[code[i].v.op]);
			break;
		default:
			break;
		}
	}
}

static void geni(enum fct f, int l, int i)
{
	if(cx >= CXMAX) {
		fprintf(stderr, "program too long\n");
		exit(1);
	}
	code[cx].f = f;
	code[cx].l = l;
	code[cx].v.i = i;
	code[cx].t = _INT;
	cx++;
}

static void genb(enum fct f, int l, int b)
{
	if(cx >= CXMAX) {
		fprintf(stderr, "program too long\n");
		exit(1);
	}
	code[cx].f = f;
	code[cx].l = l;
	code[cx].v.b = b;
	code[cx].t = _BOOL;
	cx++;
}

static void genf(enum fct f, int l, double d)
{
	if(cx >= CXMAX) {
		fprintf(stderr, "program too long\n");
		exit(1);
	}
	code[cx].f = f;
	code[cx].l = l;
	code[cx].v.d = d;
	code[cx].t = _FLOAT;
	cx++;
}

static void geno(enum fct f, int l, int o)
{
	if(cx >= CXMAX) {
		fprintf(stderr, "program too long\n");
		exit(1);
	}
	code[cx].f = f;
	code[cx].l = l;
	code[cx].v.op = o;
	code[cx].t = _OPC;
	cx++;
}

/* forward decl */
static void gen_leaf(struct ast_node *leaf);
static void gen_exp(struct ast_node *node, int);
static void gen_if(struct ast_node *node);
static void gen_while(struct ast_node *node);
static void gen_stmt(struct ast_node *node);
static void gen_block(struct ast_node *block);
static int  gen_code(struct sym_tab *ptab);

static FILE *fp;

static int type_len(struct type *t)
{
	int i;
	int lim;
	if(type_is_var(t) && !type_is_array(t))
		return 1;
	if(!type_is_var(t))
		return 0;
	return t->n * type_len(t->t2);
}

static void gen_lval(struct ast_node *node, int op)
{
	struct sym_entry *e;
	
	while(node->id != 'I')
		node = get_child(node);
	e = node->pval;
	if(e->tab->uplink)
		geni(op, 0, e->svar.offset);
	else
		geni(op, 1, e->svar.offset);
}

static void gen_call(struct ast_node *node)
{
	struct sym_entry *e;
	while(node->id != 'I')
		node = get_child(node);
	e = node->pval;
	refill[cx] = e;
	geni(cal, 1, 0);
}

static void gen_explist(struct ast_node *list)
{
	struct ast_node *exp, *tmp;
	if(list)
		list_for_each_entry_rev(exp, tmp, &list->chlds, sibling)
		{
			gen_exp(exp, 1);
		}
}

static void gen_array_num(struct ast_node *node)
{
	int i;
	int lim;
	struct ast_node *p, *l, *r;
	struct type *tlt, *trt;
	get_lr_child(node, &l, &r);
	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		if(i == 0)
		{
			tlt = ((struct sym_entry *)l->pval)->type;
			lim = 1;
		}
		else if(i == 1)
		{
			gen_exp(p, 1);
			lim *= tlt->n;
			tlt = tlt->t2;
		}
		else
		{
			geni(lit, 0, tlt->n);
			geno(opr, 0, mult);
			gen_exp(p, 1);
			geno(opr, 0, add);
			lim *= tlt->n;
			tlt = tlt->t2;
		}
		i++;
	}
	geni(lit, 0, lim);
}

static void gen_exp(struct ast_node *node, int need_reload)
{
	int i;
	int cj1, cj2;
	struct ast_node *p, *l, *r;
	struct sym_entry *e;
	int func;
	switch(node->id)
	{
	case '+':func = add;    break;
	case '-':func = minuss; break;
	case '*':func = mult;   break;
	case '/':func = divv;   break;
	case EQ_OP: func = eq;  break;
	case NE_OP: func = neq; break;
	case '<': func = lt;    break;
	case GE_OP: func = gte; break;
	case '>': func = gt;    break;
	case LE_OP: func = lte; break;
	case '%': func = mod;   break;
	case AND:
		get_lr_child(node, &l, &r);
		geni(lit, 0, 0);
		gen_exp(l, 1);
		cj1 = cx;
		geni(jpc, 0, 0);
		gen_exp(r, 1);
		cj2 = cx;
		geni(jpc, 0, 0);
		geno(opr, 0, notnot);
		code[cj1].v.i = cx;
		code[cj2].v.i = cx;
		goto clean_stack;
	case OR:
		get_lr_child(node, &l, &r);
		geni(lit, 0, 1);
		gen_exp(l, 1);
		geno(opr, 0, notnot);
		cj1 = cx;
		geni(jpc, 0, 0);
		gen_exp(r, 1);
		geno(opr, 0, notnot);
		cj2 = cx;
		geni(jpc, 0, 0);
		geno(opr, 0, notnot);
		code[cj1].v.i = cx;
		code[cj2].v.i = cx;
		goto clean_stack;
	case NOT: func = notnot;break;
	case '|':
	case '&':
	case '~':
		new_error(1,
			  node->first_line,
			  node->first_column,
			  "bitwise operation"
			  "is not supported by EIR machine.\n");
		return;
	case '=':
		get_lr_child(node, &l, &r);
		gen_exp(r, 1);
		if(l->id == 'A')
		{
			gen_array_num(l);
			gen_lval(l, sar);
			if(need_reload)
			{
				gen_array_num(l);
				gen_lval(l, lar);
			}
		}
		else
		{
			gen_lval(l, sto);
			if(need_reload)
			{
				gen_lval(l, lod);
			}
		}
		return;
	case 'F':
		get_lr_child(node, &l, &r);
		gen_explist(r);
		gen_call(l);
		goto clean_stack;
	case 'f':
		if(need_reload)
			genf(lit, 0, node->fval);
		return;
	case 'i':
		if(need_reload)
			geni(lit, 0, node->ival);
		return;
	case 'b':
		if(need_reload)
			genb(lit, 0, node->ival);
		return;
	case 'I':
		if(need_reload)
			gen_lval(node, lod);
		return;
	case 'A':
		gen_array_num(node);
		gen_lval(node, lar);
		goto clean_stack;
	default: new_error(1,
			   node->first_line,
			   node->first_column,
			   "gen_exp error\n");
	}
	get_lr_child(node, &l, &r);
	if(r)
	{
		gen_exp(l, 1);
		gen_exp(r, 1);
		geno(opr, 0, func);
	}
	else
	{
		if(func == minuss)
			func = neg;
		else if(func == add)
			return;
		gen_exp(l, 1);
		geno(opr, 0, func);
	}
clean_stack:
	if(!need_reload)
		geni(Int, 0, -1);
}

static void gen_if(struct ast_node *node)
{
	struct sym_entry *l_else, *l_next;
	struct ast_node *p;
	int i;
	l_else = get_new_label();
	l_next = get_new_label();
	
	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		switch(i)
		{
		case 0:
			gen_exp(p, 1);
			use_label(l_else);
			geni(jpc, 0, 0);
			break;
		case 2:
			put_label(l_else);
			gen_stmt(p);
			break;
		case 1:
			gen_stmt(p);
			use_label(l_next);
			geni(jmp, 0, 0);
			break;
		default: new_error(1,
				   node->first_line,
				   node->first_column,
				   "bad if stmt\n");
		}
		i++;
	}
	if(i == 2)
		put_label(l_else);
	put_label(l_next);
}

static void gen_while(struct ast_node *node)
{
	struct ast_node *exp, *stmt;
	struct sym_entry *e;
	struct sym_entry *l_head, *l_next, *l_jmp, *save_next, *save_jmp;
	l_head = get_new_label();
	l_next = get_new_label();
	l_jmp = get_new_label();
	save_jmp = while_jmp;
	while_jmp = l_jmp;
	save_next = while_next;
	while_next = l_next;
	
	get_lr_child(node, &exp, &stmt);
	put_label(l_head);
	gen_exp(exp, 1);
	
	use_label(l_next);
	geni(jpc, 0, 0);

	put_label(l_jmp);
	gen_stmt(stmt);
	use_label(l_head);
	geni(jmp, 0, 0);

	put_label(l_next);

	while_next = save_next;
	while_jmp = save_jmp;
}

static void gen_read(struct ast_node *list)
{
	struct ast_node *asgn;
	struct ast_node *exp;
	if(list)
		list_for_each_entry(exp, &list->chlds, sibling)
		{
			geno(opr, _FLOAT, readd);
			if(exp->id == 'A')
			{
				gen_array_num(exp);
				gen_lval(exp, sar);
			}
			else
				gen_lval(exp, sto);
		}
}

static void gen_stmt(struct ast_node *node)
{
	switch(node->type)
	{
	case NT_EXP:
		gen_exp(node, 0);
		break;
	case NT_IF:
		gen_if(node);
		break;
	case NT_WHILE:
		gen_while(node);
		break;
	case NT_NUL:
		break;
	case NT_BLOCK:
		gen_block(node);
		break;
	case NT_BREAK:
		use_label(while_next);
		geni(jmp, 0, 0);
		break;
	case NT_CONTINUE:
		use_label(while_jmp);
		geni(jmp, 0, 0);
		break;
	case NT_RETURN:
		if(node->id)
			gen_exp(get_child(node), 1);
		geni(sto, 0, 3);
		geno(opr, curr_parm_len, ret);
		break;
	case NT_WRITES:
		geni(lit, 0, node->id);
		geno(opr, 0, writes);
		break;
	case NT_WRITEE:
		gen_exp(get_child(node), 1);
		geno(opr, 0, writee);
		break;
	case NT_READ:
		gen_read(get_child(node));
		break;
	default:
		fprintf(stderr, "unkwon stmt\n");
		exit(1);
	}
}

static void gen_block(struct ast_node *block)
{
	struct ast_node *stmt;
	list_for_each_entry(stmt, &block->chlds, sibling)
	{
		gen_stmt(stmt);
	}
}

static void gen_init(struct type *t, int off)
{
	int i;
	int len;
	len = type_len(t);
	if(type_is_int(t))
		geni(init, _INT, off);
	else if(type_is_float(t))
		geni(init, _FLOAT, off);
	else if(type_is_bool(t))
		geni(init, _BOOL, off);
	else if(type_is_array(t))
	{
		while(t->type == TYPE_ARRAY)
			t = t->t2;
		for(i = 0; i < len; i++)
			gen_init(t, off+i);
	}
	else
	{
		__asm__("int $3");
		dump_type(t, stderr);
		exit(1);
	}
}

static void gen_initexp(struct sym_entry *e)
{
	struct ast_node *node, *p;
	int i;
	node = e->svar.iexp;
	if(node->type == NT_EXP)
	{
		gen_exp(node, 1);
		geni(sto, 0, e->svar.offset);
	}
	else
	{
		i = 0;
		list_for_each_entry(p, &node->chlds, sibling)
		{
			gen_exp(p, 1);
			geni(sto, 0, i+e->svar.offset);
			i++;
		}
	}
}

static int gen_code(struct sym_tab *ptab)
{
	int offset, parm_offset, len;
	int cx_func;
	int cx1 = 0;
	struct sym_entry *entry;
	struct type *t;
	offset = 4;
	parm_offset = 0;
	list_for_each_entry(entry, &ptab->order, order)
		if(type_is_var(entry->type))
		{
			len = type_len(entry->type);
			if(entry->svar.is_param)
			{
				parm_offset -= len;
				entry->svar.offset = parm_offset;
				gen_init(entry->type, parm_offset);
			}
			else
			{
				entry->svar.offset = offset;
				gen_init(entry->type, offset);
				offset += len;
			}
			curr_parm_len = -parm_offset;
		}

	geni(Int, 0, offset);

	list_for_each_entry(entry, &ptab->order, order)
		if(type_is_var(entry->type))
			if(entry->svar.iexp)
				gen_initexp(entry);
	
	if(!ptab->uplink)
	{
		cx1 = cx;
		geni(cal, 0, 0);
		geni(jmp, 0, ret);
	}
	
	list_for_each_entry(entry, &ptab->order, order)
		if(type_is_func(entry->type))
		{
			if(!entry->sfunc.defined)
				continue;
			cx_func = cx;
			printf("function %s is at %d\n",
			       entry->name,
			       cx_func);
			entry->sfunc.addr = cx_func;
			t = entry->sfunc.ret_type;
			if(!type_is_void(t))
				gen_init(t, 3);
			gen_code(entry->sfunc.sym);
			gen_block(entry->sfunc.stmts);
			geno(opr, curr_parm_len, ret);
		}
	return cx1;
}

static void gen_code_all(struct sym_tab *ptab, char *name)
{
	int i;
	int cx1;
	struct sym_entry *main_e;
	char sign;
	/* 初始化 */
	fp = stderr;
	memset(refill, 0, sizeof(refill));
	cx = 0;
	global_tab = ptab;

	geni(lit, 0, 1);
	geni(sto, 0, 0);
	/* 生成代码 */
	cx1 = gen_code(ptab);
	main_e = symtab_lookup(ptab, "main", 1);
	if(!main_e)
	{
		printf("can't find main!\n");
		exit(1);
	}

	/* 重填 */
	code[cx1].v.i = main_e->sfunc.addr;
	code[cx1+1].v.i = cx;
	for(i = 0; i < cx; i++)
	{
		if(refill[i])
		{
			if(type_is_func(refill[i]->type))
			{
				if(!refill[i]->sfunc.defined)
					new_error(1,
						  0,
						  0,
						  "函数 %s 声明后未定义\n",
						  refill[i]->name);
				code[i].v.i = refill[i]->sfunc.addr;
			}
			else if(type_is_label(refill[i]->type))
			{
				code[i].v.i = (long)refill[i]->gen_data;
			}
		}
	}
	listcode();

	/* 写文件 */
	fp = fopen(name, "wb");
	if(!fp)
	{
		new_error(1,
			  0, 0,
			  "Can't open %s .\n", name);
		return;
	}
	for(i = 0; i < cx; i++)
	{
		fwrite(&code[i], sizeof(instruction), 1, fp);
	}
	sign = 0xff;
	for(i = 0; i < sizeof(instruction); i++)
		fwrite(&sign, 1, 1, fp);
	for(i = 0; i < strs_count; i++)
		fwrite(strs[i],
		       strlen(strs[i])+1,
		       1,
		       fp);
	fclose(fp);
}

struct gen_info gen_eir = {
	.name = "eir",
	.gen_code = gen_code_all,
	.info = "C1到eir虚拟机"
};
