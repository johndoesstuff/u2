target:
	gcc vm/x86jit.c vm/main.c -o vm/vm -g
	gcc assembler/main.c -o assembler/assembler -g
