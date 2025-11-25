
CC       = gcc
CFLAGS   = -g3 -Wall -Wextra -Werror

COMMON   = src/common/instruction.c
VM_SRC   = src/vm/cfg.c src/vm/x86encoding.c src/vm/regalloc.c src/vm/x86jit.c src/vm/main.c
ASM_SRC  = src/assembler/main.c

VIM_SRC  = src/common/u2a.vim

BUILD_DIR= build

VM_BIN   = build/u2vm
ASM_BIN  = build/u2asm

TEST_SOURCES := $(wildcard tests/*.u2a)
TEST_OUTPUTS := $(TEST_SOURCES:.u2a=.u2b)

all: $(VM_BIN) $(ASM_BIN)

$(VM_BIN): $(COMMON) $(VM_SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON) $(VM_SRC) -o $(VM_BIN)

$(ASM_BIN): $(COMMON) $(ASM_SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON) $(ASM_SRC) -o $(ASM_BIN)

clean:
	rm -rf $(BUILD_DIR)

syntax:
	mkdir -p ~/.vim/syntax
	sudo cp $(VIM_SRC) ~/.vim/syntax/u2a.vim
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
	find . -regex '.*\.\(c\|h\)$$' -exec clang-format-18 --dry-run --Werror {} +

format:
	find . -regex '.*\.\(c\|h\)$$' -exec clang-format-18 -i {} +

