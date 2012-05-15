/* 例子1：不用任何begin、end包围的程序。C语言的函数必须用{ }包围，因而做不到这一点 */
/*var a;
procedure b;
	if 1=1 then a:=2;
call b.*/

int a;
void b()
{
	if (1==1) a=2;
}

void main()
{
	b();
}

