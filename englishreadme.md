C1 compiler instructions
-------------------
* Features *
    Multi - objective: to C1 source code can be generated MIPS assembly code, EIR binary code and C code;
    - Powerful type system: You can identify the type defined C language syntax and outputs the type expression;
    - To achieve most of the C1 language features;
    - With extended syntax: If continue, for the like;
    - More detailed error reports;

* Documents *
doc / c1.pdf
doc / c0.pdf 2.1 Jie intermediate code generation, reference c1.y

* Description * Important documents
src /
| --rule /
| | --c1.y Syntax file
| | --c1.lex Lexical file
| --src /
| | --c1c.c Main compiler
| | --sym_tab.c Symbol table implementation
| | --type.c Type system
| | --check.c Semantic checking system
| | --gen_c.c Generate C code generation from AST
| | --gen_eir.c Generating PL / 0 intermediate code generator from AST
| | --gen_spim.c Generated from AST MIPS assembly code generator
| --include /
| | --ast_node.h AST achieve
| | --list.h Linked list (extracted from the Linux kernel)
| | --node_type.h AST node type definition
| | --type.h Type system head
| | --error.h Error
| | --eir_inst.h EIR instruction format
| | --gen.h Interface Builder
|. --test / * C simple test program
|. --eir / * C1 test program

* put up *
make clean
make
generate:
c1c

* Run *
Read parameters from the command line, using a method similar to the GCC:

EIR compiled intermediate code:
./c1c src_file [-o out_file]
Explain EIR:
        ./interpreter out_file

Compile the generated C code:
./c1c src_file [-o out_file] -m c

Compiled MIPS assembly code:
./c1c src_file [-o out_file] -m spim

help:
