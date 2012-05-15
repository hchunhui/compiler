%{
#include <stdarg.h>
#include "file.h"
#include "ast_node.h"
#include "sym_tab.h"
#include "gen.h"
struct sym_tab *symtab;

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

struct ast_node *mkleafi(int type, int ival)
{
	struct ast_node *ptr;
	ptr = ast_node_new(type, 0);
	ptr->ival = ival;
	return ptr;
}

struct ast_node *mkleafp(int type, void *pval)
{
	struct ast_node *ptr;
	ptr = ast_node_new(type, 0);
	ptr->pval = pval;
	return ptr;
}

void setloc(struct ast_node *node, int line, int col)
{
	node->first_line = line;
	node->first_column = col;
}

%}

%union {
	int ival;
	char *name;
	struct ast_node *node;
	struct sym_tab *symt;
}

%locations
%token IDENTIFIER NUMBER
%token LE_OP GE_OP EQ_OP NE_OP

%token INT VOID
%token IF WHILE

%start program

%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%type <ival> NUMBER
%type <name> IDENTIFIER funcdecl1
%type <ival> '+' '-' '*' '/'
%type <ival> '%' '<' '>' LE_OP GE_OP NE_OP EQ_OP relop
%type <ival> '='
%type <ival> INT VOID IF WHILE
%type <node> stmts stmt exp cond
%destructor { free($$); } IDENTIFIER
%error-verbose
/*%define parse.lac full*/
%%

program
	: decl
	;
decl
	: decl vardecl
	| decl funcdecl
	| /* E */
	;
vdecl
	: vdecl vardecl
	| /* E */
	{
		symtab = symtab_new(symtab);
	}
	;
funcdecl
	: funcdecl1 vdecl stmts '}'
	{
		symtab_modify_func(symtab->uplink, $1, $3, symtab);
		free($1);
		symtab = symtab->uplink;
	}
	;
funcdecl1
	: VOID IDENTIFIER '(' ')' '{'
	{
		symtab_enter_func(symtab, $2);
		$$ = $2;
	}
	;
vardecl
	: INT varlist ';'
	;
varlist
	: vardef
	| varlist ',' vardef
	;
vardef
	: IDENTIFIER
	{
		symtab_enter_var(symtab, $1);
		free($1);
	}
	| IDENTIFIER '=' NUMBER
	{
		symtab_enter_const(symtab, $1, $3);
		free($1);
	}
	;
relop
	: '>'
	| '<'
	| LE_OP
	| GE_OP
	| EQ_OP
	| NE_OP
	;
exp
	: NUMBER
	{
		$$ = mknode(NT_EXP, 'N',
			    mkleafi(NT_NUMBER, $1),
			    NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| IDENTIFIER
	{
		$$ = mknode(NT_EXP, 'I',
			    mkleafp(NT_IDENT,
				    symtab_lookup(symtab, $1)),
			    NULL);
		free($1);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp '+' exp
	{
		$$ = mknode(NT_EXP, $2, $1, $3, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp '-' exp
	{
		$$ = mknode(NT_EXP, $2, $1, $3, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp '*' exp
	{
		$$ = mknode(NT_EXP, $2, $1, $3, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp '/' exp
	{
		$$ = mknode(NT_EXP, $2, $1, $3, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| '-' exp %prec UMINUS
	{
		$$ = mknode(NT_EXP, UMINUS, $2, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| '(' exp ')'
	{
		$$ = $2;
		setloc($$, @$.first_line, @$.first_column);
	}
	;
cond
	: exp relop exp
	{
		$$ = mknode(NT_COND, $2, $1, $3, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| exp '%' NUMBER
	{
		if($3 != 2) printf("error: %\n");
		$$ = mknode(NT_COND,
			    $2,
			    $1,
			    NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	;
stmt
	: IDENTIFIER '=' exp ';'
	{
		$$ = mknode(NT_ASSIGN,
			    $2,
			    mkleafp(NT_IDENT,
				    symtab_lookup(symtab, $1)),
			    $3,
			    NULL);
		free($1);
		setloc($$, @$.first_line, @$.first_column);
	}
	| IDENTIFIER '(' ')' ';'
	{
		$$ = mknode(NT_CALL,
			    0,
			    mkleafp(NT_IDENT,
				    symtab_lookup(symtab, $1)),
			    NULL);
		free($1);
		setloc($$, @$.first_line, @$.first_column);
	}
	| '{' stmts '}'
	{
		$$ = $2;
		setloc($$, @$.first_line, @$.first_column);
	}
	| IF '(' cond ')' stmt
	{
		$$ = mknode(NT_IF, 0, $3, $5, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| WHILE '(' cond ')' stmt
	{
		$$ = mknode(NT_WHILE, 0, $3, $5, NULL);
		setloc($$, @$.first_line, @$.first_column);
	}
	| ';'
	{
		$$ = NULL;
	}
	| error ';'
	{
		$$ = NULL;
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

int parse()
{
	int ret;
	//yydebug = 1;
	if(ret = yyparse())
		fprintf(stderr, "\nFAIL\n");
	else
		fprintf(stderr, "\nPASS\n");
	return ret;
}
