%{
#include <stdarg.h>
#include "file.h"
#include "ast_node.h"
#include "sym_tab.h"
#include "type.h"
#include "gen.h"
#include "error.h"
struct sym_tab *symtab;
static struct type *curr_type;

#define new_error_p(...) do { new_error(__VA_ARGS__); err++; } while(0)
static int err;

struct ast_node *mknode(int type, int id, ...);
struct ast_node *mkleafi(int type, int id, int ival);
struct ast_node *mkleafb(int type, int id, int ival);
struct ast_node *mkleaff(int type, int id, float fval);
struct ast_node *mkleafp(int type, int id, void *pval);
static void setloc(struct ast_node *node, int line, int col);
static void
decl_sym(int row, int col, char *name, struct type *type, struct list_head *args, struct ast_node *iexp);
static struct sym_entry
*def_sym(int row, int col, char *name, struct type *type, struct list_head *args, struct ast_node *iexp);

#define exp_action2(ret, loc, op, arg1, arg2)   \
	do { \
	ret = mknode(NT_EXP, op, arg1, arg2, NULL); \
	setloc(ret, loc.first_line, loc.first_column); \
	}while(0)

#define exp_action1(ret, loc, op, arg)	   \
	do { \
	ret = mknode(NT_EXP, op, arg, NULL); \
	setloc(ret, loc.first_line, loc.first_column); \
	}while(0)
%}

%union {
	float fval;
	int ival;
	char *name;
	struct ast_node *node;
	struct type *tptr;
	struct list_head *lptr;
	struct {
		struct type *type;
		char *name;
	} type_name;
	struct {
		struct type *type;
		struct list_head *args;
		char *name;
		struct ast_node *exp;
	} decl_def;
	struct sym_entry *sym_entry;
}

%locations

%token IDENTIFIER NUMBER FNUMBER BNUMBER STRING

/* keywords */
%token TYPEDEF IF ELSE WHILE BREAK RETURN FOR CONTINUE
%token READ WRITE
%token CONST ATYPE

%token LE_OP GE_OP EQ_OP NE_OP NOT AND OR

%start program
%left ','
%right '='
%left OR
%left AND
%left '|'
%left '&'
%left NE_OP EQ_OP
%left '>' GE_OP '<' LE_OP
%left '+' '-'
%left '*' '/' '%'
%right PREFIX
%left  SUFFIX '(' '['
%nonassoc HIGH
%nonassoc IF_NO_ELSE
%nonassoc ELSE

%type <fval> FNUMBER
%type <ival> NUMBER BNUMBER
%type <name> IDENTIFIER
%type <ival> IF WHILE
%type <tptr> ATYPE
%type <node> stmts stmt exp exp_list exp_or_not
%type <sym_entry> xdef

%type <ival> STRING
%type <ival>  '='
%type <ival>  OR
%type <ival>  AND
%type <ival>  '|'
%type <ival>  '&'
%type <ival>  NE_OP EQ_OP
%type <ival>  '<' '>' LE_OP GE_OP
%type <ival>  '+' '-'
%type <ival>  '*' '/' '%'
%type <ival>  NOT '~'

%type <tptr> type
%type <decl_def> decl0 decl00 decl01
%type <type_name> arg
%type <lptr> arg_list
%destructor { free($$); } IDENTIFIER
%error-verbose
/*%define parse.lac full*/
%%

program : decl;	

decl
	: decl xdecl
	| decl xdef vdecl stmts '}' {
		$2->sfunc.stmts = $4;
		$2->sfunc.sym = symtab;
		symtab = symtab->uplink;
	}
	| decl TYPEDEF type decl00 ';'
	{
		struct sym_entry *tle, *tmp;
		symtab_enter_type(symtab, $4.name, $4.type);
		curr_type = $3;
		if($4.args)
		{
			list_for_each_entry_rev(tle, tmp, $4.args, list)
			{
				list_del(&tle->list);
				free(tle);
			}
			free($4.args);
		}
	}
	| /* E */
	;

xdecl	: decl_list ';' ;
xdef	: decl0 '{' {
	$$ = def_sym(@$.first_line,
		     @$.first_column,
		     $1.name, $1.type, $1.args, $1.exp); } ;

vdecl
	: vdecl xdecl
	| /* E */
	;

decl0
	: type decl01 { curr_type = $1; $$ = $2; } ;
decl_list
	: type decl_list0 { curr_type = $1; } ;
type
	: ATYPE { $$ = curr_type; curr_type = $1; } ;
decl_list0
	: decl01 {
		decl_sym(@$.first_line,
			 @$.first_column,
			 $1.name, $1.type, $1.args, $1.exp); }
	| decl_list0 ',' decl01 {
		decl_sym(@$.first_line,
			 @$.first_column,
			 $3.name, $3.type, $3.args, $3.exp); }
	;
decl01
	: decl00 { $$ = $1; $$.exp = NULL; }
	| decl00 '=' exp { $$ = $1; $$.exp = $3; }
	| decl00 '=' '{' exp_list '}' { $$ = $1; $$.exp = $4; }
	;

