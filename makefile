target:
	gcc common/instruction.c vm/x86jit.c vm/main.c -o vm/vm -g
	gcc common/instruction.c assembler/main.c -o assembler/assembler -g

test:
	./assembler/assembler assembler/asm.u2a assembler/bytecode.u2b
	./vm/vm assembler/bytecode.u2b
