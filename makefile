target:
	gcc common/instruction.c vm/cfg.c vm/x86encoding.c vm/regalloc.c vm/x86jit.c vm/main.c -o vm/vm -g3
	gcc common/instruction.c assembler/main.c -o assembler/assembler -g3

syntax:
	mkdir -p ~/.vim/syntax
	sudo cp u2a.vim ~/.vim/syntax/u2a.vim
	mkdir -p ~/.vim/ftdetect
	echo "au BufRead,BufNewFile *.u2a set filetype=u2a" > ~/.vim/ftdetect/u2a.vim

test:
	./assembler/assembler assembler/asm.u2a assembler/bytecode.u2b
	./vm/vm assembler/bytecode.u2b
