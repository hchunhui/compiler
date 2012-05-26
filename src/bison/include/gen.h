#ifndef _GEN_H_
#define _GEN_H_

struct sym_tab;
struct gen_info
{
	char *name;
	void (*gen_code)(struct sym_tab *, char *);
	char *info;
};

extern struct gen_info gen_c;
extern struct gen_info gen_eir;
extern struct gen_info gen_spim;


#endif /* _GEN_H_ */
