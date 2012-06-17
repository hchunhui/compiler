/* C1扩展：for，数组参数等 */
typedef int A[2];
int f(A a)
{
	write(a);
	return 0;
}

void main()
{
	int i;
	A a[2]={1,2,3,4};
	for(i = 0; i < 2; i=i+1)
	{
		f(a[i]);
		continue;
	}
}

