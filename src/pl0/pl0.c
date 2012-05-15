#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "pl0.h"
#include "glo.h"
#include "file.h"

int main(int argc, char **argv)
{
	/* get options */
	int interp = 0;
	char *inf = NULL;
	char *outf = "a.out";
	int opt;
	while ((opt = getopt(argc, argv, "io:h")) != -1) {
		switch (opt) {
		case 'o':
			outf = optarg;
			break;
		case 'i':
			interp = 1;
			break;
		case 'h':
		default: /* '?' */
			fprintf(stderr,
				"%s: PL/0 compiler\n"
				"Usage: %s [-o obj_file] src_file\n\n"
				"o\t output in obj_file (default: a.out)\n"
				"i\t interpret obj_file immediately.\n",
				argv[0],
				argv[0]);
			exit(1);
		}
	}
	
	if (optind >= argc) {
		fprintf(stderr,
			"%s: Expected argument after options. use -h.\n",
			argv[0]);
		exit(1);
	}

	if (optind != argc - 1) {
		fprintf(stderr,
			"%s: Too many argument.\n",
			argv[0]);
		exit(1);
	}
	
	inf = argv[optind];

	if(strcmp(inf, "-") == 0)
		infile = stdin;
	else
		infile = fopen(inf, "r");
	if(!infile) {
		fprintf(stderr,
			"%s: File %s can't be opened.\n",
			argv[0],
			inf);
		exit(1);
	}
	
	lexer_setup();
	parse();
	write_file(outf, code, cx);
	
	if(err==0 && interp)
		interpret();
	if(err)
		printf("errors in PL/0 program\n");
	
	fclose(infile);
	return 0;
}
