int f(int a, float b);
void main()
{
	const int a=3;
	const int b[10]={1,2,3,4,5,6,7,8,9,10};
	int a[3],b[3];
	a=4;
	b[1]=4;
	f(1);
	f(1,2,3);
	a=b;
}

