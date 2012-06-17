/* 扩展测试2：typedef、数组整体赋值 */
typedef int A[3];
typedef int B;
typedef B C[3];
A a1={1,2,3},a2;
B b1;
int b2;
C c;

void main()
{
	a2=a1;
	write(a2);
	b1 = 4;
	b2 = 5;
	write(b1);
	write(b2);
	c[0]=6;
	c[1]=7;
	c[2]=8;
	write(c);
}

