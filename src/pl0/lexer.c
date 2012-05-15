#include <stdlib.h>
#include <string.h>
#include "pl0.h"
#include "glo.h"
#define norw       11             // no. of reserved words
static const char * const word[norw] = {
	"begin     ",
	"call      ",
	"const     ",
	"do        ",
	"end       ",
	"if        ",
	"odd       ",
	"procedure ",
	"then      ",
	"var       ",
	"while     ",
};
static const unsigned long wsym[norw] = {
	beginsym,
	callsym,
	constsym,
	dosym,
	endsym,
	ifsym,
	oddsym,
	procsym,
	thensym,
	varsym,
	whilesym,
};
static unsigned long ssym[256];
static char ch;               // last character read
static long cc;               // character count
static long ll;               // line length
static char a[al+1];
static long kk;
static char line[81];

const char *errmsg[33] = {
	NULL,
	"应为=而不是:=",
	"=后应为数",
	"标识符后应为=",
	"const,var,procedure 后应为标识符",
	"遗漏逗号或分号",
	"过程声明后的记号不正确",
	"应为语句",
	"分程序内的语句部分后的记号不正确",
	"应为句号",
	"语句之间漏分号",
	"标识符未声明",
	"不可向常量或过程赋值",
	"应为赋值运算符:=",
	"call 后应为标识符",
	"不可调用常量或变量",
	"应为 then",
	"应为分号或 end",
	"应为 do",
	"语句后的记号不正确",
	"应为关系运算符",
	"表达式内不可有过程标识符",
	"遗漏右括号",
	"因子后不可为此记号",
	"表达式不能以此记号开始",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"这个数太大",
	"这个常数或地址偏移太大",
	"程序嵌套层次太多",
};

void myerror(long n){
    long i;

    if(n > 32 || n < 1)
	    exit(1);
    printf(" ****");
    for (i=1; i<=cc-1; i++){
	printf(" ");
    }
    printf("^%2ld\n",n);
    printf("errmsg(%d): %s\n", n, errmsg[n]);
    err++;
}

void getch()
{
	if(cc==ll){
		ll=0; cc=0;
		printf("%5ld ", cx);
		while(((ch=getc(infile))!='\n') && (!feof(infile))){
			printf("%c",ch);
			ll=ll+1; line[ll]=ch;
		}
		printf("\n");
		if(feof(infile)){
			printf("************************************\n");
			printf("      program incomplete\n");
			printf("************************************\n");
			exit(1);
		}
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
				printf("comment\n");
			}
			else
			{
				sym = ssym[(unsigned char)'/'];
				return;
			}
		}
		else break;
	}
	if(isalpha(ch)){ 	// identified or reserved
		k=0;
		do{
			if(k<al){
				a[k]=ch; k=k+1;
			}
			getch();
		}while(isalpha(ch)||isdigit(ch));
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
	}else if(ch==':'){
		getch();
		if(ch=='='){
			sym=becomes; getch();
		}else{
			sym=nul;
		}
	}else if(ch=='<'){
		getch();
		if(ch=='='){
			sym=leq; getch();
		}else if(ch=='>'){
			sym=neq; getch();
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
	ssym['=']=eql;
	ssym[',']=comma;
	ssym['.']=period;
	ssym[';']=semicolon;
	ch = ' ';
	cc = 0;
	ll = 0;
	kk = al;
}
