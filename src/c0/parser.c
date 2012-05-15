#include <stdlib.h>
#include <string.h>
#include "c0.h"
#include "glo.h"

/* decl begin symbol set */
static const unsigned long declbegsys = varsym|procsym|nul;
/* statement begin symbol set */
static const unsigned long statbegsys = ident|beginsym|ifsym|whilesym|semicolon|nul;
/* factor begin symbol set */
static const unsigned long facbegsys  = ident|number|lparen;

/* the following variables for block */
static long dx;		/* data allocation index */
static long lev;	/* current depth of block nesting */
static long tx;		/* current table index */

enum object {
	constant, variable, proc
};
static struct{
	char name[al+1];
	enum object kind;
	long val;
	long level;
	long addr;
}table[txmax+1];

void gen(enum fct x, long y, long z)
{
	if(cx>cxmax){
		printf("program too long\n");
		exit(1);
	}
	code[cx].f=x; code[cx].l=y; code[cx].a=z;
	cx=cx+1;
}

/* error recover */
int test(unsigned long s1, unsigned long s2)
{
	if(!(sym & s1)) {     /* sym /E s1 */
		miss_error(0, s1);
		while(1) {
			if(sym & s2)   /* sym E s2 */
				return 0;
			getsym();
			if(sym & s1)   /* sym E s1 */
				break;
		}
	}
	getsym();
	return 1;
}

/* enter object into table */
void enter(enum object k)
{
	tx=tx+1;
	strcpy(table[tx].name,id);
	/*printf("enter: #%s#\n", id);*/
	table[tx].kind=k;
	switch(k){
	case constant:
		if(num>amax){
			error(1);
			num = 0;
		}
		table[tx].val=num;
		break;
	case variable:
		table[tx].level=lev; table[tx].addr=dx; dx=dx+1;
		break;
	case proc:
		table[tx].level=lev;
		break;
	}
}

/* find identifier id in table */
long position(char* id)
{
	long i;

	strcpy(table[0].name,id);
	i=tx;
	while(strcmp(table[i].name,id)!=0){
		i=i-1;
	}
	return i;
}

/*************** parse begin ***************/
void vardeclaration();
void factor();
void term();
void expression();
void term();
void expression();
void condition();
void statement();
void statements();
void block();

void vardeclaration()
{
	if(sym==ident) {
		getsym();
		if(sym==becomes) {
			getsym();
			if(sym==number) {
				enter(constant); getsym();
			}
		}
		else if(sym==comma || sym==semicolon) {
			enter(variable);
		} else {
			error(2);
		}
	} else {
		test(ident, declbegsys|semicolon);
	}
}

void factor()
{
	long i;
	while(sym & facbegsys){
		if(sym==ident){
			i=position(id);
			if(i==0){
				error(3);
			}else{
				switch(table[i].kind){
				case constant:
					gen(lit,0,table[i].val);
					break;
				case variable:
					gen(lod,lev-table[i].level,table[i].addr);
					break;
				case proc:
					error(9);
					break;
				}
			}
			getsym();
		}else if(sym==number){
			if(num>amax){
				error(1); num=0;
			}
			gen(lit,0,num);
			getsym();
		}else if(sym==lparen){
			getsym();
			expression();
			test(rparen, statbegsys);
		}else
			test(facbegsys, statbegsys);
	}
}

void term()
{
	unsigned long mulop;

	factor();
	while(sym==times||sym==slash){
		mulop=sym; getsym();
		factor();
		if(mulop==times){
			gen(opr,0,4);
		}else{
			gen(opr,0,5);
		}
	}
}

void expression()
{
	unsigned long addop;

	if(sym==plus||sym==minus){
		addop=sym; getsym();
		term();
		if(addop==minus){
			gen(opr,0,1);
		}
	}else{
		term();
	}
	while(sym==plus||sym==minus){
		addop=sym; getsym();
		term();
		if(addop==plus){
			gen(opr,0,2);
		}else{
			gen(opr,0,3);
		}
	}
}

