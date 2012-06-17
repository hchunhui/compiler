CC       = gcc -g -c -Iinclude
LD	 = gcc
LEX      = flex -i -I 
YACC     = bison -d -y -r all --debug

OBJ	 = src/c0.tab.o src/c0.lex.o src/sym_tab.o src/gen_c.o src/check.o src/gen_eir.o src/gen_spim.o src/bc0c.o src/type.o

src/%.o : src/%.c
	$(CC) -o $@ $<
src/%.lex.c : rule/%.lex
	$(LEX) -o $@ $<
src/%.tab.c : rule/%.y
	$(YACC) -b c0 -o $@ $<

bc0c: $(OBJ)
	$(LD) -o $@ $(OBJ)
type: src/type.tab.o src/type.lex.o
	$(LD) -o $@ src/type.tab.o src/type.lex.o
all: clean bc0c
clean:
	rm -f src/*.lex.c src/*.tab.c src/*.tab.h src/*~ *~
	rm -f src/*.o
	rm -f $(OBJ)
	rm -f bc0c
	rm -f src/*.output
