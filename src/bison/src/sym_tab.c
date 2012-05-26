#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sym_tab.h"
#include "ast_node.h"
#include "error.h"

#define xmalloc(x, len)					\
	do {						\
	x = malloc(len);				\
	if(!x) {					\
		fprintf(stderr, "malloc fail!\n"); \
		exit(1); \
	} \
	} while(0)

/* JS Hash Function */
static unsigned int strhash(char *_str)
{
	char *str = _str;
	unsigned int hash = 1315423911;
	for(;*str;str++)
		hash ^= (hash << 5) + *str + (hash >> 2);
	/*printf("hash: %s -> %u\n", _str, hash);*/
	return hash;
}

static struct sym_entry *find_entry(struct sym_tab *ptab, char *name)
{
	unsigned int hash, idx;
	struct sym_entry *entry;
	hash = strhash(name);
	idx = hash%HASHSIZE;
	list_for_each_entry(entry, &ptab->entry[idx], list)
	{
		if(strcmp(name, entry->name) == 0)
			return entry;
	}
	return NULL;
}

static struct sym_entry *new_entry(struct sym_tab *ptab, char *name)
{
	unsigned int hash, idx;
	struct sym_entry *entry;

	hash = strhash(name);
	xmalloc(entry, sizeof(struct sym_entry));
	idx = hash%HASHSIZE;
	entry->name = strdup(name);
	entry->tab = ptab;
	entry->gen_data = NULL;
	list_add_tail(&entry->list, &ptab->entry[idx]);
	list_add_tail(&entry->order, &ptab->order);
	return entry;
}

struct sym_entry *
symtab_enter(struct sym_tab *ptab, char *name, struct type *type)
{
	struct sym_entry *entry;
	if(find_entry(ptab, name))
		new_error(1,
			  0,
			  0, "符号重名\n");
	entry = new_entry(ptab, name);
	entry->type = type;
	switch(type->type == TYPE_FUNC)
	{
	case TYPE_FUNC:
		entry->sfunc.stmts = NULL;
		entry->sfunc.sym = NULL;
		entry->sfunc.addr = 0;
		entry->sfunc.defined = 0;
		break;
	case TYPE_INT:
	case TYPE_FLOAT:
	case TYPE_BOOL:
		entry->svar.is_param = 0;
		break;
	}
	return entry;
}

struct sym_entry *symtab_lookup(struct sym_tab *ptab, char *name, int follow)
{
	struct sym_entry *entry;
	if(!follow)
		return find_entry(ptab, name);
	while(ptab)
	{
		if(entry = find_entry(ptab, name))
			return entry;
		ptab = ptab->uplink;
	}
	return NULL;
}

struct sym_tab *symtab_new(struct sym_tab *uplink)
{
	struct sym_tab *ptab;
	int i;
	xmalloc(ptab, sizeof(struct sym_tab));
	for(i = 0; i < HASHSIZE; i++)
		INIT_LIST_HEAD(&ptab->entry[i]);
	INIT_LIST_HEAD(&ptab->order);
	ptab->uplink = uplink;
	return ptab;
}

void symtab_destroy(struct sym_tab *ptab)
{
	int i;
	struct sym_entry *e, *tmp;
	list_for_each_entry_safe(e, tmp, &ptab->order, order)
	{
		list_del(&e->order);
		free(e);
	}
	free(ptab);
}
