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
static struct type *gen_leaf(struct ast_node *leaf);
static struct type *gen_exp(struct ast_node *node);
static void gen_if(struct ast_node *node);
static void gen_while(struct ast_node *node);
static void gen_stmt(struct ast_node *node);
static void gen_block(struct ast_node *block);
static void gen_code(struct sym_tab *ptab);

static int in_while;
static struct type *func_ret;
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
		return get_type(TYPE_VOID, 0, NULL, NULL);
	}
	if(lt == rt)
		return lt;
	if(lt->type == TYPE_INT && rt->type == TYPE_FLOAT)
		return rt;
	if(lt->type == TYPE_FLOAT && rt->type == TYPE_INT)
		return lt;
	if(lt->type == TYPE_INT && rt->type == TYPE_BOOL)
		return lt;
	if(lt->type == TYPE_BOOL && rt->type == TYPE_INT)
		return rt;
	if(lt->type == TYPE_FLOAT && rt->type == TYPE_BOOL)
		return lt;
	if(lt->type == TYPE_BOOL && rt->type == TYPE_FLOAT)
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
	return get_type(TYPE_VOID, 0, NULL, NULL);
}
static struct type *gen_lval(struct ast_node *node);
static struct type *gen_array(struct ast_node *node)
{
	int i;
	struct ast_node *p;
	struct type *lt, *rt;
	i = 0;
	list_for_each_entry(p, &node->chlds, sibling)
	{
		if(i == 0)
			lt = gen_lval(p);
		else
		{
			rt = gen_exp(p);
			if(!type_is_int(rt))
				new_error_p(0,
					    p->first_line,
					    p->first_column,
					    "数组访问下标不是整数\n");
			if(type_is_array(lt))
				lt = lt->t2;
			else
				new_error_p(0,
					  node->first_line,
					  node->first_column,
					  "数组维数不正确\n");
		}
		i++;
	}
	if(type_is_array(lt))
		new_error_p(0,
			  node->first_line,
			  node->first_column,
			  "数组维数不够\n");
	return lt;
}

static struct type *gen_lval(struct ast_node *node)
{
	struct sym_entry *e;
	int check;
	if(node->type == NT_IDENT)
	{
		e = node->pval;
		return e->type;
	}
	if(node->type == NT_EXP && node->id == 'I')
	{
		e = get_child(node)->pval;
		return e->type;
	}
	if(node->type == NT_EXP && node->id == 'A')
	{
		return gen_array(node);
	}
	new_error_p(0,
		    node->first_line,
		    node->first_column,
		    "需要左值但不是左值\n");
	return get_type(TYPE_VOID, 0, NULL, NULL);
}

static struct type *gen_leaf(struct ast_node *node)
{
	struct sym_entry *entry;
	switch(node->type)
	{
	case NT_NUMBER:
		return get_type(node->id, 0, NULL, NULL);
	case NT_IDENT:
		return gen_lval(node);
	default:
		new_error_p(1,
			  node->first_line,
			  node->first_column,
			  "内部错误\n");
	}
}

static struct type *gen_exp(struct ast_node *node)
{
	int i;
	struct ast_node *p, *l, *r;
	struct type *lt, *rt;
	
	get_lr_child(node, &l, &r);
	switch(node->id)
	{
	case 'N': case 'I':
		return gen_leaf(get_child(node));
	case EQ_OP: case NE_OP: case '<': case GE_OP: case '>': case LE_OP:
	case AND: case OR: case NOT:
		return get_type(TYPE_BOOL, 0, NULL, NULL);
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
		lt = gen_lval(l);
		rt = gen_exp(r);
		up_type(node, lt, rt);
		if(type_is_func(rt) || type_is_void(rt))
			new_error_p(0,
				  node->first_line,
				  node->first_column,
				  "赋值非法\n");
		return lt;
	case 'F':
		lt = gen_lval(l);
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
				return get_type(TYPE_VOID, 0, NULL, NULL);
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
		return gen_array(node);
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

static void gen_while(struct ast_node *node)
{
	struct ast_node *exp, *stmt;
	struct sym_entry *e;
	struct type *t;
	int saved_in_while;
	saved_in_while = in_while;
	in_while = 1;
	get_lr_child(node, &exp, &stmt);
	t = gen_exp(exp);
	if(!type_is_var(t) || type_is_array(t))
		new_error_p(0,
			  node->first_line,
			  node->first_column,
			  "while条件错\n");
	gen_stmt(stmt);
	in_while = saved_in_while;
}

static void gen_read(struct ast_node *list)
{
	struct ast_node *exp;
	if(list)
		list_for_each_entry(exp, &list->chlds, sibling)
		{
			gen_lval(exp);
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
	case NT_WHILE:
		gen_while(node);
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
	struct sym_entry *entry;
	list_for_each_entry(entry, &ptab->order, order)
		if(type_is_func(entry->type))
		{
			if(!entry->sfunc.defined)
				continue;
			func_ret = entry->type;
			while(func_ret->type == TYPE_FUNC)
				func_ret = func_ret->t2;
			gen_code(entry->sfunc.sym);
			gen_block(entry->sfunc.stmts);
		}
		else
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
