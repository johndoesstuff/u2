CC       = gcc
CFLAGS   = -g3 -Wall -Wextra

COMMON   = common/instruction.c
VM_SRC   = vm/cfg.c vm/x86encoding.c vm/regalloc.c vm/x86jit.c vm/main.c
ASM_SRC  = assembler/main.c

VM_BIN   = vm/vm
ASM_BIN  = assembler/assembler

TEST_SOURCES := $(wildcard tests/*.u2a)
TEST_OUTPUTS := $(TEST_SOURCES:.u2a=.u2b)

all: $(VM_BIN) $(ASM_BIN)

$(VM_BIN): $(COMMON) $(VM_SRC)
	$(CC) $(CFLAGS) $(COMMON) $(VM_SRC) -o $(VM_BIN)

$(A_BIN): $(COMMON) $(A_SRC)
	$(CC) $(CFLAGS) $(COMMON) $(ASM_SRC) -o $(ASM_BIN)

syntax:
	mkdir -p ~/.vim/syntax
	sudo cp u2a.vim ~/.vim/syntax/u2a.vim
	mkdir -p ~/.vim/ftdetect
	echo "au BufRead,BufNewFile *.u2a set filetype=u2a" > ~/.vim/ftdetect/u2a.vim

test: $(TEST_OUTPUTS)
	@for b in $(TEST_OUTPUTS); do \
		echo "--- Running $$b ---"; \
		$(VM_BIN) $$b; \
	done

tests/%.u2b: tests/%.u2a assembler/assembler
	$(ASM_BIN) $< $@
