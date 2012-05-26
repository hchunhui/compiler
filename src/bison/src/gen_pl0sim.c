#include <stdio.h>
#include "ast_node.h"
#include "sym_tab.h"
#include "c0.tab.h"
#include "pl0_inst.h"
#include "gen.h"

#define CXMAX 2000
static instruction code[CXMAX];
static int cx;

/* forward decl */
static void gen_leaf(struct ast_node *leaf);
static void gen_call(struct ast_node *node);
static void gen_exp(struct ast_node *node);
static void gen_cond(struct ast_node *node);
static void gen_assign(struct ast_node *node);
static void gen_if(struct ast_node *node);
static void gen_while(struct ast_node *node);
static void gen_stmt(struct ast_node *node);
static void gen_block(struct ast_node *block);
static int gen_code(struct sym_tab *ptab);

static void gen(enum fct x, long y, long z)
{
	if(cx >= CXMAX) {
		printf("program too long\n");
		exit(1);
	}
	code[cx].f=x;
	code[cx].l=y;
	code[cx].a=z;
	cx++;
}

static void gen_leaf(struct ast_node *node)
{
	struct sym_entry *entry;
	switch(node->type)
	{
	case NT_NUMBER:
		gen(lit, 0, node->ival);
		break;
	case NT_IDENT:
		entry = node->pval;
		switch(entry->type)
		{
		case SYM_CONST:
			gen(lit, 0, entry->sconst.value);
			break;
		case SYM_VAR:
			if(entry->tab->uplink)
				gen(lod,
				    0,
				    entry->svar.offset);
			else
				gen(lod,
				    1,
				    entry->svar.offset);
			break;
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
	gen(cal, 1, e->sfunc.addr);
}

static void gen_exp(struct ast_node *node)
{
	struct ast_node *p;
	int func;
	switch(node->id)
	{
		//case UMINUS: func = 1;break;
	case '+': func = 2;break;
	case '-': func = 3;break;
	case '*': func = 4;break;
	case '/': func = 5;break;
	case 'N':
	case 'I': gen_leaf(get_child(node));return;
	default: printf("gen_exp error\n");exit(1);
	}
	list_for_each_entry(p, &node->chlds, sibling)
		gen_exp(p);
	gen(opr, 0, func);
}

static void gen_cond(struct ast_node *node)
{
	struct ast_node *p;
	int func;
	switch(node->id)
	{
	case '%': func = 6;break;
	case EQ_OP: func = 8;break;
	case NE_OP: func = 9;break;
	case '<': func = 10;break;
	case GE_OP: func = 11;break;
	case '>': func = 12;break;
	case LE_OP: func = 13;break;
	default: return;
	}
	list_for_each_entry(p, &node->chlds, sibling)
		gen_exp(p);
	gen(opr, 0, func);
}

static void gen_assign(struct ast_node *node)
{
	struct ast_node *id, *exp;
	struct sym_entry *e;
	int lev;

	get_lr_child(node, &id, &exp);
	e = id->pval;
	if(e->type != SYM_VAR)
	{
		exit(2);
	}
	gen_exp(exp);
	
	if(e->tab->uplink)
		lev = 0;
	else
		lev = 1;
	gen(sto, lev, e->svar.offset);
}

static void gen_if(struct ast_node *node)
{
	int cx1;
	struct ast_node *cond, *stmt;
	struct sym_entry *e;
	get_lr_child(node, &cond, &stmt);
	gen_cond(cond);
	cx1 = cx;
	gen(jpc, 0, 0);
	gen_stmt(stmt);
	code[cx1].a = cx;
}

static void gen_while(struct ast_node *node)
{
	int cx1, cx2;
	struct ast_node *cond, *stmt;
	struct sym_entry *e;
	get_lr_child(node, &cond, &stmt);
	cx1 = cx;
	gen_cond(cond);
	cx2 = cx;
	gen(jpc, 0, 0);
	gen_stmt(stmt);
	gen(jmp, 0, cx1);
	code[cx2].a = cx;
}

static void gen_stmt(struct ast_node *node)
{
	switch(node->type)
	{
	case NT_CALL:	
		gen_call(node);
		break;
	case NT_EXP:
		gen_exp(node);
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
	offset = 3;
	list_for_each_entry(entry, &ptab->order, order)
		if(entry->type == SYM_VAR)
			entry->svar.offset = offset++;

	gen(Int, 0, offset);
	if(!ptab->uplink)
	{
		cx1 = cx;
		gen(cal, 0, 0);
		gen(opr, 0, 0);
	}
	
	list_for_each_entry(entry, &ptab->order, order)
		if(entry->type == SYM_FUNC)
		{
			cx_func = cx;
			printf("function %s is at %d\n",
			       entry->name,
			       cx_func);
			entry->sfunc.addr = cx_func;
			gen_code(entry->sfunc.sym);
			gen_block(entry->sfunc.stmts);
			gen(opr, 0, 0);
		}
	return cx1;
}

static void listcode()
{
	long i;
	for(i = 0; i < cx; i++)
		fprintf(stderr,
			"%10ld%5s%3ld%5ld\n",
			i,
			mnemonic[code[i].f],
			code[i].l,
			code[i].a);
}

static void gen_code_all(struct sym_tab *ptab, char *name)
{
	int cx1;
	struct sym_entry *main_e;
	cx = 0;
	cx1 = gen_code(ptab);
	main_e = symtab_lookup(ptab, "main");
	if(!main_e)
	{
		printf("can't find main!\n");
		exit(1);
	}
	code[cx1].a = main_e->sfunc.addr;
	write_file(name, code, cx);
	listcode();
}

struct gen_info gen_pl0sim = {
	.name = "pl0sim",
	.gen_code = gen_code_all,
	.info = "C0到PL/0虚拟机"
};
