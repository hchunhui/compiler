#ifndef _AST_NODE_H_
#define _AST_NODE_H_
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

struct ast_node {
	unsigned short type;
	unsigned short id;
	struct list_head sibling;
	int first_line;
	int first_column;
	union {
		void *pval;
		int ival;
		float fval;
		struct list_head chlds;
	};
};

#define T_LEAF 0x8000

#define xmalloc(x, len)					\
	do {						\
	x = malloc(len);				\
	if(!x) {					\
		fprintf(stderr, "malloc fail!\n"); \
		exit(1); \
	} \
	} while(0)

static inline struct ast_node *ast_node_new(int type, int id)
{
	struct ast_node *ptr;
	xmalloc(ptr, sizeof(struct ast_node));
	ptr->type = type;
	ptr->id = id;
	INIT_LIST_HEAD(&ptr->sibling);
	if(!(type&T_LEAF))
		INIT_LIST_HEAD(&ptr->chlds);
	return ptr;
}

static inline void ast_node_delete(struct ast_node *ptr)
{
	struct ast_node *p;
	struct ast_node *tmp;
	if(!(ptr->type&T_LEAF))
		list_for_each_entry_safe(p, tmp, &ptr->chlds, sibling)
		{
			list_del(&p->sibling);
			ast_node_delete(p);
		}
	free(ptr);
}

static inline void ast_node_add_chld(
	struct ast_node *fath,
	struct ast_node *chld)
{
	if(chld)
		list_add_tail(&chld->sibling, &fath->chlds);
}

static inline void get_lr_child(struct ast_node *node,
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
		*r = NULL;
}

static inline struct ast_node *get_child(struct ast_node *node)
{
	struct ast_node *p;
	list_for_each_entry(p, &node->chlds, sibling)
		return p;
	return NULL;
}

enum {
#define TYPE(a, b) NT_##a,
#define LEAF_START() NT_NONE = T_LEAF,
#include "node_type.h"
#undef TYPE
};

#endif /* _AST_NODE_H_ */
