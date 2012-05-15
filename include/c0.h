#ifndef _C0_H_
#define _C0_H_

#define nul	   0x1
#define ident      0x2
#define number     0x4
#define plus       0x8
#define minus      0x10
#define times      0x20
#define slash      0x40
#define oddsym     0x80
#define eql        0x100
#define neq        0x200
#define lss        0x400
#define leq        0x800
#define gtr        0x1000
#define geq        0x2000
#define lparen     0x4000
#define rparen     0x8000
#define comma      0x10000
#define semicolon  0x20000
#define becomes    0x40000
#define beginsym   0x80000
#define endsym     0x100000
#define ifsym      0x200000
#define whilesym   0x400000
#define varsym     0x800000
#define procsym    0x1000000

/* lexer */
#define error myerror
void getsym();
void lexer_setup();
void miss_error(long n, unsigned long s);

/* parser */
void parser();

/* interpreter */
void listcode(long cx0);
void interpret();

#endif /* _C0_H_ */
