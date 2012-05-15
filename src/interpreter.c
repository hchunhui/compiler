#include <stdio.h>
#include "glo.h"

#define stacksize 50000
static long s[stacksize];	// datastore

/* list code generated for this block */
void listcode(long cx0)
{
	long i;
	for(i=cx0; i<=cx-1; i++)
		fprintf(stderr,
			"%10ld%5s%3ld%5ld\n",
			i,
			mnemonic[code[i].f],
			code[i].l,
			code[i].a);
}

long base(long b, long l)
{
    long b1;

    b1=b;
    while (l>0){	// find base l levels down
	b1=s[b1]; l=l-1;
    }
    return b1;
}

void interpret()
{
    long p,b,t;		// program-, base-, topstack-registers
    instruction i;	// instruction register

    printf("start PL/0\n");
    t=0; b=1; p=0;
    s[1]=0; s[2]=0; s[3]=0;
    do{
	i=code[p]; p=p+1;
	switch(i.f){
	    case lit:
		t=t+1; s[t]=i.a;
		break;
	    case opr:
		switch(i.a){ 	// operator
		    case 0:	// return
			t=b-1; p=s[t+3]; b=s[t+2];
			break;
		    case 1:
			s[t]=-s[t];
			break;
		    case 2:
			t=t-1; s[t]=s[t]+s[t+1];
			break;
		    case 3:
			t=t-1; s[t]=s[t]-s[t+1];
			break;
		    case 4:
			t=t-1; s[t]=s[t]*s[t+1];
			break;
		    case 5:
			t=t-1; s[t]=s[t]/s[t+1];
			break;
		    case 6:
			s[t]=s[t]%2;
			break;
		    case 8:
			t=t-1; s[t]=(s[t]==s[t+1]);
			break;
		    case 9:
			t=t-1; s[t]=(s[t]!=s[t+1]);
			break;
		    case 10:
			t=t-1; s[t]=(s[t]<s[t+1]);
			break;
		    case 11:
			t=t-1; s[t]=(s[t]>=s[t+1]);
			break;
		    case 12:
			t=t-1; s[t]=(s[t]>s[t+1]);
			break;
		    case 13:
			t=t-1; s[t]=(s[t]<=s[t+1]);
		}
		break;
	    case lod:
		t=t+1; s[t]=s[base(b,i.l)+i.a];
		break;
	    case sto:
		s[base(b,i.l)+i.a]=s[t]; printf("%10ld\n", s[t]); t=t-1;
		break;
	    case cal:		// generate new block mark
		s[t+1]=base(b,i.l); s[t+2]=b; s[t+3]=p;
		b=t+1; p=i.a;
		break;
	    case Int:
		t=t+i.a;
		break;
	    case jmp:
		p=i.a;
		break;
	    case jpc:
		if(s[t]==0){
		    p=i.a;
		}
		t=t-1;
	}
    }while(p!=0);
    printf("end PL/0\n");
}
