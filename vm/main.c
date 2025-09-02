/**
	U2 VIRTUAL MACHINE

	This is the main file for the u2 virtual machine
	u2 bytecode -> x86 execution

	Usage = u2vm bytecode.u2b
*/

#include <stdio.h>
#include <stdlib.h>

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
	}

	fclose(bytecodeFile);
	return 0;
}
