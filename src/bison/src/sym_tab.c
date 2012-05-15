#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sym_tab.h"
#include "ast_node.h"

#define xmalloc(x, len)					\
	do {						\
	x = malloc(len);				\
	if(!x) {					\
		fprintf(stderr, "malloc fail!\n"); \
		exit(1); \
	} \
	} while(0)

int new_error(char *str)
{
	puts(str);
	exit(1);
}

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

void symtab_enter_func(struct sym_tab *ptab, char *name)
{
	struct sym_entry *entry;
	if(find_entry(ptab, name))
		new_error("dup name");
	entry = new_entry(ptab, name);
	entry->type = SYM_FUNC;
	entry->sfunc.stmts = NULL;
	entry->sfunc.sym = NULL;
}

void symtab_modify_func(struct sym_tab *ptab,
		       char *name,
		       struct ast_node *stmts,
		       struct sym_tab  *sym)
{
	struct sym_entry *entry;
	if(!(entry = find_entry(ptab, name)))
		new_error("can't find name");
	entry->sfunc.stmts = stmts;
	entry->sfunc.sym = sym;
}

void symtab_enter_var(struct sym_tab *ptab, char *name)
{
	struct sym_entry *entry;
	if(find_entry(ptab, name))
		new_error("dup name");
	entry = new_entry(ptab, name);
	entry->type = SYM_VAR;
}

void symtab_enter_const(struct sym_tab *ptab, char *name, int val)
{
	struct sym_entry *entry;
	if(find_entry(ptab, name))
		new_error("dup name");
	entry = new_entry(ptab, name);
	entry->type = SYM_CONST;
	entry->sconst.value = val;
}

struct sym_entry *symtab_lookup(struct sym_tab *ptab, char *name)
{
	struct sym_entry *entry;
	while(ptab)
	{
		if(entry = find_entry(ptab, name))
			return entry;
		ptab = ptab->uplink;
	}
	new_error("no name");
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
	
}
