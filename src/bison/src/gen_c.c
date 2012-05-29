#include <stdio.h>
#include "ast_node.h"
#include "sym_tab.h"
#include "c0.tab.h"
#include "gen.h"
#include "error.h"

/* forward decl */
static void gen_exp(struct ast_node *node);
static void gen_if(struct ast_node *node);
static void gen_while(struct ast_node *node);
static void gen_stmt(struct ast_node *node);
static void gen_block(struct ast_node *block);
static void gen_code(struct sym_tab *ptab);

static FILE *fp;

static void gen_explist(struct ast_node *list)
{
	struct ast_node *exp;
	if(list)
		list_for_each_entry(exp, &list->chlds, sibling)
		{
			gen_exp(exp);
		}
}

static void gen_exp(struct ast_node *node)
{
	int i;
	struct ast_node *p, *l, *r;
	struct sym_entry *entry;
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
	case AND: func = "&&";break;
	case OR: func = "||";break;
	case NOT: func = "!";break;
	case '|': func = "|";break;
	case '&': func = "&";break;
	case '~': func = "~";break;
	case '=': func = "=";break;
	case 'F':
		get_lr_child(node, &l, &r);
		gen_exp(l);
		fprintf(fp, "(");
		gen_explist(r);
		fprintf(fp, ")");
		return; 
	case 'i':
	case 'b':
		fprintf(fp,"%d ", node->ival);
		break;
	case 'f':
		fprintf(fp,"%f ", node->fval);
		break;
	case 'I':
		entry = node->pval;
		fprintf(fp,"%s ", entry->name);
		break;
	case 'A':
		i = 0;
		list_for_each_entry(p, &node->chlds, sibling)
		{
			if(i == 0)
			{
				fprintf(fp, "(");
				gen_exp(p);
				fprintf(fp, ")");
			}
			else
			{
				fprintf(fp, "[");
				gen_exp(p);
				fprintf(fp, "]");
			}
			i++;
		}
		return; 
	default: new_error(1,
			   node->first_line,
			   node->first_column,
			   "gen_exp error\n");
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

static void gen_if(struct ast_node *node)
{
	struct ast_node *p;
	int i;
	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		switch(i)
		{
		case 0:
			fprintf(fp, "if(");
			gen_exp(p);
			fprintf(fp, ") ");
			break;
		case 2:
			fprintf(fp, " else ");
		case 1:
			gen_stmt(p);
			break;
		default: new_error(1,
				   node->first_line,
				   node->first_column,
				   "bad if stmt\n");
		}
		i++;
	}
}

static void gen_while(struct ast_node *node)
{
	struct ast_node *exp, *stmt;
	get_lr_child(node, &exp, &stmt);
	fprintf(fp,"while(");
	gen_exp(exp);
	fprintf(fp,") ");
	gen_stmt(stmt);
}

static void gen_stmt(struct ast_node *node)
{
	if(!node)
	{
		fprintf(fp, ";\n");
		return;
	}
	switch(node->type)
	{
	case NT_EXP:
		gen_exp(node);fprintf(fp,";\n");
		break;
	case NT_IF:
		gen_if(node);
		break;
	case NT_WHILE:
		gen_while(node);
		break;
	case NT_NUL:
		fprintf(fp, ";\n");
		break;
	case NT_BLOCK:
		fprintf(fp,"{\n");gen_block(node);fprintf(fp,"}\n");
		break;
	case NT_BREAK:
		fprintf(fp, "break;\n");
		break;
	case NT_RETURN:
		fprintf(fp,"return ");
		if(node->id)
			gen_exp(get_child(node));
		fprintf(fp,";\n");
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

static void gen_code(struct sym_tab *ptab)
{
	struct sym_entry *entry;
	
	list_for_each_entry(entry, &ptab->order, order)
		if(entry->type->type == TYPE_FUNC)
		{
			if(!entry->sfunc.defined)
				continue;
			fprintf(fp, "function: %s, type ", entry->name);
			dump_type(entry->type, fp);
			fprintf(fp, "\n{\n");
			gen_code(entry->sfunc.sym);
			gen_block(entry->sfunc.stmts);
			fprintf(fp,"}\n\n");
		}
		else
		{
			fprintf(fp, "var: %s, type ", entry->name);
			dump_type(entry->type, fp);
			fprintf(fp, "\n");
		}
}

static void gen_code_all(struct sym_tab *ptab, char *name)
{
	fp = fopen(name, "w");
	if(!fp)
	{
		new_error(1,
			  0, 0,
			  "Can't open %s .\n", name);
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
