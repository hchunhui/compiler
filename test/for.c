int main()
{
	int i;
	i=5;
	for(;i;i=i+1)
	{
		write(i);
		i=-1;
		continue;
	}
	i=5;
	while(i)
	{
		write(i);
		i=0;
		continue;
	}
}
