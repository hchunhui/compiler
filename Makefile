# This Makefile requires GNU make.
SHELL = /bin/sh
CC = gcc
LEX = flex
OBJDIR = bin
BINDIR = bin
SRCDIR = src
LIB =
OBJS = $(OBJDIR)/glo.o $(OBJDIR)/interpreter.o $(OBJDIR)/file.o
OBJC0 = $(OBJDIR)/c0o/c0.o $(OBJDIR)/c0o/lexer.o $(OBJDIR)/c0o/parser.o
OBJPL0= $(OBJDIR)/pl0o/pl0.o $(OBJDIR)/pl0o/lexer.o $(OBJDIR)/pl0o/parser.o
OBJC0I = $(OBJDIR)/c0o/c0i.o $(OBJS)

INC=-Iinclude
CFLAGS = -g $(INC) $(CDEF)
CCOMPILE = $(CC) $(CFLAGS) -c

default: bin/c0o bin/pl0o bin/pl0 bin/c0c bin/c0i
	make -C src/bison
	ln -s -f bin/c0c .
	ln -s -f bin/c0i .
	ln -s -f bin/bc0c .

bin/pl0o:
	mkdir -p bin/pl0o
bin/c0o:
	mkdir -p bin/c0o

bin/pl0: $(OBJS) $(OBJPL0)
	$(CC) $(CFLAGS) -o bin/pl0 $(OBJS) $(OBJPL0)
bin/c0c: $(OBJS) $(OBJC0)
	$(CC) $(CFLAGS) -o bin/c0c  $(OBJS) $(OBJC0)

bin/c0i: $(OBJC0I)
	$(CC) $(CFLAGS) -o bin/c0i $(OBJC0I)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CCOMPILE) -o $@ $<
$(OBJDIR)/pl0o/%.o: $(SRCDIR)/pl0/%.c
	$(CCOMPILE) -o $@ $<
$(OBJDIR)/c0o/%.o: $(SRCDIR)/c0/%.c
	$(CCOMPILE) -o $@ $<

all: clean default
clean:
	- rm -f $(OBJS) $(OBJC0) $(OBJPL0) $(OBJC0I)
	- rm -f $(SRCDIR)/c0yy.c
	- rm -f $(SRCDIR)/*~
	- rm -f *~
	make -C src/bison clean

