#ifndef _DEFS_H_
#define _DEFS_H_

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#define TRUE	1
#define FALSE	0
#define _INT	0
#define _FLOAT	1
#define _BOOL	2
#define _OPC	3
#define _VOID	4

enum fct
{
	lit=0, opr, lod, sto, cal, Int, jmp, jpc, lar, sar, jpe ,init       // functions
};


enum opc
{
	ret=0, neg, add, minuss, mult, divv, mod, andand, oror, eq, neq, lt, lte, gt, gte, readd, writee,writes, notnot
};

typedef struct
{
	union 
	{
		double d;
		int i;
		bool b;
		enum opc op;
	}v;// displacement address
	int t;
}stack;

typedef struct
{
	enum fct f;// function code
	int l;// level
	union 
	{
		double d;
		int i;
		bool b;
		enum opc op;
	}v;// displacement address
	int t;
} __attribute__((packed)) instruction;


#endif
