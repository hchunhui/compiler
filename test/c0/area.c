int a;
void f()
{
	int a, b;
	b=a;
	a=4;
}

void main()
{
	int c,b;
	b=2;
	a=1;
	f();
	c=a;
	c=b;
}
