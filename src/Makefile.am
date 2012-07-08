bin_PROGRAMS = c1c
c1c_SOURCES = ../rule/c1.tab.y ../rule/c1.lex.l \
	      sym_tab.c check.c type.c \
	      gen_c.c gen_eir.c gen_spim.c \
	      c1c.c
AM_YFLAGS=-d
AM_CPPFLAGS=-I$(top_srcdir)/include
CLEANFILES=c1.tab.h c1.tab.c c1.lex.c
