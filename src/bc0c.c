#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "sym_tab.h"
#include "type.h"
#include "gen.h"

int parse();

extern struct gen_info check;
struct gen_info *gi[] = {
	&gen_eir,
	&gen_c,
	&check,
	&gen_spim,
	NULL,
};

/* c0.y */
extern struct sym_tab *symtab;

/* c0.lex */
extern FILE *yyin;

int main(int argc, char **argv)
{
	/* get options */
	FILE *infile;
	int i;
	char *mach = "eir";
	char *inf = NULL;
	char *outf = "a.out";
	int opt;
	while ((opt = getopt(argc, argv, "m:o:h")) != -1) {
		switch (opt) {
		case 'o':
			outf = optarg;
			break;
		case 'm':
			mach = optarg;
			break;
		case 'h':
		default: /* '?' */
			fprintf(stderr,
				"%s: Bison C0 compiler\n"
				"Usage: %s [-m mach] [-o obj_file] src_file\n\n"
				"o  output in obj_file (default: a.out)\n"
				"m  select a object machine (default:pl0sim)\n",
				argv[0],
				argv[0]);
			fprintf(stderr,
				"\nAvailable object machine:\n\n");
			for(i = 0; gi[i]; i++)
			fprintf(stderr,
				"%-10s %s\n",
				gi[i]->name,
				gi[i]->info);
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
	
	symtab = symtab_new(NULL);
	type_init();
	symtab_enter_t(symtab, "int", get_type(TYPE_INT, 0, 0, NULL, NULL));
	symtab_enter_t(symtab, "float", get_type(TYPE_FLOAT, 0, 0, NULL, NULL));
	symtab_enter_t(symtab, "bool", get_type(TYPE_BOOL, 0, 0, NULL, NULL));
	symtab_enter_t(symtab, "void", get_type(TYPE_VOID, 0, 0, NULL, NULL));
	yyin = infile;
	fprintf(stderr, "syntax check\n");
	if(parse())
		return 1;
	fprintf(stderr, "semantic check\n");
	check.gen_code(symtab, NULL);
	fprintf(stderr, "code generation\n");
	for(i = 0; gi[i]; i++)
	{
		if(strcmp(mach, gi[i]->name) == 0)
		{
			fprintf(stderr, "use machine %s.\n", gi[i]->name);
			gi[i]->gen_code(symtab, outf);
		}
	}
	
	fclose(infile);
	return 0;
}
