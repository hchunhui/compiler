#ifndef _SYM_TABLE_H_
#define _SYM_TABLE_H_
#include <stdlib.h>
#include "list.h"
#include "ast_node.h"
#include "type.h"

struct sym_tab;

struct sym_func {
	int defined;
	struct type *ret_type;
	struct ast_node *stmts;
	struct sym_tab *sym;
	unsigned int addr;
};
struct sym_var {
	unsigned int offset;
	int is_param;
	struct ast_node *iexp;
};

struct sym_entry {
	struct list_head list;
	struct list_head order;
	struct sym_tab *tab;
	char *name;
	enum {
		SYM_VAR,
		SYM_FUNC,
		SYM_TYPE,
	} kind;
	unsigned int attr;
	struct type *type;
	void *gen_data;
	union {
		struct sym_func sfunc;
		struct sym_var  svar;
	};
};
	
#define HASHSIZE 256
struct sym_tab {
	struct list_head entry[HASHSIZE];
	struct list_head order;
	struct sym_tab *uplink;
};

struct sym_entry *
symtab_enter(struct sym_tab *ptab, char *name, struct type *type);
struct sym_entry *
symtab_enter_t(struct sym_tab *ptab, char *name, struct type *type);
struct sym_entry *
symtab_enter_type(struct sym_tab *ptab, char *name, struct type *type);

struct sym_entry *symtab_lookup(struct sym_tab *ptab, char *name, int follow);
struct sym_tab *symtab_new(struct sym_tab *uplink);
void symtab_destroy(struct sym_tab *ptab);

#endif /* _SYM_TABLE_H_ */
