/* 基本测试2：较刁钻的表达式，如连续赋值和短路 */
void main()
{
	int i,j,k;
	int a[5];
	i = 1+ +2+ -3;
	j = (k = i)- -1;
	if(i != 0 || k != 0 || j != 1)
	{
		write("FAIL\npass1\n");
		return;
	}
	k=i&&(j=k);
	if(i != 0 || k != 0 || j != 1)
	{
		write(i);
		write(j);
		write(k);
		write("\nFAIL\npass2\n");
		return;
	}
	j = 2;
	k = 3;
	i = 4;
	i+j;
	i&&k;
	if(j || k && i)
		write("PASS\n");
	else
		write("FAIL\npass3\n");
}