decl00
	: IDENTIFIER { $$.type = curr_type; $$.name = $1; $$.args = NULL;}
	| decl00 '[' NUMBER ']'{
		$$.type = array_type($1.type, $3);
		$$.name = $1.name;
		$$.args = $1.args;
	}
	| decl00 '(' arg_list ')' {
		$$.type = func_type($1.type, $3);
		$$.name = $1.name;
		$$.args = $3;
	}
	| decl00 '(' ')' {
		$$.type = func_type($1.type, NULL);
		$$.name = $1.name;
		$$.args = NULL;
	}
	;
arg_list
	: arg { $$ = type_list_start($1.type, $1.name); }
	| arg_list ',' arg { $$ = type_list_add($1, $3.type, $3.name); }
	;
arg
	: ATYPE IDENTIFIER { $$.type = $1; $$.name = $2; }
	| arg '[' NUMBER ']' { $$.type = array_type($1.type, $3); $$.name = $1.name; }
	| arg '(' arg_list ')' { $$.type = func_type($1.type, $3); $$.name = $1.name; }
	| arg '(' ')'
	{
		$$.type = func_type($1.type, NULL);
		$$.name = $1.name;
	}
	;

exp_list
	: exp
	{
		$$ = mknode(NT_EXPLIST, 0, $1, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp_list ',' exp
	{
		$$ = $1;
		ast_node_add_chld($1, $3);
	}
	;
exp
	: NUMBER
	{
		$$ = mkleafi(NT_EXP, 'i', $1);
		setloc($$, @$.first_line, @$.first_column);
	}
	| FNUMBER
	{
		$$ = mkleaff(NT_EXP, 'f', $1);
		setloc($$, @$.first_line, @$.first_column);
	}
	| BNUMBER
	{
		$$ = mkleafi(NT_EXP, 'b', $1);
		setloc($$, @$.first_line, @$.first_column);
	}
	| IDENTIFIER
	{
		struct sym_entry *e;
		e = symtab_lookup(symtab, $1, 1);
		if(!e)
			new_error_p(0,
				    @$.first_line,
				    @$.first_column,
				    "找不到符号 %s\n", $1);
		$$ = mkleafp(NT_EXP, 'I', e);
		free($1);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp '(' ')'
	{
		$$ = mknode(NT_EXP, 'F', $1, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp '(' exp_list ')'
	{
		$$ = mknode(NT_EXP, 'F', $1, $3, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp '[' exp ']' {
		if($1->type == NT_EXP &&
		   $1->id == 'A')
		{
			ast_node_add_chld($1, $3);
			$$ = $1;
		}
		else
			exp_action2($$, @$, 'A', $1, $3);
	  }
	| exp '=' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp OR exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp AND exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp '|' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp '&' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp NE_OP exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp EQ_OP exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp '>' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp '<' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp GE_OP exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp LE_OP exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp '+' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp '-' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp '*' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp '/' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| exp '%' exp	{ exp_action2($$, @$, $2, $1, $3); }
	| '+' exp %prec PREFIX	{ exp_action1($$, @$, $1, $2); }
	| '-' exp %prec PREFIX	{ exp_action1($$, @$, $1, $2); }
	| NOT exp %prec PREFIX	{ exp_action1($$, @$, $1, $2); }
	| '~' exp %prec PREFIX	{ exp_action1($$, @$, $1, $2); }
	| '(' exp ')' %prec HIGH
	{
		$$ = $2;
		setloc($$, @$.first_line, @$.first_column);
	}
	;

exp_or_not
	:/* E */
	{
		$$ = mknode(NT_NUL, 0, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp		{ $$ = $1; }
stmt
	: exp_or_not ';'{ $$ = $1; }
	| '{' stmts '}'	{ $$ = $2; }
	| error ';'
	{
		$$ = mknode(NT_NUL, 0, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| IF '(' exp ')' stmt %prec IF_NO_ELSE
	{
		$$ = mknode(NT_IF, 0, $3, $5, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| IF '(' exp ')' stmt ELSE stmt
	{
		$$ = mknode(NT_IF, 1, $3, $5, $7, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| WHILE '(' exp ')' stmt
	{
		struct ast_node *nul1, *nul2;
		nul1 = mknode(NT_NUL, 0, NULL);
		nul2 = mknode(NT_NUL, 0, NULL);
		$$ = mknode(NT_FOR, 0, nul1, $3, $5, nul2, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| FOR '(' exp_or_not ';' exp_or_not ';' exp_or_not ')' stmt
	{
		$$ = mknode(NT_FOR, 0, $3, $5, $9, $7, NULL);
		setloc($$, @$.first_line, @$.first_column);

	}
	| BREAK ';'
	{
		$$ = mknode(NT_BREAK, 0, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| CONTINUE ';'
	{
		$$ = mknode(NT_CONTINUE, 0, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| RETURN exp ';'
	{
		$$ = mknode(NT_RETURN, 1, $2, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| RETURN ';'
	{
		$$ = mknode(NT_RETURN, 0, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| WRITE '(' STRING ')' ';'
	{
		$$ = mknode(NT_WRITES, $3, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| WRITE '(' exp ')' ';'
	{
		$$ = mknode(NT_WRITEE, 0, $3, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| READ '(' exp_list ')' ';'
	{
		$$ = mknode(NT_READ, 0, $3, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	;
stmts
	: stmts stmt
	{
		ast_node_add_chld($1, $2);
		$$ = $1;
	}
	| /* E */
	{
		$$ = ast_node_new(NT_BLOCK, 0);
		setloc($$, @$.first_line, @$.first_column);
	}
	;
%%
void type_init();
int parse()
{
	int ret;
//	yydebug = 1;
	err = 0;
	ret = yyparse();
	ret = ret || err;
	if(ret)
		fprintf(stderr, "\nFAIL\n");
	else
		fprintf(stderr, "\nPASS\n");
	return ret;
}

struct ast_node *mknode(int type, int id, ...)
{
	struct ast_node *ptr, *pn;
	va_list args;
	va_start(args, id);
	ptr = ast_node_new(type, id);
	while(pn = va_arg(args, struct ast_node *))
		ast_node_add_chld(ptr, pn);
	va_end(args);
	return ptr;
}

struct ast_node *mkleafi(int type, int id, int ival)
{
	struct ast_node *ptr;
	ptr = ast_node_new(type, id);
	ptr->ival = ival;
	return ptr;
}

struct ast_node *mkleaff(int type, int id, float fval)
{
	struct ast_node *ptr;
	ptr = ast_node_new(type, id);
	ptr->fval = fval;
	return ptr;
}

struct ast_node *mkleafp(int type, int id, void *pval)
{
	struct ast_node *ptr;
	ptr = ast_node_new(type, id);
	ptr->pval = pval;
	return ptr;
}

static void setloc(struct ast_node *node, int line, int col)
{
	node->first_line = line;
	node->first_column = col;
}

static void decl_sym(int row,
		     int col,
		     char *name,
		     struct type *type,
		     struct list_head *args,
		     struct ast_node *iexp)
{
	struct sym_entry *e;
	struct type_list *tle, *tmp;
	e = symtab_lookup(symtab, name, 0);
	if(e)
	{
		if(!type_is_equal_byname(e->type, type))
			new_error_p(0,
				    row,
				    col,
				    "符号 %s 类型与之前声明不匹配\n", name);
		if(e->kind != SYM_FUNC)
			new_error_p(0,
				    row,
				    col,
				    "符号 %s 不是函数\n", name);
	}
	else
		e = symtab_enter(symtab, name, type);
	new_remark("sym %s, type ", name);
	dump_type(type, stderr);
	new_eol();
	free(name);

	if(iexp && e->kind == SYM_FUNC)
	{
		new_error_p(0,
			    row,
			    col,
			    "不能对函数初始化\n", name);
	}
	if(iexp && e->kind == SYM_VAR)
	{
		e->svar.iexp = iexp;
		if(type_is_array(e->type) && iexp->type != NT_EXPLIST ||
		   type_is_var(e->type) && !type_is_array(e->type)
		   && iexp->type != NT_EXP)
			new_error_p(0,
				    row,
				    col,
				    "初始化错误\n", name);
	}

	if(args)
	{
		list_for_each_entry_rev(tle, tmp, args, list)
		{
			list_del(&tle->list);
			free(tle);
		}
		free(args);
	}
}

static struct sym_entry
*def_sym(int row,
	 int col,
	 char *name,
	 struct type *type,
	 struct list_head *args,
	 struct ast_node *iexp)
{
	struct sym_entry *e, *ee;
	struct type_list *tle, *tmp;
	e = symtab_lookup(symtab, name, 0);
	if(e)
	{
		if(e->kind != SYM_FUNC)
			new_error_p(0,
				    row,
				    col,
				    "符号 %s 不是函数\n", name);
		else
		{
			if(!type_is_equal_byname(e->type, type))
				new_error_p(0,
					    row,
					    col,
					    "符号 %s 类型不匹配\n", name);
			if(e->sfunc.defined)
				new_error_p(0,
					    row,
					    col,
					    "符号 %s 重定义\n", name);
		}
	}
	else
		e = symtab_enter(symtab, name, type);
	if(iexp && e->kind == SYM_FUNC)
	{
		new_error_p(0,
			    row,
			    col,
			    "不能对函数初始化\n", name);
	}
	if(e->kind == SYM_FUNC)
		e->sfunc.defined = 1;
	new_remark("sym %s, type ", name);
	dump_type(type, stderr);
	new_eol();
	free(name);
	
	symtab = symtab_new(symtab);
	if(args)
	{
		list_for_each_entry_rev(tle, tmp, args, list)
		{
			new_remark("args: %s  ", tle->name);
			dump_type(tle->type, stderr);
			new_eol();

			ee = symtab_enter(symtab, tle->name, tle->type);
			ee->svar.is_param = 1;
			
				
			list_del(&tle->list);
			free(tle);
		}
		free(args);		
	}
	return e;
}
