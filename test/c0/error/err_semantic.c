int c;
void a()
{

	b(); /* error 3 */
	if(c%4){} /* error 4 */
}

void b(){}

void main()
{
	void nest(){} /* error 8 */
	a=3; /* error 5 */
	c(); /* error 6 */
	c=b+1; /* error 9 */
}
