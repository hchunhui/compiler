%{
#include <stdio.h>
#include "c1.tab.h"
#include "sym_tab.h"
#include "type.h"
#include "error.h"

extern struct sym_tab *symtab;
char *strs[256];
int strs_count = 0;
#define ret(x) yylval.ival=x; return x

int yycolumn = 0;

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
    yycolumn += yyleng;
%}

%option yylineno

D	   [0-9]
L	   [a-zA-Z_]
H	   [a-fA-F0-9]

%%
\/\*[^\*]*\*(([^\*\/][^\*]*)?\*)*\/ {  }

"typedef"		{  return(TYPEDEF); }

"if"			{  return(IF); }
"else"			{  return(ELSE); }
"while"			{  return(WHILE); }
"break"			{  return(BREAK); }
"return"		{  return(RETURN); }
"for"			{  return(FOR); }
"continue"		{  return(CONTINUE); }
"do"                    {  return(DO); }
"const"			{  return(CONST); }
"auto"                  { return(AUTO);  }
"read"			{  return(READ); }
"write"			{  return(WRITE); }
\"[^\"]*\"			{
	if(strs_count >= 256)
	{
		new_error(1, 0, 0, "too many strs\n");
	}
	yylval.ival = strs_count;
	strs[strs_count] = strdup(yytext+1);
	strs[strs_count][yyleng-2] = 0;
	strs_count++;
	return(STRING);
}

"true"			{
	yylval.ival = 1;
	return(BNUMBER);
}

"false"			{
	yylval.ival = 0;
	return(BNUMBER);
}

{L}({L}|{D})*		{
	struct sym_entry *e;
	e = symtab_lookup(symtab, yytext, 1);
	if(e && e->kind == SYM_TYPE)
	{
		yylval.tptr = e->type;
		return(ATYPE);
	}
	yylval.name = strdup(yytext);
	return(IDENTIFIER);
   }

0[xX]{H}+		{
	
	yylval.ival = strtol(yytext, NULL, 16);
	return(NUMBER);
	}
0{D}+			{
	
	yylval.ival = strtol(yytext, NULL, 8);
	return(NUMBER);
    }
{D}+			{
	
	yylval.ival = strtol(yytext, NULL, 10);
	return(NUMBER);
   }

{D}+\.{D}+		{
	yylval.fval = strtof(yytext, NULL);
	return(FNUMBER);
	 }

"&"			{  ret('&'); }
"|"			{  ret('|'); }
"~"			{  ret('~'); }


"!"			{  ret(NOT); }
"&&"			{  ret(AND); }
"||"			{  ret(OR); }
"<="			{  ret(LE_OP); }
">="			{  ret(GE_OP); }
"=="			{  ret(EQ_OP); }
"!="			{  ret(NE_OP); }
"<"			{  ret('<'); }
">"			{  ret('>'); }


"-"			{  ret('-'); }
"+"			{  ret('+'); }
"*"			{  ret('*'); }
"/"			{  ret('/'); }
"%"			{  ret('%'); }
"("			{  ret('('); }
")"			{  ret(')'); }


";"			{  ret(';'); }
"{"			{  ret('{'); }
"}"			{  ret('}'); }
"["			{  ret('['); }
"]"			{  ret(']'); }
","			{  ret(','); }
"="			{  ret('='); }

[ \v\f]			{  }
\t			{ yycolumn += 8 - yycolumn%8; }
[\n\r]			{yycolumn = 0;}
.			{ printf("unmatched: %s\n", yytext); return 1; }

%%

int yywrap(void)
{
	return 1;
}

void yyerror(char const *s)
{
	new_error(1,
		  yylloc.first_line,
		  yylloc.first_column,
		  "%s\n", s);
}
