#ifndef _SYM_TABLE_H_
#define _SYM_TABLE_H_
#include <stdlib.h>
#include "list.h"
#include "ast_node.h"

struct sym_tab;

struct sym_func {
	struct ast_node *stmts;
	struct sym_tab *sym;
	unsigned int addr;
};
struct sym_var {
	unsigned int offset;
};
struct sym_const {
	int value;
};

enum {
	SYM_FUNC,
	SYM_VAR,
	SYM_CONST,
};

struct sym_entry {
	struct list_head list;
	struct list_head order;
	struct sym_tab *tab;
	char *name;
	int type;
	void *gen_data;
	union {
		struct sym_func sfunc;
		struct sym_var  svar;
		struct sym_const sconst;
	};
};
	
#define HASHSIZE 256
struct sym_tab {
	struct list_head entry[HASHSIZE];
	struct list_head order;
	struct sym_tab *uplink;
};

void symtab_enter_func(struct sym_tab *ptab, char *name);
void symtab_modify_func(struct sym_tab *ptab,
		       char *name,
		       struct ast_node *stmts,
		       struct sym_tab  *ftab);
void symtab_enter_var(struct sym_tab *ptab, char *name);
void symtab_enter_const(struct sym_tab *ptab, char *name, int val);

struct sym_entry *symtab_lookup(struct sym_tab *ptab, char *name);
struct sym_tab *symtab_new(struct sym_tab *uplink);
void symtab_destroy(struct sym_tab *ptab);

#endif /* _SYM_TABLE_H_ */
