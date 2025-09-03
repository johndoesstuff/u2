#include "../common/instruction.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int countDelim(char* str, char delim) {
	int count = 0;
	for (char* clone = str; *clone != '\0'; clone++) {
		if (*clone == delim) count++;
	}
	return count;
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
	size_t len;
	ssize_t read;
	while ((read = getline(&line, &len, asmFile)) != -1) {
		// remove \n from line
		line[read - 2] = '\0';

		// get op components
		char** opargs = malloc(sizeof(char*) * countDelim(line, ' '));
		int i = 0;
		for (char* pch = strtok(line, " "); pch != NULL; pch = strtok(NULL, " ")) {
			opargs[i++] = pch;
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
	}
}
