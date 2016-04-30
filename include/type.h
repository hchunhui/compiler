#ifndef _TYPE_H_
#define _TYPE_H_
#include <stdio.h>
struct type {
    struct list_head list;
    enum {
	TYPE_VOID = 0,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_BOOL,
	TYPE_ARRAY,
	TYPE_FUNC,
	TYPE_LABEL,
	TYPE_TYPE,
    } type;
    int             n;
    int             is_const;
    union {
	struct sym_entry *e;
	struct type    *t1;
    };
    struct type    *t2;
};

struct type_list {
    struct list_head list;
    struct type    *type;
    char           *name;
};

static inline int
type_is_xx(struct type *t, int type)
{
    if (t->type == type)
	return 1;
    if (t->type != TYPE_TYPE)
	return 0;
    /*
     * if type is TYPE_TYPE, follow down 
     */
    return type_is_xx(t->t2, type);
}

#define type_is_int(t) type_is_xx(t, TYPE_INT)
#define type_is_float(t) type_is_xx(t, TYPE_FLOAT)
#define type_is_bool(t) type_is_xx(t, TYPE_BOOL)
#define type_is_void(t) type_is_xx(t, TYPE_VOID)
#define type_is_func(t) type_is_xx(t, TYPE_FUNC)
#define type_is_type(t) type_is_xx(t, TYPE_TYPE)
#define type_is_array(t) type_is_xx(t, TYPE_ARRAY)

#define type_is_var(t) \
	(type_is_int(t) || type_is_float(t)		\
	 || type_is_bool(t) || type_is_array(t))

#define type_is_label(t) ((t)->type == TYPE_LABEL)
static inline int
type_is_const(struct type *t)
{
    if (t->is_const)
	return 1;
    if (type_is_array(t) || type_is_type(t))
	return type_is_const(t->t2);
    return 0;
}

static inline int
type_is_equal_byname(struct type *t1, struct type *t2)
{
    return t1 == t2;
}

static inline int
type_is_equal_bystru(struct type *ty1, struct type *ty2)
{
    if (ty1->type == TYPE_TYPE)
	return type_is_equal_bystru(ty1->t2, ty2);
    if (ty2->type == TYPE_TYPE)
	return type_is_equal_bystru(ty1, ty2->t2);
    if (ty1->type != ty2->type)
	return 0;
    if (ty1->type == TYPE_VOID ||
	ty1->type == TYPE_INT ||
	ty1->type == TYPE_FLOAT ||
	ty1->type == TYPE_BOOL || ty1->type == TYPE_LABEL)
	return 1;
    if (ty1->type == TYPE_FUNC)
	return type_is_equal_bystru(ty1->t1, ty2->t1) &&
	    type_is_equal_bystru(ty1->t2, ty2->t2);
    if (ty1->type == TYPE_ARRAY)
	return ty1->n == ty2->n && type_is_equal_bystru(ty1->t2, ty2->t2);
    return 0;
}

struct type    *get_type(int type, int n, int is_const, void *t1,
			 struct type *t2);
struct type    *array_type(struct type *t, int n);
struct type    *func_type(struct type *t, struct list_head *type_list);
void            dump_type(struct type *t, FILE * fp);

struct list_head *type_list_start(struct type *type, char *name);
struct list_head *type_list_add(struct list_head *h, struct type *type,
				char *name);

int             type_len(struct type *t);
void            type_init();
#endif				/* _TYPE_H_ */
