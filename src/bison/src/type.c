#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "type.h"
#include "error.h"
#include "sym_tab.h"

#define HASHSIZE 256
struct list_head type_entry[HASHSIZE];

#define xmalloc(x, len)						\
	do {							\
		x = malloc(len);				\
		if(!x) {					\
			fprintf(stderr, "malloc fail!\n");	\
			exit(1);				\
		}						\
	} while(0)

static unsigned long typehash(int type, int n, void *t1, struct type *t2)
{
	return type ^ n ^ (((unsigned long)t1)>>3);
}

static struct type *find_type(int type, int n, void *t1, struct type *t2)
{
	unsigned int hash, idx;
	struct type *e;
	hash = typehash(type, n, t1, t2);
	idx = hash%HASHSIZE;
	list_for_each_entry(e, &type_entry[idx], list)
	{
		if(e->type == type &&
		   e->n == n &&
		   e->t1 == t1 &&
		   e->t2 == t2)
			return e;
	}
	return NULL;
}

static struct type *new_type(int type, int n, void *t1, struct type *t2)
{
	unsigned int hash, idx;
	struct type *e;
	hash = typehash(type, n, t1, t2);
	xmalloc(e, sizeof(struct type));
	idx = hash%HASHSIZE;
	e->type = type;
	e->n = n;
	e->t1 = t1;
	e->t2 = t2;
	list_add_tail(&e->list, &type_entry[idx]);
	return e;
}

struct type *get_type(int type, int n, void *t1, struct type *t2)
{
	struct type *e;
	e = find_type(type, n, t1, t2);
	if(e)
		return e;
	return new_type(type, n, t1, t2);
}

struct type *array_type(struct type *t, int n)
{
	struct type *p;
	if(t->type != TYPE_ARRAY &&
	   t->type != TYPE_FUNC)
		return new_type(TYPE_ARRAY, n, NULL, t);
	p = t;
	while(p->t2->type == TYPE_ARRAY ||
	      p->t2->type == TYPE_FUNC)
		p = p->t2;
	p->t2 = new_type(TYPE_ARRAY, n, NULL, p->t2);
	return t;
}

struct type *func_type(struct type *t, struct list_head *type_list)
{
	struct type_list *tle, *tmp;
	struct type *p, *nt;
	if(t->type != TYPE_ARRAY &&
	   t->type != TYPE_FUNC)
	{
		p = t;
		nt = p;
	}
	else
	{
		p = t;
		while(p->t2->type == TYPE_ARRAY ||
		      p->t2->type == TYPE_FUNC)
			p = p->t2;
		nt = p->t2;
	}

	if(type_list == NULL)
		nt = new_type(TYPE_FUNC, 0,
			      get_type(TYPE_VOID, 0, NULL, NULL),
			      nt);
	else
		list_for_each_entry_safe(tle, tmp, type_list, list)
			nt = new_type(TYPE_FUNC, 0, tle->type, nt);
	
	if(t->type == TYPE_ARRAY ||
	   t->type == TYPE_FUNC)
	{
		p->t2 = nt;
		return t;
	}
	return nt;
}

void dump_type(struct type *t, FILE *fp)
{
	switch(t->type)
	{
	case TYPE_INT:
		fprintf(fp,"int");
		break;
	case TYPE_FLOAT:
		fprintf(fp,"float");
		break;
	case TYPE_BOOL:
		fprintf(fp,"bool");
		break;
	case TYPE_VOID:
		fprintf(fp,"void");
		break;
	case TYPE_FUNC:
		fprintf(fp,"(");
		dump_type(t->t1, fp);
		fprintf(fp,")->(");
		dump_type(t->t2, fp);
		fprintf(fp,")");
		break;
	case TYPE_ARRAY:
		fprintf(fp,"array(%d, ", t->n);
		dump_type(t->t2, fp);
		fprintf(fp,")");
		break;
	case TYPE_LABEL:
		fprintf(fp,"label");
		break;
	case TYPE_TYPE:
		fprintf(fp, "%s", t->e->name);
		//dump_type(t->t2, fp);
		break;
	default:
		fprintf(fp,"\n错误：%d\n", t->type);
		exit(1);
	}
}

struct list_head *type_list_start(struct type *type, char *name)
{
	struct type_list *lp;
	struct list_head *h;
	xmalloc(lp, sizeof(struct type_list));
	xmalloc(h, sizeof(struct list_head));
	lp->type = type;
	lp->name = name;
	INIT_LIST_HEAD(h);
	list_add(&lp->list, h);
	return h;
}

struct list_head *type_list_add(struct list_head *h, struct type *type, char *name)
{
	struct type_list *lp;
	xmalloc(lp, sizeof(struct type_list));
	lp->type = type;
	lp->name = name;
	list_add(&lp->list, h);
	return h;
}

int type_len(struct type *t)
{
	if(type_is_type(t))
		return type_len(t->t2);
	if(type_is_var(t) && !type_is_array(t))
		return 1;
	if(!type_is_var(t))
		return 0;
	return t->n * type_len(t->t2);
}

void type_init()
{
	int i;
	for(i = 0; i < HASHSIZE; i++)
		INIT_LIST_HEAD(&type_entry[i]);
}
