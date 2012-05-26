#ifndef _TYPE_H_
#define _TYPE_H_
#include <stdio.h>

struct type {
	struct list_head list;
	enum {
		TYPE_VOID,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_BOOL,
		TYPE_ARRAY,
		TYPE_FUNC,
		TYPE_LABEL,
		TYPE_TYPE,
	} type;
	int n;
	struct type *t1;
	struct type *t2;
};

struct type_list {
	struct list_head list;
	struct type *type;
	char *name;
};

#define type_is_int(t) ((t)->type == TYPE_INT)
#define type_is_float(t) ((t)->type == TYPE_FLOAT)
#define type_is_bool(t) ((t)->type == TYPE_BOOL)
#define type_is_void(t) ((t)->type == TYPE_VOID)
#define type_is_array(t) ((t)->type == TYPE_ARRAY)
#define type_is_func(t) ((t)->type == TYPE_FUNC)

#define type_is_var(t) \
	(type_is_int(t) || type_is_float(t) \
	 || type_is_bool(t) || type_is_array(t))
#define type_is_label(t) ((t)->type == TYPE_LABEL)
#define type_is_type(t) ((t)->type == TYPE_TYPE)


struct type *get_type(int type, int n, struct type *t1, struct type *t2);
struct type *array_type(struct type *t, int n);
struct type *func_type(struct type *t, struct list_head *type_list);
void dump_type(struct type *t, FILE *fp);

struct list_head *type_list_start(struct type *type, char *name);
struct list_head *type_list_add(struct list_head *h, struct type *type, char *name);
#endif /* _TYPE_H_ */
