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

void setBitRange(int *instruction, int value, int start, int length) {
	int mask = ((1 << length) - 1) << start;
	*instruction &= ~mask;
	*instruction |= (value & ((1 << length) - 1)) << start;
}

void setOp(int* instruction, int opcode) {
	setBitRange(instruction, opcode, 26, 6);
}

void setRd(int* instruction, int rd) {
	setBitRange(instruction, rd, 22, 4);
}

void setRs1(int* instruction, int rs1) {
	setBitRange(instruction, rs1, 18, 4);
}

void setRs2(int* instruction, int rs2) {
	setBitRange(instruction, rs2, 14, 4);
}

void setImm(int* instruction, int imm) {
	setBitRange(instruction, imm, 0, 14);
}

void emitByte(unsigned char byte, FILE* fptr) {
	//printf("Emit %X\n", byte);
	fwrite(&byte, sizeof(unsigned char), 1, fptr);
}

void emitInst(unsigned int inst, FILE* fptr) {
	fwrite(&inst, sizeof(unsigned int), 1, fptr);
}

int toRegister(char* reg) {
	if (reg == NULL) {
		printf("Internal Error: Invalid Register\n");
		exit(1);
	}
	if (tolower(*reg) != 'r') {
		printf("Invalid Register, expected [r1-r16]\n");
		exit(1);
	}
	reg++;
	int ret = atoi(reg);
	
	// atoi fail or invalid reg range
	if (ret > 16 || ret <= 0) {
		printf("Invalid Register, expected [r1-r16]\n");
		exit(1);
	}
	return atoi(reg);
}

int getFirstChar(char* str, char ch) {
	// return -1 if not found
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == ch) return i;
	}
	return -1;
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

	char* bcPath = argv[2];
	FILE* bcFile = fopen(bcPath, "wb");
	if (bcFile == NULL) {
		printf("Could not open file of path %s\n", bcPath);
		exit(1);
	}

	char* line = NULL;
	size_t linec = 0;
	size_t len;
	ssize_t read;
	while ((read = getline(&line, &len, asmFile)) != -1) {
		// remove \n from line
		line[read - 1] = '\0';

		// ignore past ;
		int comment = getFirstChar(line, ';');
		if (comment != -1) {
			line[comment] = '\0';
		}

		// get op components
		char** opargs = malloc(sizeof(char*) * (countDelim(line, ' ') + 1));
		int opargsc = 0;
		for (char* pch = strtok(line, " "); pch != NULL; pch = strtok(NULL, " ")) {
			opargs[opargsc++] = pch;
			printf("Found arg: %s\n", pch);
		}

		// if line is empty ignore
		if (opargsc == 0) continue;

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
				if (opargsc != 2) error_argnum(opargsc, 2, instruction.name, linec);
				break;
			case FORMAT_NONE:
				if (opargsc != 1) error_argnum(opargsc, 1, instruction.name, linec);
				break;
		}

		int rd  = -1;
		int rs1 = -1;
		int rs2 = -1;
		int imm = -1;

		switch (instruction.format) {
			case FORMAT_F:
				rd = toRegister(opargs[1]);
				rs1 = toRegister(opargs[2]);
				rs2 = toRegister(opargs[3]);
				break;
			case FORMAT_R:
				rd = toRegister(opargs[1]);
				rs1 = toRegister(opargs[2]);
				break;
			case FORMAT_I:
				rd = toRegister(opargs[1]);
				imm = atoi(opargs[2]);
				break;
			case FORMAT_J:
				imm = atoi(opargs[1]);
				break;
			case FORMAT_D:
				rd = toRegister(opargs[1]);
				break;
			case FORMAT_NONE:
				break;
			default:
				printf("Internal Error: Unknown Instruction format\n");
				exit(1);
		}

		// generate bytecode
		
		unsigned int instBC = 0;
		setOp(&instBC, opcode);
		if (rd != -1) setRd(&instBC, rd);
		if (rs1 != -1) setRs1(&instBC, rs1);
		if (rs2 != -1) setRs2(&instBC, rs2);
		if (imm != -1) setImm(&instBC, imm);
		printf("Instruction: %X\n", instBC);

		emitInst(instBC, bcFile);

		// cleanup
		free(opargs);
		linec++;
	}
}
