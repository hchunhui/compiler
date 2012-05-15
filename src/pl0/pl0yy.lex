%{
#include <stdlib.h>
#include "pl0.h"
#include "glo.h"
#define retpr(sym) do{printf("%10s %s\n", #sym, yytext);return sym;}while(0)
%}

/* try1: \/\*(\/)*([^\/]|\*|([^\*]\/))*\*\/  fail: accept "/ * * / * * /"  */
/* try2: \/\*([^\*]|(\*[^\/]))*(\*)?\*\/     fail: accept "/ * * * / * /"  */
%%
\/\*[^\*]*\*(([^\*\/][^\*]*)?\*)*\/ {
	printf("comment:\n");
	printf("%s\n", yytext);
}

begin retpr(beginsym);
call  retpr(callsym);
const retpr(constsym);
do    retpr(dosym);
end   retpr(endsym);
if    retpr(ifsym);
odd   retpr(oddsym);
procedure retpr(procsym);
then  retpr(thensym);
var   retpr(varsym);
while retpr(whilesym);

[a-zA-Z][a-zA-Z0-9]* {strcpy(id, yytext);retpr(ident);}
[1-9][0-9]*|0  {num = strtol(yytext  , NULL, 10);retpr(number);}
0[xX][0-9a-f]+ {num = strtol(yytext+2, NULL, 16);retpr(number);}
0[0-7]+        {num = strtol(yytext+1, NULL,  8);retpr(number);}

":=" retpr(becomes);
"<>" retpr(neq);
"<=" retpr(leq);
"<"  retpr(lss);
">=" retpr(geq);
">"  retpr(gtr);

"+" retpr(plus);
"-" retpr(minus);
"*" retpr(times);
"/" retpr(slash);
"(" retpr(lparen);
")" retpr(rparen);
"=" retpr(eql);
"," retpr(comma);
"." retpr(period);
";" retpr(semicolon);
[ \t]+ {}
\n {printf ("%10s cx = %d\n", "** newline", cx);}
.  {printf("error\n");return 0;}
%%
void myerror(long n)
{
    printf(" **** error no.%2ld\n",n);
    err++;
}

void getsym()
{
	sym = yylex();
}

int yywrap()
{
	printf("************************************\n"
	       "      program incomplete\n"
	       "************************************\n");
	exit(1);
}

void lexer_setup()
{
	yyin = infile;
}
