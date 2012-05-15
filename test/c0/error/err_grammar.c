void a()
{
	int v;
	v=2   /* miss ; */
	if(v = 1)v=3; /* == */
	while(1){} /* condition */
	v==3; /* stmt err */
}

void b()
{
	int a,b==3; /* error 2 */
	int if; /*vardecl err*/
}

void main()
{
	a();
	a(;  /* miss ) */
}
