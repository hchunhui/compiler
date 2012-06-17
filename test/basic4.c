/* 基本测试4：类型和简单的typedef */
typedef int A[2];
void main()
{
	int i, j, k;
	float f, g;
	A c[3]={0,1,2,3,4,5};
	bool b;
	if(c[1][1] != 3)
	{
		write("FAIL\npass1\n");
		return;
	}
	f=3.1415;
	i=f;
	g=i;
	if(i != 3 || g != 3)
	{
		write("FAIL\npass2\n");
		return;
	}
	b=(i==f);
	write(b);write((i==f)+1);
	if(b != false || (i==f)+1 != 1)
	{
		write("FAIL\npass3\n");
		return;
	}
	write("\nPASS\n");
}