void condition()
{
	unsigned long relop;

	expression();
	relop = sym;
	if(test(eql|neq|lss|gtr|leq|geq|oddsym, rparen|statbegsys))
	{    
		if(relop == oddsym){
			if(sym == number && num==2)
				gen(opr,0,6);
			else
				error(4);
			getsym();
		}
		expression();
		switch(relop){
		case eql:
			gen(opr,0,8);
			break;
		case neq:
			gen(opr,0,9);
			break;
		case lss:
			gen(opr,0,10);
			break;
		case geq:
			gen(opr,0,11);
			break;
		case gtr:
			gen(opr,0,12);
			break;
		case leq:
			gen(opr,0,13);
			break;
		}
	}
}

void statement()
{
	long i,cx1,cx2;
	if(sym==ident) {
		i=position(id);
		if(i==0) {
			error(3);
		}
		getsym();
		if(sym == becomes) {
			if(table[i].kind!=variable) {
				error(5); i=0;
			}
			getsym();
			expression();
			test(semicolon, statbegsys);
			if(i != 0)
				gen(sto,lev-table[i].level,table[i].addr);
		}else if(sym == lparen) {
			if(table[i].kind!=proc) {
				error(6); i=0;
			}
			getsym();
			if(test(rparen, statbegsys)) {
				if(i != 0)
					gen(cal,lev-table[i].level,table[i].addr);
				test(semicolon, statbegsys);
			}
		} else
			test(lparen|becomes, rparen|statbegsys);
	}else if(sym==ifsym) {
		getsym();
		if(test(lparen, rparen|statbegsys)) {
			condition();
			cx1=cx;	gen(jpc,0,0);
			test(rparen, statbegsys);
			statement();
			code[cx1].a=cx;
		}
	}else if(sym==beginsym) {
		getsym();
		statements();
		test(endsym, statbegsys);
	}else if(sym==whilesym){
		getsym();
		if(test(lparen, rparen|statbegsys))
		{
			cx1 = cx;
			condition();
			cx2 = cx;
			gen(jpc, 0, 0);
			test(rparen, statbegsys);
		}
		statement(); gen(jmp,0,cx1);
		code[cx2].a=cx;
	}else if(sym==semicolon)
		getsym();
}

void statements()
{
	while(sym&statbegsys) {
		if(sym == nul)
			break;
		statement();
	}
}

void block()
{
	int i;
	long tx0;		// initial table index
	long cx0; 		// initial code index
	long tx1;		// save current table index before processing nested procedures
	long dx1;		// save data allocation index

	dx=3; tx0=tx; table[tx].addr=cx; gen(jmp,0,0);
	if(lev>levmax){
		error(7);
	}
	do{
		while(sym == varsym) {
			getsym();
			vardeclaration();
			while(sym==comma) {
				getsym();
				vardeclaration();
			}
			test(semicolon, declbegsys);
		}
		while(sym == procsym) {
			/* C0 does not support nesting!! */
			if(lev > 0)
				error(8);
			getsym();
			if(sym == ident) {
				enter(proc);
				getsym();
			}else{
				test(ident, lparen|rparen|statbegsys);
			}
			test(lparen, rparen|statbegsys);
			test(rparen, statbegsys);
			test(beginsym, endsym|declbegsys);
			lev=lev+1; tx1=tx; dx1=dx;
			block();
			lev=lev-1; tx=tx1; dx=dx1;
			test(endsym, declbegsys);
		}
		if(sym == nul)
			break;
	}while(sym&declbegsys);
	code[table[tx0].addr].a=cx;
	table[tx0].addr=cx;		// start addr of code
	cx0=cx; gen(Int,0,dx);
	/* hide the last block */
	if(lev) {
		statements();
	} else {
		test(nul, declbegsys|statbegsys);
		strcpy(id, "main      ");
		i = position(id);
		if(i == 0) {
			error(3);
		} else if(table[i].kind != proc) {
			error(6);
		} else {
			gen(cal,lev-table[i].level,table[i].addr);
		}
	}
	gen(opr,0,0); // return
	listcode(cx0);
}
/*************** parse end ***************/

/* interface */
void parse()
{
	err = 0;
	cx = 0;
	getsym();
	lev = 0;
	tx = 0;
	block();
}
