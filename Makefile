CC       = gcc
CXX      = g++
CFLAGS   = -g3 -Wall -Wextra -Werror

COMMON   = src/common/instruction.c
VM_SRC   = src/vm/cfg.c src/vm/x86encoding.c src/vm/regalloc.c src/vm/x86jit.c src/vm/main.c
ASM_SRC  = src/assembler/main.c

ASM_CPP_SRC = src/assembler/cpp/main.cpp

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

$(ASM_BIN): $(COMMON) $(ASM_CPP_SRC)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CFLAGS) $(ASM_CPP_SRC) -o $(ASM_BIN)

clean:
	rm -rf $(BUILD_DIR)

syntax:
	mkdir -p ~/.vim/syntax
	sudo cp $(VIM_SRC) ~/.vim/syntax/u2a.vim
	mkdir -p ~/.vim/ftdetect
	echo "au BufRead,BufNewFile *.u2a set filetype=u2a" > ~/.vim/ftdetect/u2a.vim

.PHONY: test
test:
	./tests/test.sh

format-dry:
	find . -regex '.*\.\(c\|h\)$$' -exec clang-format --dry-run --Werror {} +

format:
	find . -regex '.*\.\(c\|h\)$$' -exec clang-format -i {} +

