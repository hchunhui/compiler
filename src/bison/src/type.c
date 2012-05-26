#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "type.h"
#include "error.h"

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

static unsigned long typehash(int type, int n, struct type *t1, struct type *t2)
{
	return type ^ n ^ (((unsigned long)t1)>>3) ^ (((unsigned long)t2)>>3);
}

static struct type *find_type(int type, int n, struct type *t1, struct type *t2)
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

struct type *get_type(int type, int n, struct type *t1, struct type *t2)
{
	unsigned int hash, idx;
	struct type *e;
	e = find_type(type, n, t1, t2);
	if(e)
		return e;
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

struct type *array_type(struct type *t, int n)
{
	struct type *nt;
	nt = t;
	if(nt->type != TYPE_ARRAY &&
	   nt->type != TYPE_FUNC)
		nt = get_type(TYPE_ARRAY, n, NULL, nt);
	else
		nt = get_type(nt->type,
			      nt->n,
			      nt->t1,
			      get_type(TYPE_ARRAY, n, NULL, nt->t2));
	return nt;
}

struct type *func_type(struct type *t, struct list_head *type_list)
{
	struct type_list *tle, *tmp;
	struct type *nt;
	if(t->type != TYPE_ARRAY &&
	   t->type != TYPE_FUNC)
		nt = t;
	else
		nt = t->t2;
		
	list_for_each_entry_safe(tle, tmp, type_list, list)
	{
		nt = get_type(TYPE_FUNC, 0, tle->type, nt);
		/*list_del(&tle->list);*/
		/*free(tle);*/
	}
	/*free(type_list);*/
	
	if(t->type == TYPE_ARRAY ||
	   t->type == TYPE_FUNC)
		nt = get_type(t->type,
			      t->n,
			      t->t1,
			      nt);
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

void type_init()
{
	int i;
	for(i = 0; i < HASHSIZE; i++)
		INIT_LIST_HEAD(&type_entry[i]);
}
