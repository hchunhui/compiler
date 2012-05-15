#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c0.h"
#include "file.h"

instruction code[2010];
int cx;

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr,
			"This is C0 interpreter\n"
			"Usage: %s obj_file\n",
			argv[0]);
		exit(1);
	}

	switch(read_file(argv[1], code, 2000, &cx))
	{
	case 1:
		fprintf(stderr,
			"%s: Can't read file %s\n",
			argv[0],
			argv[1]);
		exit(1);
		break;
	case 2:
		fprintf(stderr,
			"%s: Bad endian or bad object file.\n",
			argv[0]);
		exit(1);
		break;
	case 3:
		fprintf(stderr,
			"%s: Warning: code too long.\n",
			argv[0]);
		break;
	case 4:
		fprintf(stderr,
				"%s: Bad object file %s\n",
				argv[0],
				argv[1]);
		exit(1);
		break;
	}
	listcode(0);
	interpret();
	return 0;
}
