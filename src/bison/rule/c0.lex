%{
#include <stdio.h>
#include "c0.tab.h"

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

"int"			{  return(INT); }
"void"			{  return(VOID); }
"if"			{  return(IF); }
"while"			{  return(WHILE); }

{L}({L}|{D})*		{
	
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


"<="			{  ret(LE_OP); }
">="			{  ret(GE_OP); }
"=="			{  ret(EQ_OP); }
"!="			{  ret(NE_OP); }
";"			{  ret(';'); }
"{"			{  ret('{'); }
"}"			{  ret('}'); }
"<"			{  ret('<'); }
">"			{  ret('>'); }
","			{  ret(','); }
"="			{  ret('='); }
"("			{  ret('('); }
")"			{  ret(')'); }
"-"			{  ret('-'); }
"+"			{  ret('+'); }
"*"			{  ret('*'); }
"/"			{  ret('/'); }
"%"			{  ret('%'); }

[ \v\f]			{  }
\t			{ yycolumn += 8 - yycolumn%8; }
\n			{yycolumn = 0;}
.			{ printf("unmatched: %s\n", yytext); return 1; }

%%

int yywrap(void)
{
	return 1;
}

void yyerror(char const *s)
{
	fflush(stdout);
	printf("Error@(%d:%d): %s\n",
	       yylloc.first_line,
	       yylloc.first_column,
	       s);
	/*printf("\n%*s\n%*s\n",
	       yylloc.first_column,
	       "^",
	       yylloc.first_column,
	       s);*/
}
