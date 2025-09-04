#include "../common/instruction.h"
#include "../common/config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "error.c"

int countDelim(char* str, char delim) {
	int count = 0;
	for (char* clone = str; *clone != '\0'; clone++) {
		if (*clone == delim) count++;
	}
	return count;
}

void setOp(int* instruction, int opcode) {
	*instruction &= ~((1 << 6) - 1);
	*instruction |= opcode << 26;
}

void setRd(int* instruction, int rd) {
	rd &= (1 << 4) - 1;
	*instruction &= ~((1 << 4) - 1);
	*instruction |= rd << 22;
}

void setRs1(int* instruction, int rs1) {
	rs1 &= (1 << 4) - 1;
	*instruction &= ~((1 << 4) - 1);
	*instruction |= rs1 << 18;
}

void setRs2(int* instruction, int rs2) {
	rs2 &= (1 << 4) - 1;
	*instruction &= ~((1 << 4) - 1);
	*instruction |= rs2 << 18;
}

void setImm(int* instruction, int imm) {
	imm &= (1 << 14) - 1;
	*instruction &= ~((1 << 14) - 1);
	*instruction |= imm;
}

int main(int argc, char** argv) {
	// correct usage check
	if (argc < 3) {
		printf("Usage: u2asm assembly.u2a bytecode.u2b\n");
		exit(1);
	}

	// try to open file
	char* asmPath = argv[1];
	FILE* asmFile = fopen(asmPath, "r");
	if (asmFile == NULL) {
		printf("Could not find file of path %s\n", asmPath);
		exit(1);
	}

	char* line = NULL;
	size_t linec = 0;
	size_t len;
	ssize_t read;
	while ((read = getline(&line, &len, asmFile)) != -1) {
		// remove \n from line
		line[read - 1] = '\0';

		// get op components
		char** opargs = malloc(sizeof(char*) * countDelim(line, ' '));
		int opargsc = 0;
		for (char* pch = strtok(line, " "); pch != NULL; pch = strtok(NULL, " ")) {
			opargs[opargsc++] = pch;
			printf("Found arg: %s\n", pch);
		}

		// search for op
		int opcode = -1;
		for (int i = 0; i < Instruction_Count; i++) {
			// convert to lower (ops arent case sensitive)
			for (char* t = opargs[0]; *t; ++t) *t = tolower(*t);
			if (strcmp(Instructions[i].name, opargs[0]) == 0) {
				opcode = i;
				break;
			}
		}

		if (opcode == -1) {
			printf("Unknown Instruction \"%s\"", opargs[0]);
			exit(1);
		}

		// correct format?
		Instruction instruction = Instructions[opcode];
		switch (instruction.format) {
			case FORMAT_F:
				if (opargsc != 4) error_argnum(opargsc, 4, instruction.name, linec);
				break;
			case FORMAT_R:
				if (opargsc != 3) error_argnum(opargsc, 3, instruction.name, linec);
				break;
			case FORMAT_I:
				if (opargsc != 3) error_argnum(opargsc, 3, instruction.name, linec);
				break;
			case FORMAT_J:
				if (opargsc != 2) error_argnum(opargsc, 2, instruction.name, linec);
				break;
			case FORMAT_D:
				if (opargsc != 1) error_argnum(opargsc, 1, instruction.name, linec);
				break;
		}

		unsigned int instBC = 0;
		setOp(&instBC, opcode);
		printf("Instruction: %X\n", instBC);

		// cleanup
		free(opargs);
		linec++;
	}
}
