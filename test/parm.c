/*int f(int a, bool b, int e[3])
{
	int c;
	int d;
	write(e[0]);
	write(e[1]);
	write(e[2]);
}*/
typedef int A[3];
void main()
{
	A h[2], i[2]={1,2,3,4,5,6};
	h[1] = h[0] = i[1];
	write(h);
	write("\n");
	write(i[1]);
	read(h[0]);
	write(h[0]);
	/*h = e[1];*/
	/*g = e[1];
	f(1, 32, g);*/
}
