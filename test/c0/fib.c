int n;
int ret;
void fib()
{
	int pn;
	int f1, f2;
	pn = n;
	if(n == 0)
		ret = 1;
	if(n == 1)
		ret = 1;
	if(n >= 2)
	{
		n = n - 1;
		fib();
		f1 = ret;
		n = n - 1;
		fib();
		f2 = ret;
		ret = f1 + f2;
	}
	n = pn;
}


void main()
{
	n = 7;
	fib();
	ret = ret; /* echo */
}
