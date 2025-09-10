#include <stdio.h>
#include <stdlib.h>
#include "../common/instruction.h"
#include "../common/config.h"

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

	unsigned int instruction;
	while (nextInstruction(bytecodeFile, &instruction)) {
		printf("Read instruction %d (0x%08X)\n", instruction, instruction);

		unsigned int opcode = getOpcode(instruction);
		unsigned int rd = getRd(instruction);
		unsigned int rs1 = getRs1(instruction);
		unsigned int rs2 = getRs2(instruction);
		unsigned int immediate = getImm(instruction);

		/*printf("Opcode %X\n", opcode);
		printf("Rd %X\n", rd);
		printf("Rs1 %X\n", rs1);
		printf("Rs2 %X\n", rs2);
		printf("Immediate %X\n", immediate); */
		Instruction instructionObj = Instructions[opcode];

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
	}

	fclose(bytecodeFile);
	return 0;
}
