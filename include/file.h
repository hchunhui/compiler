#ifndef _FILE_H_
#define _FILE_H_
#include "pl0_inst.h"

int read_file(char *file, instruction *code, int max, int *pcx);
void write_file(char *file, instruction *code, int cx);


#endif /* _FILE_H_ */
