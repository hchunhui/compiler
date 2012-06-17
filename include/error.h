#ifndef _ERROR_H_
#define _ERROR_H_
#include <stdio.h>
#include <stdarg.h>

static inline void new_error(int ex, int row, int col, char *fmt,...)
{
	va_list arg;
	fflush(stdout);
	va_start(arg, fmt);
	fprintf(stderr, "Error@(%d:%d): ", row, col);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	if(ex)
		exit(1);
}

static inline void new_warn(int ex, int row, int col, char *fmt,...)
{
	va_list arg;
	fflush(stdout);
	va_start(arg, fmt);
	fprintf(stderr, "Warning@(%d:%d): ", row, col);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	if(ex)
		exit(1);
}

static inline void new_remark(char *fmt,...)
{
	va_list arg;
	va_start(arg, fmt);
	fprintf(stderr, "Remark: ");
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

static inline void new_eol()
{
	fprintf(stderr, "\n");
}

static inline void new_space()
{
	fprintf(stderr, " ");
}

#endif /* _ERROR_H_ */
