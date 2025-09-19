#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "../common/config.h"
#include "../common/instruction.h"

#include "x86jit.h"

/**
	U2 VIRTUAL MACHINE

	This is the main file for the u2 virtual machine
	u2 bytecode -> x86 execution

	Usage = u2vm bytecode.u2b
*/

int nextInstruction(FILE* f, unsigned int* inst) {
	size_t n = fread(inst, sizeof(unsigned int), 1, f);
	if (n != 1) {
		if (feof(f)) {
			return 0;
		} else {
			printf("Error reading instruction\n");
			exit(1);
		}
	}
	return 1;
}

unsigned int getOpcode(unsigned int inst) {
	return inst >> (32 - OPCODE_BITS);
}

unsigned int getRd(unsigned int inst) {
	return (inst >> (32 - OPCODE_BITS - REG_BITS)) & ((1u << REG_BITS) - 1);
}

unsigned int getRs1(unsigned int inst) {
	return (inst >> (32 - OPCODE_BITS - 2*REG_BITS)) & ((1u << REG_BITS) - 1);
}

unsigned int getRs2(unsigned int inst) {
	return (inst >> (32 - OPCODE_BITS - 3*REG_BITS)) & ((1u << REG_BITS) - 1);
}

unsigned int getImm(unsigned int inst) {
	unsigned int mask = (1u << IMM_BITS) - 1;
	int imm = inst & mask;
	// sign extend
	if (imm & (1u << (IMM_BITS - 1))) {
		imm |= ~mask;
	}
	return imm;
}

int main(int argc, char** argv) {
	// correct usage check
	if (argc < 2) {
		printf("Usage: u2vm bytecode.u2b\n");
		exit(1);
	}

	// try to open file
	char* bytecodePath = argv[1];
	FILE* bytecodeFile = fopen(bytecodePath, "rb");
	if (bytecodeFile == NULL) {
		printf("Could not find file of path %s\n", bytecodePath);
		exit(1);
	}

	// prepare memory for jit execution
	char *jit_memory = mmap(NULL,     // address
			4096,             // size
			PROT_READ | PROT_WRITE | PROT_EXEC,
			MAP_PRIVATE | MAP_ANONYMOUS,
			-1,               // fd
			0);               // offset
	if (jit_memory == MAP_FAILED) {
		printf("Could not allocate memory for jit compilation!\n");
		exit(1);
	}
	char *jit_base = jit_memory;

	unsigned int instruction;
	while (nextInstruction(bytecodeFile, &instruction)) {
		printf("Read instruction %d (0x%08X)\n", instruction, instruction);

		unsigned int opcode = getOpcode(instruction);
		unsigned int rd = getRd(instruction);
		unsigned int rs1 = getRs1(instruction);
		unsigned int rs2 = getRs2(instruction);
		unsigned int immediate = getImm(instruction);

		Instruction instructionObj = Instructions[opcode];

		// print decoded instruction
		printf("%s", instructionObj.name);
		switch (instructionObj.format) {
			case FORMAT_F:
				printf(" r%d r%d r%d", rd, rs1, rs2);
				break;
			case FORMAT_R:
				printf(" r%d r%d", rd, rs1);
				break;
			case FORMAT_I:
				printf(" r%d %d", rd, immediate);
				break;
			case FORMAT_J:
				printf(" %d", immediate);
				break;
			case FORMAT_D:
				printf(" r%d", rd);
				break;
			case FORMAT_NONE:
				break;
			default:
				printf(" [unknown]");
				exit(1);
				break;
		}
		printf("\n");

		emit_jit(&jit_memory, opcode, rd, rs1, rs2, immediate);
	}

	// return from jit
	emit_x86ret_reg(&jit_memory, 1);

	// dump machine code because god knows im not getting this right my first try
	// or my second or third or fourth
	size_t emitted_size = jit_memory - jit_base;
	printf("===== x86 dump =====\n");
	for (int i = 0; i < emitted_size; i++) {
		printf("%02X ", (unsigned char)jit_base[i]);
	}
	printf("\n\n");

	// try to execute jit memory
	int (*func)() = (int (*)())jit_base;
	int result = func();

	printf("%X\n", result);

	fclose(bytecodeFile);
	return 0;
}
