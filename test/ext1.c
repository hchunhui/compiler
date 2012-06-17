/* 扩展测试1：函数前置声明 */
int f()
{
}
int g();
void main()
{
	f();
	g();
}

int g()
{
	write("PASS\n");
}
