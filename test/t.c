int main()
{
	int i;
	bool j;
	float k;
	int a[2]={1,3};
	int b[2]={2,4};
	for(i=0; i<2;i=i+1)
	{
		write(a[i]);
		write(b[i]);
	}
	i = 2147483648;
	j = 4;
	k = 4;
	while(1)
	{
		write("i=");
		write(i);
		write("\nj=");
		write(j);
		write("\nk=");
		write(k);
		write("\n");
		read(i,j,k);
	}
}

