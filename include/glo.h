#ifndef _GLO_H_
#define _GLO_H_
#ifndef EXTERN
#define EXTERN extern
#endif
#include <stdio.h>
#include "pl0_inst.h"

#define txmax      100            // length of identifier table
#define nmax       14             // max. no. of digits in numbers
#define al         10             // length of identifiers
#define amax       2047           // maximum address
#define levmax     3              // maximum depth of block nesting
#define cxmax      2000           // size of code array



EXTERN unsigned long sym;     // last symbol read
EXTERN char id[al+1];         // last identifier read
EXTERN long num;              // last number read
EXTERN long err;
EXTERN long cx;               // code allocation index

EXTERN instruction code[cxmax+1];

EXTERN FILE *infile;


#endif /* _GLO_H_ */
