
CC       = gcc
CFLAGS   = -g3 -Wall -Wextra

COMMON   = src/common/instruction.c
VM_SRC   = src/vm/cfg.c src/vm/x86encoding.c src/vm/regalloc.c src/vm/x86jit.c src/vm/main.c
ASM_SRC  = src/assembler/main.c

VM_BIN   = build/vm
ASM_BIN  = build/assembler

TEST_SOURCES := $(wildcard tests/*.u2a)
TEST_OUTPUTS := $(TEST_SOURCES:.u2a=.u2b)

all: $(VM_BIN) $(ASM_BIN)

$(VM_BIN): $(COMMON) $(VM_SRC)
	$(CC) $(CFLAGS) $(COMMON) $(VM_SRC) -o $(VM_BIN)

$(ASM_BIN): $(COMMON) $(ASM_SRC)
	$(CC) $(CFLAGS) $(COMMON) $(ASM_SRC) -o $(ASM_BIN)

clean:
	rm $(VM_BIN)
	rm $(ASM_BIN)

syntax:
	mkdir -p ~/.vim/syntax
	sudo cp u2a.vim ~/.vim/syntax/u2a.vim
	mkdir -p ~/.vim/ftdetect
	echo "au BufRead,BufNewFile *.u2a set filetype=u2a" > ~/.vim/ftdetect/u2a.vim

.PHONY: test
test: $(TEST_OUTPUTS)
	@for b in $(TEST_OUTPUTS); do \
		echo "--- Running $$b ---"; \
		$(VM_BIN) $$b; \
	done

tests/%.u2b: tests/%.u2a $(ASM_BIN)
	$(ASM_BIN) $< $@

format-dry:
	clang-format-18 --dry-run --Werror -style=file $(find . -name '*.c' -o -name '*.h')

format:
	find . -regex '.*\.\(c\|h\|cpp\|hpp\)$$' -exec clang-format-18 -i {} +

