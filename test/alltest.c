typedef int     A[2];

 
int
main() 
{
    
	/*
	 * test types 
	 */ 
    int             i,
                    j = 1,
	k;
    
int            a[2][3] = { 1, 2, 3, 5, 6, 7 };
    
float          f,
                    g;
    
A c[3] = {
    0, 1, 2, 3, 4, 5};
    
bool b;
    
write(c[1][2]);
    
write("\n");
    
	/*
	 * test expressions 
	 */ 
	i = 1 + +2 + -3;
    
j = k = i;
    
write(i);
    write(j);
    write(k);
    
 
	/*
	 * test for while 
	 */ 
	for (i = 0; i < 2; i = i + 1)
	
 {
	
j = 0;
	
while (j < 4)
	    
 {
	    
write(a[i][j]);
	    
if (j == 2)
		break;
	    
j = j + 1;
	    
}
	
write("good1\n");
	
}
    
write("good2\n");
    
 
i = 1;
    
for (; i; i = i + 1)
	
 {
	
write(i);
	
i = -1;
	
continue;
	
}
    
write("good3\n");
    
 
i = 2;
    
while (i)
	
 {
	
write(i);
	
i = 0;
	
continue;
	
}
    
write("good4\n");
    
 
	/*
	 * test bool float 
	 */ 
	f = 3.1415;
    
i = f;
    
g = j;
    
write(i);
    write(f);
    write(g);
    write(j);
    
b = (i == f);
    
write("\n");
    
write(b);
    write(i == f);
    
 
	/*
	 * test short circuit 
	 */ 
	k = i && (j = a[1][1]);
    
write(k);
    write(j);
    write(a[1][1]);
    
j = 2;
    
k = 3;
    
i = 4;
    
i + j;
    i && k;
    
if (j || k && i)
	
write("good circuit\n");
    
    else
	
write("bad circuit\n");

}
