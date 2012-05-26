#include <stdio.h>
#include <stdint.h>
#include "pl0_inst.h"

const char magic1[]="#!";
const char default_i[]="c0i";
const char magic2[]="c0o";

struct file_inst
{
	int16_t f;
	int16_t l;
	int32_t a;
};

static int match_magic(FILE *fin, const char *magic)
{
	int i, ch;
	for(i = 0; magic[i]; i++)
	{
		ch = fgetc(fin);
		if(ch != magic[i])
			return 1;
	}
	return 0;
}

static int skip_line(FILE *fin)
{
	int ch;
	for(ch = fgetc(fin);
	    ch != '\n' && ch != EOF;
	    ch = fgetc(fin));
	if(ch == EOF)
		return 1;
	return 0;
}

static int check_file(FILE *fin)
{
	uint32_t end = 0;
	/* match magic */
	if(match_magic(fin, magic1))
		return 2;
	if(skip_line(fin))
		return 2;
	if(match_magic(fin, magic2))
		return 2;

	/* check endian */
	fread(&end, sizeof(uint32_t), 1, fin);
	if(end != 0x12345678)
		return 1;
	return 0;
}

int read_file(char *file, instruction *code, int max, int *pcx)
{
	FILE *fin;
	int i;
	struct file_inst fir;
	fin = fopen(file, "rb");
	if(!fin)
		return 1;
	switch(check_file(fin))
	{
	case 1: return 2;
	case 2: return 4;
	}
	
	for(i = 0; i < max; i++)
	{
		if(fread(&fir, sizeof(struct file_inst), 1, fin) != 1)
			break;
		code[i].f = fir.f;
		code[i].l = fir.l;
		code[i].a = fir.a;
	}
	*pcx = i;
	if(i >= max)
		return 3;
	return 0;
}

void write_file(char *file, instruction *code, int cx)
{
	int i;
	uint32_t end;
	FILE *fout;
	struct file_inst fir;
	
	fout = fopen(file, "wb");
	if(!fout) {
		fprintf(stderr,
			"Warning: Can't open %s .\n",
			file);
		return;
	}
	fprintf(fout, "%s%s\n%s", magic1, default_i, magic2);
	end = 0x12345678;
	fwrite(&end, sizeof(end), 1, fout);
	for(i = 0; i < cx; i++)
	{
		fir.f = code[i].f;
		fir.l = code[i].l;
		fir.a = code[i].a;
		fwrite(&fir, sizeof(fir), 1, fout);
	}
	fclose(fout);
	chmod(file, 0755);
}
