#include <stdio.h>
#include "ast_node.h"
#include "sym_tab.h"
#include "c0.tab.h"
#include "gen.h"
#include "error.h"
#include "type.h"

/*
 * 需要做的检查
 * 1. 类型检查和提升
 * 2. break在while中
 * 3. 变量不能是void
 * 4. 常量计算
 */
/* forward decl */
static struct type *gen_exp(struct ast_node *node);
static void gen_if(struct ast_node *node);
static void gen_for(struct ast_node *node);
static void gen_stmt(struct ast_node *node);
static void gen_block(struct ast_node *block);
static void gen_code(struct sym_tab *ptab);

static int in_while;
static struct type *func_ret;
static int call_count;
#define new_error_p(...) do { new_error(__VA_ARGS__); err++; } while(0)
static int err;

static struct type
*up_type(struct ast_node *node, struct type *lt, struct type *rt)
{
	if(!lt || !rt)
	{
		new_error_p(0,
			    node->first_line,
			    node->first_column,
			    "类型错误\n");
		return get_type(TYPE_VOID, 0, 0, NULL, NULL);
	}
	if(type_is_array(lt) || type_is_array(rt))
	{
		if(type_is_equal_byname(lt, rt))
			return lt;
	}
	else
	{
		if(type_is_equal_bystru(lt, rt))
			return lt;
	}
	if(type_is_int(lt) && type_is_float(rt))
		return rt;
	if(type_is_float(lt) && type_is_int(rt))
		return lt;
	if(type_is_int(lt) && type_is_bool(rt))
		return lt;
	if(type_is_bool(lt) && type_is_int(rt))
		return rt;
	if(type_is_float(lt) && type_is_bool(rt))
		return lt;
	if(type_is_bool(lt) && type_is_float(rt))
		return rt;
	new_error_p(0,
		  node->first_line,
		  node->first_column,
		  "类型错误\n");
	new_remark("类型分别为：");
	dump_type(lt, stderr);
	new_space();
	dump_type(rt, stderr);
	new_eol();
	return get_type(TYPE_VOID, 0, 0, NULL, NULL);
}
static struct type *gen_lval(struct ast_node *node, int w);
static struct type *gen_array(struct ast_node *node, int w)
{
	int i;
	struct ast_node *p;
	struct type *lt, *rt;
	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		if(i == 0)
			lt = gen_lval(p, w);
		else
		{
			rt = gen_exp(p);
			if(!type_is_int(rt))
				new_error_p(0,
					    p->first_line,
					    p->first_column,
					    "数组访问下标不是整数\n");
			if(type_is_array(lt))
			{
				while(lt->type == TYPE_TYPE)
					lt = lt->t2;
				lt = lt->t2;
			}
			else
				new_error_p(0,
					  node->first_line,
					  node->first_column,
					  "数组维数不正确\n");
			if(w && type_is_const(lt))
				new_error_p(0,
					    node->first_line,
					    node->first_column,
					    "向常量单元赋值\n");
		}
		i++;
	}
	return lt;
}

static struct type *gen_lval(struct ast_node *node, int w)
{
	struct sym_entry *e;
	if(node->type == NT_EXP && node->id == 'I')
	{
		e = node->pval;
		if(w && type_is_const(e->type))
			new_error_p(0,
				    node->first_line,
				    node->first_column,
				    "向常量单元赋值\n");
		return e->type;
	}
	if(node->type == NT_EXP && node->id == 'A')
	{
		return gen_array(node, w);
	}
	new_error_p(0,
		    node->first_line,
		    node->first_column,
		    "需要左值但不是左值\n");
	return get_type(TYPE_VOID, 0, 0, NULL, NULL);
}

static struct type *gen_exp(struct ast_node *node)
{
	struct ast_node *p, *l, *r;
	struct type *lt, *rt;
	if(node->type == NT_NUL) return get_type(TYPE_VOID, 0, 0, NULL, NULL);
	switch(node->id)
	{
	case 'i':
		return get_type(TYPE_INT, 0, 0, NULL, NULL);
	case 'f':
		return get_type(TYPE_FLOAT, 0, 0, NULL, NULL);
	case 'b':
		return get_type(TYPE_BOOL, 0, 0, NULL, NULL);
	case 'I':
		return gen_lval(node, 0);
	}
	get_lr_child(node, &l, &r);
	
