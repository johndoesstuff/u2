#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>   // mmap
#include <inttypes.h>   // PRIX64
#include "../common/config.h"
#include "../common/instruction.h"

#include "x86jit.h"

/**
	U2 VIRTUAL MACHINE

	This is the main file for the u2 virtual machine
	u2 bytecode -> x86 execution

	Usage = u2vm bytecode.u2b
*/

uint32_t nextInstruction(FILE* f, uint32_t* inst) {
	size_t n = fread(inst, sizeof(uint32_t), 1, f);
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

uint32_t getOpcode(uint32_t inst) {
	return inst >> (32 - OPCODE_BITS);
}

uint32_t getRd(uint32_t inst) {
	return (inst >> (32 - OPCODE_BITS - REG_BITS)) & ((1u << REG_BITS) - 1);
}

uint32_t getRs1(uint32_t inst) {
	return (inst >> (32 - OPCODE_BITS - 2*REG_BITS)) & ((1u << REG_BITS) - 1);
}

uint32_t getRs2(uint32_t inst) {
	return (inst >> (32 - OPCODE_BITS - 3*REG_BITS)) & ((1u << REG_BITS) - 1);
}

uint32_t getImm(uint32_t inst) {
	uint32_t mask = (1u << IMM_BITS) - 1;
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
	uint8_t *jit_memory = mmap(NULL,     // address
			4096,             // size
			PROT_READ | PROT_WRITE | PROT_EXEC,
			MAP_PRIVATE | MAP_ANONYMOUS,
			-1,               // fd
			0);               // offset
	if (jit_memory == MAP_FAILED) {
		printf("Could not allocate memory for jit compilation!\n");
		exit(1);
	}
	uint8_t *jit_base = jit_memory;

	uint32_t instruction;
	while (nextInstruction(bytecodeFile, &instruction)) {
		printf("Read instruction %u (0x%08X)\n", instruction, instruction);

		uint32_t opcode = getOpcode(instruction);
		uint32_t rd = getRd(instruction);
		uint32_t rs1 = getRs1(instruction);
		uint32_t rs2 = getRs2(instruction);
		uint64_t immediate = getImm(instruction);

		Instruction instructionObj = Instructions[opcode];

		// print decoded instruction
		printf("%s", instructionObj.name);

		//printf("DEBUG: rs2 = %d\n", rs2);

		// check for long immediates
		if (rs2 && instructionObj.format != FORMAT_F) { // value in rs2 when one shouldn't be expected
			switch (instructionObj.format) { // check imm extension is supported
				case FORMAT_J:
				case FORMAT_I:
				case FORMAT_M:
					break;
				default:
					printf("Immediate extension is not supported for instructions of type %s", instructionObj.name);
					exit(1);
			}

			// if rs2 contains 1 or 2 load next rs2 bytes into imm
			if (rs2 == 1 || rs2 == 2) {
				uint32_t immExt;
				int captured = nextInstruction(bytecodeFile, &immExt);
				if (!captured) {
					printf("Expected immediate extension but instead recieved EOF? Check rs2 value for last inst.\n");
				}
				immediate = immExt;
				if (rs2 == 2) {
					captured = nextInstruction(bytecodeFile, &immExt);
					if (!captured) {
						printf("Expected immediate extension but instead recieved EOF? Check rs2 value for second to last inst.\n");
					}
					immediate |= (uint64_t)immExt << 32;
				}
			} else {
				printf("Invalid rs2 value\n");
			}
		}



		switch (instructionObj.format) {
			case FORMAT_F:
				printf(" r%d r%d r%d", rd, rs1, rs2);
				break;
			case FORMAT_R:
				printf(" r%d r%d", rd, rs1);
				break;
			case FORMAT_I:
				printf(" r%d %" PRIX64, rd, immediate);
				break;
			case FORMAT_J:
				printf(" %" PRIX64, immediate);
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
	uint64_t (*func)() = (uint64_t (*)())jit_base;
	uint64_t result = func();

	printf("%" PRIX64 "\n", result);

	fclose(bytecodeFile);
	return 0;
}
