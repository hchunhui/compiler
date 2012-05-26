#ifndef _EIR_INST_H_
#define _EIR_INST_H_

#define _INT	0
#define _FLOAT	1
#define _BOOL	2
#define _OPC	3
#define _VOID	4

/*  lit 0, a : load constant a
    opr 0, a : execute operation a
    lod l, a : load variable l, a
    sto l, a : store variable l, a
    cal l, a : call procedure a at level l
    Int 0, a : increment t-register by a
    jmp 0, a : jump to a
    jpc 0, a : jump conditional to a       */

enum fct
{
	lit=0, opr, lod, sto, cal, Int, jmp, jpc, lar, sar, jpe ,init
};


enum opc
{
	ret=0, neg, add, minuss, mult, divv, mod, andand, oror, eq, neq, lt, lte, gt, gte, readd, writee,writes, notnot
};

typedef struct
{
	enum fct f;// function code
	int l;// level
	union 
	{
		double d;
		int i;
		int b;
		enum opc op;
	}v;// displacement address
	int t;
} __attribute__((packed)) instruction;

#endif /* _EIR_INST_H_ */