	switch(node->id)
	{
	case EQ_OP: case NE_OP: case '<': case GE_OP: case '>': case LE_OP:
	case AND: case OR: case NOT:
		if(l)
			lt = gen_exp(l);
		if(r)
			rt = gen_exp(r);
		return get_type(TYPE_BOOL, 0, 0, NULL, NULL);
	case '%':
	case '|': case '&': case '~':
		lt = gen_exp(l);
		rt = gen_exp(r);
		if(!type_is_int(lt) || !type_is_int(rt))
			new_error_p(0,
				  node->first_line,
				  node->first_column,
				  "'%c'运算符需要整型\n", node->id);
		return lt;
	case '=':
		lt = gen_lval(l, 1);
		rt = gen_exp(r);
		up_type(node, lt, rt);
		if(type_is_func(rt) || type_is_void(rt))
			new_error_p(0,
				  node->first_line,
				  node->first_column,
				  "赋值非法\n");
		return lt;
	case 'F':
		call_count++;
		lt = gen_lval(l, 0);
		if(!type_is_func(lt))
			new_error_p(0,
				    l->first_line,
				    l->first_column,
				    "函数调用非法\n");
		if(!r)
		{
			if(type_is_void(lt->t1))
				lt = lt->t2;
			goto func_out;
		}
		list_for_each_entry(p, &r->chlds, sibling)
		{
			up_type(p, lt->t1, gen_exp(p));
			lt = lt->t2;
			if(lt == NULL)
			{
				new_error_p(0,
					    p->first_line,
					    p->first_column,
					    "函数调用参数多\n");
				return get_type(TYPE_VOID, 0, 0, NULL, NULL);
			}
		}
	func_out:
		if(type_is_func(lt))
			new_error_p(0,
				    node->first_line,
				    node->first_column,
				    "函数调用参数不足\n");
		return lt;
	case 'A':
		return gen_array(node, 0);
	default:
		lt = gen_exp(l);
		if(r)
		{
			rt = gen_exp(r);
			return up_type(node, lt, rt);
		}
		else
			return lt;
	}
}

static void gen_if(struct ast_node *node)
{
	struct ast_node *p;
	struct type *t;
	int i;
	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		switch(i)
		{
		case 0:
			t = gen_exp(p);
			if(!type_is_var(t) || type_is_array(t))
				new_error_p(0,
					  node->first_line,
					  node->first_column,
					  "if条件错\n");
			break;
		case 2:
		case 1:
			gen_stmt(p);
			break;
		}
		i++;
	}
}

static void gen_for(struct ast_node *node)
{
	int i;
	struct ast_node *p;
	struct type *t;
	int saved_in_while;
	saved_in_while = in_while;
	in_while = 1;

	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		switch(i)
		{
		case 0: /* init */
			gen_exp(p);
			break;
		case 1: /* cond */
			t = gen_exp(p);
			if(!type_is_var(t) || type_is_array(t))
				new_error_p(0,
					    node->first_line,
					    node->first_column,
					    "while条件错\n");
			break;
		case 2: /* stmt */
			gen_stmt(p);
			break;
		case 3: /* inc */
			gen_exp(p);
		}
		i++;
	}
	in_while = saved_in_while;
}

static void gen_read(struct ast_node *list)
{
	struct ast_node *exp;
	if(list)
		list_for_each_entry(exp, &list->chlds, sibling)
		{
			gen_lval(exp, 1);
		}
}

static void gen_stmt(struct ast_node *node)
{
	if(!node)
		return;
	switch(node->type)
	{
	case NT_EXP:
		gen_exp(node);
		break;
	case NT_IF:
		gen_if(node);
		break;
	case NT_FOR:
		gen_for(node);
		break;
	case NT_BLOCK:
		gen_block(node);
		break;
	case NT_BREAK:
		if(!in_while)
			new_error_p(0,
				  node->first_line,
				  node->first_column,
				  "break不在while块中\n");
	case NT_CONTINUE:
		if(!in_while)
			new_error_p(0,
				  node->first_line,
				  node->first_column,
				  "continue不在while块中\n");
		break;
	case NT_RETURN:
		if(node->id)
			up_type(node, gen_exp(get_child(node)), func_ret);
		break;
	case NT_READ:
		gen_read(get_child(node));
		break;
	}
}

static void gen_block(struct ast_node *block)
{
	struct ast_node *stmt;
	list_for_each_entry(stmt, &block->chlds, sibling)
	{
		gen_stmt(stmt);
	}
}

static void gen_code(struct sym_tab *ptab)
{
	struct type *ret_type;
	struct sym_entry *entry;
	list_for_each_entry(entry, &ptab->order, order)
		if(entry->kind == SYM_FUNC)
		{
			if(!entry->sfunc.defined)
				continue;
			ret_type = entry->type;
			while(ret_type->type == TYPE_FUNC)
				ret_type = ret_type->t2;
			entry->sfunc.ret_type = ret_type;
			func_ret = ret_type;
			call_count = 0;
			gen_code(entry->sfunc.sym);
			gen_block(entry->sfunc.stmts);
			entry->attr = !(call_count);
		}
		else if(entry->kind == SYM_VAR)
		{
			if(type_is_void(entry->type))
				new_error_p(0,
					    entry->sfunc.stmts->first_line,
					    entry->sfunc.stmts->first_column,
					    "变量不能是void类型\n");
		}
}

static void gen_code_all(struct sym_tab *ptab, char *name)
{
	err = 0;
	in_while = 0;
	gen_code(ptab);
	if(err)
		new_error(1, 0, 0, "语义检查错\n");
}

struct gen_info check = {
	.name = "check",
	.gen_code = gen_code_all,
	.info = "语义检查"
};
