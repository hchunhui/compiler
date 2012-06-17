/* 基本测试1：测试C1最基本的语法和逻辑 */
int ga, gb;
int x[10];
int f(){return 10;}
int g(int a, int b){return a;}
int h(int a, int b){return b;}
void ff(int a){a=-1;}
void gg(){ga=99;}
void main()
{
	int pass;
	int a, b;
	pass = 1;
	while(1)
	{
/* 基本赋值和判断 */
		a = 1;
		if(a != 1)
			break;
/* 变量间赋值 */
		pass = pass + 1;
		b = 2;
		a = b;
		if(a != 2 || b != 2)
			break;
/* 全局变量赋值 */
		pass = pass + 1;
		ga = 3;
		gb = a;
		if(ga != 3 || gb != 2)
			break;
/* 函数返回值测试 */
		pass = pass + 1;
		if(10 != f())
			break;
/* 函数参数传递测试 */
		pass = pass + 1;
		if(111 != g(111, 222))
			break;
		pass = pass + 1;
		if(222 != h(111, 222))
			break;
/* 实参、形参测试 */
		pass = pass + 1;
		a = 10;
		ff(a);
		if(a != 10)
			break;
/* 作用域测试 */
		pass = pass + 1;
		ga = 0;
		gg();
		if(ga != 99)
			break;
/* if-else测试 */
		pass = pass + 1;
		a = 1;
		if(a != 1)
			b = 10;
		else
			b = 100;
		if(100 != b)
			break;
		pass = pass + 1;
		a = 2;
		if(a != 1)
			b = 10;
		else
			b = 100;
		if(10 != b)
			break;
/* 悬空else测试 */
		pass = pass + 1;
		a = 1;
		b = 0;
		if(a == 1)
			if(a != 1)
				b = 1;
			else b = 2;
		if(2 != b)
			break;
/* 数组测试 */
		pass = pass + 1;
		a = 0;
		while(a < 10)
		{
			x[a] = a+a;
			a = a + 1;
		}
		a = a - 1;
		while(a >= 0)
		{
			write(x[a]);
			write("\n");
			if(x[a] != a+a)
				break;
			a = a - 1;
		}
		if(a != -1)
			break;

		write("PASS\n");
		return;
	}
	write("FAIL\npass:");
	write(pass);
	write("\n");
}
