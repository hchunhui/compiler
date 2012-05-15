#ifndef _PL0_INST_H_
#define _PL0_INST_H_

enum fct {
    lit = 0, opr, lod, sto, cal, Int, jmp, jpc         // functions
};
static const char * const mnemonic[] = {
	"lit",
	"opr",
	"lod",
	"sto",
	"cal",
	"int",
	"jmp",
	"jpc",
};

typedef struct{
    enum fct f;		// function code
    long l; 		// level
    long a; 		// displacement address
} instruction;
/*  lit 0, a : load constant a
    opr 0, a : execute operation a
    lod l, a : load variable l, a
    sto l, a : store variable l, a
    cal l, a : call procedure a at level l
    Int 0, a : increment t-register by a
    jmp 0, a : jump to a
    jpc 0, a : jump conditional to a       */

#endif /* _PL0_INST_H_ */
