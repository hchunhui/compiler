int m=7, n=85;
int x,y,z,q,r;
void multiply()
    {
        int a,b;
        a=x; b=y; z=0;
        while(b>0)
            {
                if((b)%2)  z=z+a;
                a=2*a; b=b/2;
            }
    }
void main()
{
    x=m; y=n;
    multiply();
}
