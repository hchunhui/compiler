#ifndef _PL0_H_
#define _PL0_H_

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
#define period     0x40000
#define becomes    0x80000
#define beginsym   0x100000
#define endsym     0x200000
#define ifsym      0x400000
#define thensym    0x800000
#define whilesym   0x1000000
#define dosym      0x2000000
#define callsym    0x4000000
#define constsym   0x8000000
#define varsym     0x10000000
#define procsym    0x20000000

/* lexer */
#define error myerror
void getsym();
void lexer_setup();

/* parser */
void parser();

/* interpreter */
void listcode(long cx0);
void interpret();

#endif /* _PL0_H_ */
