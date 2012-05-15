#include <stdlib.h>
#include <string.h>
#include "c0.h"
#include "glo.h"
#define norw       4             // no. of reserved words
static const char * const word[norw] = {
	"if        ",
	"int       ",
	"void      ",
	"while     ",
};
static const unsigned long wsym[norw] = {
	ifsym,
	varsym,
	procsym,
	whilesym,
};
static unsigned long ssym[256];
static char ch;               // last character read
static long cc;               // character count
static long ll;               // line length
static char a[al+1];
static long kk;
static char line[81];

static const char *symb[] = {
	"",
	"EOF",
	"ID",
	"NUMBER",
	"'+'",
	"'-'",
	"'*'",
	"'/'",
	"'%'",
	"'=='",
	"'!='",
	"'<'",
	"'<='",
	"'>'",
	"'>='",
	"'('",
	"')'",
	"','",
	"';'",
	"'='",
	"'{'",
	"'}'",
	"'if'",
	"'while'",
	"'int'",
	"'void'",
};

const char *errmsg[10] = {
	"miss %s token",
	"number is too large",
	"variable or constant declaration error",
	"can not found identifier '%s'",
	"only support '\%2' now",
	"can not assign a function or constant",
	"can not call a variable or constant",
	"too many levels",
	"C0 does not support nest function declaration",
	"function identifier is not a factor",
};

void miss_error(long n, unsigned long s)
{
	long i;
	int p, flag;
	char str[256];

	str[0] = 0;
	
	if(n > 9 || n < 0 || s == 0)
	{
		printf("baderr\n");
		exit(1);
	}
	
	printf(" ****");
	for (i=1; i<=cc-1; i++){
		printf(" ");
	}
	printf("^%2ld\n",n);

	p = 1;
	flag = 0;
	while(s)
	{
		if(s&1) {
			if(flag)
				strcat(str, ", ");
			strcat(str, symb[p]);
			flag++;
		}
		p++;
		s >>= 1;
	}
	printf("errmsg(%d): ", n);
	printf(errmsg[n], str);
	printf("\n");
	err++;
}

void myerror(long n)
{
    long i;

    if(n > 9 || n < 0)
    {
	    printf("baderr\n");
	    exit(1);
    }
    printf(" ****");
    for (i=1; i<=cc-1; i++){
	printf(" ");
    }
    printf("^%2ld\n",n);
    printf("errmsg(%d): ", n);
    printf(errmsg[n], id);
    printf("\n");
    err++;
}

void getch()
{
	if(feof(infile)) {
		ch = 0;
		return;
	}
	if(cc==ll){
		ll=0; cc=0;
		printf("%5ld ", cx);
		while(1)
		{
			ch = getc(infile);
			if(ch == '\n' || feof(infile))break;
			printf("%c",ch);
			ll=ll+1; line[ll]=ch;
		}
		printf("\n");
		ll=ll+1; line[ll]=' ';
	}
	cc=cc+1; ch=line[cc];
}

void getsym()
{
	long i,j,k;
	char last_ch;
	for(;;)
	{
		while(ch==' '||ch=='\t'){
			getch();
		}
		if(ch == '/') //comment
		{
			getch();
			if(ch == '*')
			{
				getch();
				do {
					last_ch = ch;
					getch();
				}while(last_ch != '*' || ch != '/');
				getch();
				/*printf("comment\n");*/
			}
			else
			{
				sym = ssym[(unsigned char)'/'];
				return;
			}
		}
		else break;
	}
	if(isalpha(ch) || ch == '_'){ 	// identified or reserved
		k=0;
		do{
			if(k<al){
				a[k]=ch; k=k+1;
			}
			getch();
		}while(isalpha(ch)||isdigit(ch) || ch=='_');
		if(k>=kk){
			kk=k;
		}else{
			do{
				kk=kk-1; a[kk]=' ';
			}while(k<kk);
		}
		strcpy(id,a); i=0; j=norw-1;
		do{
			k=(i+j)/2;
			if(strcmp(id,word[k])<=0){
				j=k-1;
			}
			if(strcmp(id,word[k])>=0){
				i=k+1;
			}
		}while(i<=j);
		if(i-1>j){
			sym=wsym[k];
		}else{
			sym=ident;
		}
	}else if(isdigit(ch)){ // number
		k=0; num=0; sym=number;
		if(ch == '0') {
			getch();
			if(ch == 'x' || ch == 'X') {
				getch();
				for(;;)
				{
					if(isdigit(ch))
						ch -= '0';
					else if('a' <= ch && 'f' >= ch)
						ch -= 'a' - 10;
					else if('A' <= ch && 'F' >= ch)
						ch -= 'A' - 10;
					else
						break;
					num = num*16 + ch;
					getch();
				}
			} else {
				for(;;)
				{
					if('0' <= ch && '7' >= ch)
						ch -= '0';
					else
						break;
					num = num*8 + ch;
					getch();
				}
			}
		} else {
			do{
				num=num*10+(ch-'0');
				k=k+1; getch();
			}while(isdigit(ch));
			if(k>nmax){
				error(31);
			}
		}
	}else if(ch=='='){
		getch();
		if(ch=='='){
			sym=eql; getch();
		}else{
			sym=becomes;
		}
	}else if(ch=='!'){
		getch();
		if(ch=='='){
			sym=neq; getch();
		}else{
			sym=nul;
		}
	}else if(ch=='<'){
		getch();
		if(ch=='='){
			sym=leq; getch();
		}else{
			sym=lss;
		}
	}else if(ch=='>'){
		getch();
		if(ch=='='){
			sym=geq; getch();
		}else{
			sym=gtr;
		}
	}else{
		sym=ssym[(unsigned char)ch]; getch();
	}
}

void lexer_setup()
{
	long i;
	for(i=0; i<256; i++){
		ssym[i]=nul;
	}
	ssym['+']=plus;
	ssym['-']=minus;
	ssym['*']=times;
	ssym['/']=slash;
	ssym['(']=lparen;
	ssym[')']=rparen;
	ssym[',']=comma;
	ssym[';']=semicolon;
	ssym['%']=oddsym;
	ssym['{']=beginsym;
	ssym['}']=endsym;
	ssym[0]=nul;
	ch = ' ';
	cc = 0;
	ll = 0;
	kk = al;
}
