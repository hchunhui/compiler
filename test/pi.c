
float mp2(int num_steps)
{
	int i;
	float x, pi;
	float step;
	step = 1.0/num_steps;
	pi = 0.0;
	for(i = 0; i < num_steps; i=i+1)
	{
		x = (i + 0.5)*step;
		pi = pi+4.0/(1.0 + x*x);
	}
	pi = pi/num_steps;
	return pi;	
}

int main()
{
	write(mp2(10000));
	return 0;
}
		
