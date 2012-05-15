void dump_ast(struct ast_node *node)
{
	struct ast_node *p;
	struct sym_entry *entry;
	if(node->type&T_LEAF)
	{
		switch(node->type)
		{
		case NT_NUMBER:
			printf("%d ", node->ival);
			break;
		case NT_IDENT:
			entry = node->pval;
				printf("%s ",
				       entry->name);
			break;
		}
	}
	else
	{
		if(0);
		#undef LEAF_START
		#undef TYPE
		#define LEAF_START()
		#define TYPE(a, b) \
		else if(node->type == NT_##a)		\
			printf("(%s ", b);
		#include "node_type.h"
		list_for_each_entry(p, &node->chlds, sibling)
		{
			dump_ast(p);
			printf(" ");
			if(node->type == NT_BLOCK)
				printf("\n");
		}
		printf(")");
	}
}

void dump_symtab(struct sym_tab *ptab)
{
	int i;
	struct sym_entry *entry;
	list_for_each_entry(entry, &ptab->order, order)
	{
		switch(entry->type)
		{
		case SYM_VAR:
			printf("(int %s )\n", entry->name);
			break;
		case SYM_CONST:
			printf("(const %s %d )\n",
			       entry->name,
			       entry->sconst.value);
			break;
		case SYM_FUNC:
			printf("(defun %s\n", entry->name);
			dump_symtab(entry->sfunc.sym);
			dump_ast(entry->sfunc.stmts);
			printf(")\n");
			break;
		}
	}
}
