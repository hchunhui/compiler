#include <stdio.h>
#include "ast_node.h"
#include "sym_tab.h"
#include "c0.tab.h"
#include "pl0_inst.h"
#include "gen.h"

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
static void gen_code(struct sym_tab *ptab);

static instruction code[100];
static int cx;
static FILE *fp;

static void get_lr_child(struct ast_node *node,
			 struct ast_node **l,
			 struct ast_node **r)
{
	struct ast_node *p;
	int i;
	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		if(i == 0)
			*l = p;
		else if(i == 1)
			*r = p;
		i++;
	}
	if(i != 2)
	{
		*r = NULL;
		/*fprintf(fp,"error get_lr_child\n");
		  exit(1);*/
	}
}

static struct ast_node *get_child(struct ast_node *node)
{
	struct ast_node *p;
	list_for_each_entry(p, &node->chlds, sibling)
		return p;
	fprintf(fp,"error get_child\n");
	exit(1);
}

static void gen_leaf(struct ast_node *node)
{
	struct sym_entry *entry;
	switch(node->type)
	{
	case NT_NUMBER:
		fprintf(fp,"%d ", node->ival);
		break;
	case NT_IDENT:
		entry = node->pval;
		fprintf(fp,"%s ", entry->name);
		break;
	default:
		fprintf(fp,"gen_leaf error\n");
		exit(1);
	}
}

static void gen_call(struct ast_node *node)
{
	struct ast_node *p;
	struct sym_entry *e;
	p = get_child(node);
	e = p->pval;
	fprintf(fp,"%s() ", e->name);
}

static void gen_exp(struct ast_node *node)
{
	struct ast_node *p, *l, *r;
	char *func;
	switch(node->id)
	{
	case '+':func = "+";break;
	case '-':func = "-";break;
	case '*':func = "*";break;
	case '/':func = "/";break;
	case EQ_OP: func = "==";break;
	case NE_OP: func = "!=";break;
	case '<': func = "<";break;
	case GE_OP: func = ">=";break;
	case '>': func = ">";break;
	case LE_OP: func = "<=";break;
	case '%': func = "%";break;
	case 'N':
	case 'I': gen_leaf(get_child(node));return;
	default: fprintf(fp,"gen_exp error\n");exit(1);
	}
	get_lr_child(node, &l, &r);
	if(r)
	{
		fprintf(fp,"(");
		gen_exp(l);
		fprintf(fp,")%s(", func);
		gen_exp(r);
		fprintf(fp,") ");
	}
	else
	{
		fprintf(fp,"%s(", func);
		gen_exp(l);
		fprintf(fp,") ");
	}
}

static void gen_assign(struct ast_node *node)
{
	struct ast_node *id, *exp;
	struct sym_entry *e;
	int lev;

	get_lr_child(node, &id, &exp);
	e = id->pval;
	fprintf(fp,"%s = ", e->name);
	gen_exp(exp);
}

static void gen_if(struct ast_node *node)
{
	int cx1;
	struct ast_node *cond, *stmt;
	struct sym_entry *e;
	get_lr_child(node, &cond, &stmt);
	fprintf(fp,"if(");
	gen_exp(cond);
	fprintf(fp,") ");
	gen_stmt(stmt);
}

static void gen_while(struct ast_node *node)
{
	int cx1, cx2;
	struct ast_node *cond, *stmt;
	struct sym_entry *e;
	get_lr_child(node, &cond, &stmt);
	fprintf(fp,"while(");
	gen_exp(cond);
	fprintf(fp,") ");
	gen_stmt(stmt);
}

static void gen_stmt(struct ast_node *node)
{
	switch(node->type)
	{
	case NT_CALL:	
		gen_call(node);fprintf(fp,";\n");
		break;
	case NT_EXP:
		gen_exp(node);fprintf(fp,";\n");
		break;
	case NT_ASSIGN:
		gen_assign(node);fprintf(fp,";\n");
		break;
	case NT_IF:
		gen_if(node);
		break;
	case NT_WHILE:
		gen_while(node);
		break;
	case NT_BLOCK:
		fprintf(fp,"{\n");gen_block(node);fprintf(fp,"}\n");
		break;
	default:
		fprintf(fp,"unkwon stmt\n");
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

static void gen_code(struct sym_tab *ptab)
{
	int offset;
	int cx_func;
	int cx1 = 0;
	struct sym_entry *entry;
	
	list_for_each_entry(entry, &ptab->order, order)
		if(entry->type == SYM_FUNC)
		{
			fprintf(fp,"void %s()\n{\n", entry->name);
			gen_code(entry->sfunc.sym);
			gen_block(entry->sfunc.stmts);
			fprintf(fp,"}\n\n");
		}
		else if(entry->type == SYM_VAR)
			fprintf(fp,"int %s;\n", entry->name);
		else
			fprintf(fp,"int %s = %d;\n",
			       entry->name,
			       entry->sconst.value);
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
	gen_code(ptab);
	fclose(fp);
}

struct gen_info gen_c = {
	.name = "c",
	.gen_code = gen_code_all,
	.info = "C0到C0源代码"
};
