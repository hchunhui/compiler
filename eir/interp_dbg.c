#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <vector>
#include <fstream>
#include "defs.h"
using namespace std;
char infilename[80];
char line[81];
InstructionList insvec;
vector<string> strs;
const char fctt[][5]=
{
	"lit", "opr", "lod", "sto", "cal", "Int", "jmp", "jpc", "lar", "sar", "jpe" ,"init"       // functions
};

const char opcc[][10]=
{
	"ret", "neg", "add", "minuss", "mult", "divv", "mod", "andand", "oror", "eq", "neq", "lt", "lte", "gt", "gte", "readd", "writee" , "writes", "notnot"
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
#define stacksize 5000
stack s[stacksize];
#define GETVAL(s) ((s).t==_BOOL?(s).v.b:((s).t==_INT?(s).v.i:(s).v.d))
#define GETVALP(s) ((s)->t==_BOOL?(s)->v.b:((s)->t==_INT?(s)->v.i:(s)->v.d))

int base(int b, int l)
{
	if(l==-1)
		return 0;
    while (l > 0) {	// find base l levels down
        b = s[b].v.i;
        l--;
    }
    return b;
}

void debug(int p, int b, int t)
{
  int i;
  printf("p=%d b=%d t=%d\n", p-1, b, t);
  for(i = 0; i < t+4; i++)
    {
      if(i == b)
	printf("b-> ");
      else
	printf("    ");
      switch(s[i].t)
	{
	case _INT:
	  printf("%d", s[i].v.i);
	  break;
	case _FLOAT:
	  printf("%f", s[i].v.d);
	  break;
	case _BOOL:
	  printf("%d", s[i].v.b);
	  break;
	}
      if(i == t)
	printf(" <-t\n");
      else
	printf("\n");
    }
      getchar();
}
void interpret(InstructionList &insv)
{
    long p, b, t;
    instruction *i;

	int size;
	int loc;
			

	cout<<"-----------------------------------program runs\n";
    t = 0;
    b = 1;
    p = 0;
	memset(s,0,sizeof(stack)*4);
    do {
        if(t > stacksize) {
            cout<<"stack overflow\n";
            exit(0);
        }
        i = insv[p++];
	debug(p,b,t);
        switch(i->f) {
        case lit:
			t++;
			if(i->t==_INT)
				s[t].v.i = GETVALP(i);
			else if(i->t==_FLOAT)
				s[t].v.d = GETVALP(i);
			else if(i->t==_BOOL)
				s[t].v.b = GETVALP(i);
            s[t].t=i->t;
            break;
        case opr:
            switch(i->v.op) { 	// operator
            case ret:	// return
                t = b - 1;
                p = s[t + 3].v.i;
                b = s[t + 2].v.i;
				s[t+1-i->l]=s[t+4];
                t = t+1 - i->l;
                break;
            case neg:
				if(s[t].t==_INT)
					s[t].v.i = -GETVAL(s[t]);
				else if(s[t].t==_FLOAT)
					s[t].v.d = -GETVAL(s[t]);
				else if(s[t].t==_BOOL)
					s[t].v.b = -GETVAL(s[t]);
				else
					cout<<"type error"<<endl;
                break;
            case add:
                t--;
				if(s[t].t==_FLOAT||s[t+1].t==_FLOAT)
				{
					s[t].v.d=GETVAL(s[t])+GETVAL(s[t+1]);
					s[t].t=_FLOAT;
				}
				else
				{
					s[t].v.i=GETVAL(s[t])+GETVAL(s[t+1]);
					s[t].t=_INT;
				}
                break;
            case minuss:
                t--;
				if(s[t].t==_FLOAT||s[t+1].t==_FLOAT)
				{
					s[t].v.d=GETVAL(s[t])-GETVAL(s[t+1]);
					s[t].t=_FLOAT;
				}
				else
				{
					s[t].v.i=GETVAL(s[t])-GETVAL(s[t+1]);
					s[t].t=_INT;
				}
                break;
            case mult:
                t--;
				if(s[t].t==_FLOAT||s[t+1].t==_FLOAT)
				{
					s[t].v.d=GETVAL(s[t])*GETVAL(s[t+1]);
					s[t].t=_FLOAT;
				}
				else
				{
					s[t].v.i=GETVAL(s[t])*GETVAL(s[t+1]);
					s[t].t=_INT;
				}
                break;
            case divv:     //slash
                t--;
				if(GETVAL(s[t+1])==0)
				{
					cout<<"divide 0\n";
					exit(0);
				}
				if(s[t].t==_FLOAT||s[t+1].t==_FLOAT)
				{
					s[t].v.d=GETVAL(s[t])/GETVAL(s[t+1]);
					s[t].t=_FLOAT;
				}
				else
				{
					s[t].v.i=GETVAL(s[t])/GETVAL(s[t+1]);
					s[t].t=_INT;
				}
                break;
            case mod:
                t--;
				if(s[t].t!=_INT||s[t+1].t!=_INT)
				{
					cout<<"mod float\n";
					exit(0);
				}
				s[t].v.i=s[t].v.i%s[t+1].v.i;
				s[t].t=_INT;
                break;
            case eq:
                t--;
                s[t].v.b = (GETVAL(s[t]) == GETVAL(s[t+1]));
				s[t].t=_BOOL;
                break;
            case neq:
                t--;
                s[t].v.b = (GETVAL(s[t]) != GETVAL(s[t+1]));
				s[t].t=_BOOL;
                break;
            case lt:
                t--;
                s[t].v.b = (GETVAL(s[t]) < GETVAL(s[t+1]));
				s[t].t=_BOOL;
                break;
            case gte:
                t--;
                s[t].v.b = (GETVAL(s[t]) >= GETVAL(s[t+1]));
				s[t].t=_BOOL;
                break;
            case gt:
                t--;
                s[t].v.b = (GETVAL(s[t]) > GETVAL(s[t+1]));
				s[t].t=_BOOL;
                break;
            case lte:
                t--;
                s[t].v.b = (GETVAL(s[t]) <= GETVAL(s[t+1]));
				s[t].t=_BOOL;
                break;
            case readd:   // read ins.
				//TODO read
				t++;
				s[t].t=i->l;
				if(i->l==_INT)
					cin>>s[t].v.i;
				else if(i->l==_FLOAT)
					cin>>s[t].v.d;
				else if(i->l==_BOOL)
					cin>>s[t].v.b;
                break;
            case writee:    // write ins.
				if(s[t].t==_BOOL)
				{
					if(s[t].v.b)
						cout<<"true";
					else
						cout<<"false";
				}
				else
					cout<<GETVAL(s[t]);
                t--;
                break;
            case writes:    // write ins.
                cout<<strs[s[t].v.i];
                t--;
                break;
            case andand:    // and
                t--;
                s[t].v.b = (GETVAL(s[t]) && GETVAL(s[t+1]));
				s[t].t=_BOOL;
                break;
            case oror:    // or
                t--;
                s[t].v.b = (GETVAL(s[t]) || GETVAL(s[t+1]));
				s[t].t=_BOOL;
                break;
            case notnot:    // not
                s[t].v.b = !GETVAL(s[t]) ;
				s[t].t=_BOOL;
                break;
			default:
				cout<<"instruction not supportted\n";
            }
            break;
        case lod:
            t++;
            s[t].t = s[base(b, i->l) + i->v.i].t;
			if(s[t].t==_INT)
				s[t].v.i = GETVAL(s[base(b, i->l) + i->v.i]);
			else if(s[t].t==_FLOAT)
				s[t].v.d = GETVAL(s[base(b, i->l) + i->v.i]);
			else if(s[t].t==_BOOL)
				s[t].v.b = GETVAL(s[base(b, i->l) + i->v.i]);
            break;
        case sto:
			if(s[base(b, i->l) + i->v.i].t==_INT)
				s[base(b, i->l) + i->v.i].v.i = GETVAL(s[t]);
			else if(s[base(b, i->l) + i->v.i].t==_FLOAT)
				s[base(b, i->l) + i->v.i].v.d = GETVAL(s[t]);
			else if(s[base(b, i->l) + i->v.i].t==_BOOL)
				s[base(b, i->l) + i->v.i].v.b = GETVAL(s[t]);
            t--;
            break;
        case cal:		// generate new block mark
            s[t + 1].v.i = base(b, i->l);
            s[t + 2].v.i = b;
            s[t + 3].v.i = p;
            b = t + 1;
            p = i->v.i;
            break;
        case Int:
            t += i->v.i;
            break;
        case jmp:
            p = i->v.i;
            break;
        case jpc:
            if(GETVAL(s[t]) == 0)
                p = i->v.i;
			t=t-1;
            break;
        case jpe:
            if(GETVAL(s[t]) == 1)
                p = i->v.i;
			t=t-1;
            break;
        case init:
			if(i->l==_INT)
				s[base(b, 0) + i->v.i].v.i = GETVAL(s[base(b, 0) + i->v.i]);
			else if(i->l==_FLOAT)
				s[base(b, 0) + i->v.i].v.d = GETVAL(s[base(b, 0) + i->v.i]);
			else if(i->l==_BOOL)
				s[base(b, 0) + i->v.i].v.b = GETVAL(s[base(b, 0) + i->v.i]);
            s[base(b, 0) + i->v.i].t=i->l;
            break;
        case lar:
			s[t].v.i=GETVAL(s[t]);
			s[t-1].v.i=GETVAL(s[t-1]);
			size=s[t].v.i;
			loc=s[t-1].v.i;
			if(loc>=size||loc<0)
			{
				cout<<"array out of boundary\n"<<size<<"\t"<<loc<<endl;
				cout<<p-1<<endl;
				exit(-1);
			}
			t--;
			s[t].t=s[base(b, i->l) + i->v.i+loc].t;
			if(s[t].t==_INT)
				s[t].v.i = GETVAL(s[base(b, i->l) + i->v.i+loc]);
			else if(s[t].t==_FLOAT)
				s[t].v.d = GETVAL(s[base(b, i->l) + i->v.i+loc]);
			else if(s[t].t==_BOOL)
				s[t].v.b = GETVAL(s[base(b, i->l) + i->v.i+loc]);
			break;		
        case sar:
			s[t].v.i=GETVAL(s[t]);
			s[t-1].v.i=GETVAL(s[t-1]);
			size=s[t].v.i;
			loc=s[t-1].v.i;
			if(loc>=size||loc<0)
			{
				cout<<"array out of boundary\n"<<size<<"\t"<<loc<<endl;
				exit(-1);
			}
			if(s[base(b, i->l) + i->v.i+loc].t==_INT)
				s[base(b, i->l) + i->v.i+loc].v.i = GETVAL(s[t-2]);
			else if(s[base(b, i->l) + i->v.i+loc].t==_FLOAT)
				s[base(b, i->l) + i->v.i+loc].v.d = GETVAL(s[t-2]);
			else if(s[base(b, i->l) + i->v.i+loc].t==_BOOL)
				s[base(b, i->l) + i->v.i+loc].v.b = GETVAL(s[t-2]);
			t-=2;
			break;
        }
    } while(p != insv.size());
    cout<<"------------------------------program complete.\n";
}

int test(instruction *temp)
{
	unsigned char *p=(unsigned char *)temp;
	for(int i=0;i<sizeof(instruction)/sizeof(int);i++)
		if((*p)!=0xff)
			return true;
	return false;
}
void getstrs(ifstream &ifs)
{
	char ch;
	char temp[1024];
	int nn=0;
	while(ifs.get(ch))
	{
		switch(ch)
		{
		case '\\':
			ifs.get(ch);
			switch(ch)
			{
			case 'n':
				temp[nn++]='\n';
				break;
			case 't':
				temp[nn++]='\t';
				break;
			case '\\':
				temp[nn++]='\\';
				break;
			case '\"':
				temp[nn++]='\"';
				break;
			default:
				cout<<"error here\n";
				break;
			}
			break;
		case 0:
			temp[nn++]='\0';
			strs.push_back(temp);
			nn=0;
			break;
		default:
			temp[nn++]=ch;
			break;
		}
	};
}
void check()
{
	for(int i=0;i<insvec.size();i++)
	{
		cout<<i<<"\t"<<fctt[insvec[i]->f]<<"\t";
		cout<<insvec[i]->l<<"\t";
		switch(insvec[i]->t)
		{
		case _INT:
			cout<<insvec[i]->v.i<<"\n";
			break;
		case _FLOAT:
			cout<<insvec[i]->v.d<<"\n";
			break;
		case _BOOL:
			cout<<insvec[i]->v.b<<"\n";
			break;
		case _OPC:
			cout<<opcc[insvec[i]->v.op]<<"\n";
			break;
		default:
			break;
		}
	}
	cout<<"---------------------strs begins-----------"<<endl;
	for(int i=0;i<strs.size();i++)
		cout<<i<<"|\t"<<strs[i]<<endl;
}
int main(int argc, char *argv[])
{
	instruction *temp=new instruction();
    if(argc == 1) {
        cout<<"please input intermediate code file name: ";
        cin>>infilename;
    } else {
        strcpy(infilename, argv[1]);
    }
	ifstream infile;
    infile.open(infilename);
	infile.read((char *)temp,sizeof(instruction));
    while(test(temp)){
		insvec.push_back(temp);
		temp=new instruction();
		infile.read((char *)temp,sizeof(instruction));
    }
	getstrs(infile);
    infile.close();
	check();
    interpret(insvec);
    return 0;
}
