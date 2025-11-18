#include "../common/instruction.h"
#include "../common/config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>    // tolower
#include <stdint.h>   // uint32_t, uint64_t, ...
#include <inttypes.h> // PRIX64
#include "label.h"

#include "error.c"

/**
	U2 ASSEMBLER

	This is the main file for the u2 assembler
	u2 assembly (*.u2a) -> u2 bytecode (*.u2b)

	Usage = u2asm asm.u2a bytecode.u2b
 */

int count_delim(char* str, char delim) {
	int count = 0;
	for (char* clone = str; *clone != '\0'; clone++) {
		if (*clone == delim) count++;
	}
	return count;
}

void set_bit_range(uint32_t *instruction, int value, int start, int length) {
	int mask = ((1 << length) - 1) << start;
	*instruction &= ~mask;
	*instruction |= (value & ((1 << length) - 1)) << start;
}

void set_op(uint32_t* instruction, uint32_t opcode) {
	set_bit_range(instruction, opcode, 26, 6);
}

void set_rd(uint32_t* instruction, uint32_t rd) {
	set_bit_range(instruction, rd, 22, 4);
}

void set_rs1(uint32_t* instruction, uint32_t rs1) {
	set_bit_range(instruction, rs1, 18, 4);
}

void set_rs2(uint32_t* instruction, uint32_t rs2) {
	set_bit_range(instruction, rs2, 14, 4);
}

void set_imm(uint32_t* instruction, uint64_t imm) {
	set_bit_range(instruction, (int)imm, 0, 14);
}

void emit_inst(uint32_t inst, FILE* fptr, uint32_t* pc) {
    // important to note is pc is relative to each 32bit segment, it would be
    // silly to give pc byte-level precision since all instructions are 4bytes
    (*pc)++;
	fwrite(&inst, sizeof(uint32_t), 1, fptr);
}

uint32_t expect_register(char* reg) {
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
	return atoi(reg) - 1;
}

uint64_t expect_immediate(char* immediate, LabelTable* labels, int pass) {
	if (immediate == NULL) {
		printf("Internal Error: Invalid Immediate\n");
		exit(1);
	}
	
	char* endptr;
	int base = 10;

	// lets cover all our bases! heh..
	if (strlen(immediate) > 2 && immediate[0] == '0') {
		if (immediate[1] == 'x' || immediate[1] == 'X') {
			base = 16;
			immediate += 2; // skip 0x
		} else if (immediate[1] == 'b' || immediate[1] == 'B') {
			base = 2;
			immediate += 2; // skip 0b
		}
	}

	uint64_t val = strtoull(immediate, &endptr, base);
	if (*endptr != '\0') {
        // wait wait it could be a label.. we should wait for 2nd pass until
        // making any final decisions
        if (pass != 2) return 0;

        int fl = find_label(labels, immediate);
        if (fl < 0) { // we've done all we can.. give up :(
            printf("Invalid Immediate, unexpected character '%c' in %s\n", *endptr, immediate);
            exit(1);
        }
        return (uint64_t)fl;
	}
	return val;
}

int get_first_char(char* str, char ch) {
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

    // try to open output
	char* bcPath = argv[2];
	FILE* bcFile = fopen(bcPath, "wb");
	if (bcFile == NULL) {
		printf("Could not open file of path %s\n", bcPath);
		exit(1);
	}

    // initialize
	char* line = NULL;
	size_t linec = 0;
	size_t len;
	ssize_t read;
    LabelTable* labels = new_label_table();
    /*
     * 2 PASS SYSTEM
     *
     * First pass will just inc pc and set label addr for LabelTable* labels
     * Se/cond pass will use labels from labels to resolve addresses
     *
     * This system is necessary to resolve forward declared labels
     */
    int pass = 1;
    uint32_t pc = 0;

    asm_pass:
	while ((read = getline(&line, &len, asmFile)) != -1) {
		// remove \n from line
		line[read - 1] = '\0';

		// ignore past ;
		int comment = get_first_char(line, ';');
		if (comment != -1) {
			line[comment] = '\0';
		}

		// get op components
		char** opargs = malloc(sizeof(char*) * (count_delim(line, ' ') + 1));
		char** opargs_base = opargs;
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

        // might be label, not op. check if last character is a ':'
        if (opargs[0][strlen(opargs[0]) - 1] == ':') {
            if (opargsc != 1) {
                // why would you include something after the label??
                printf("Adding instructions in the same line as a label isn't currently supported.\n");
                exit(1);
            }
            if (pass == 1) {
                // remove ending ':'
                char* label_str = strdup(opargs[0]);
                label_str[strlen(label_str) - 1] = '\0';
                add_label(labels, label_str, pc);
            }
            continue;
        }

		if (opcode == -1) {
			printf("Unknown Instruction \"%s\"\n", opargs[0]);
			exit(1);
		}

		// correct format?
		Instruction instruction = Instructions[opcode];
		switch (instruction.format) {
			case FORMAT_F:
				if (opargsc != 4) error_argnum(opargsc, 4, instruction.name, linec);
				break;
			case FORMAT_M:
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

		uint32_t rd  = 0;
		uint32_t rs1 = 0;
		uint32_t rs2 = 0;
		uint64_t imm = 0;

		switch (instruction.format) {
			case FORMAT_F:
				rd = expect_register(opargs[1]);
				rs1 = expect_register(opargs[2]);
				rs2 = expect_register(opargs[3]);
				break;
			case FORMAT_M:
				rd = expect_register(opargs[1]);
				rs1 = expect_register(opargs[2]);
				imm = expect_immediate(opargs[3], labels, pass);
			case FORMAT_R:
				rd = expect_register(opargs[1]);
				rs1 = expect_register(opargs[2]);
				break;
			case FORMAT_I:
				rd = expect_register(opargs[1]);
				imm = expect_immediate(opargs[2], labels, pass);
				break;
			case FORMAT_J:
				imm = expect_immediate(opargs[1], labels, pass);
				break;
			case FORMAT_D:
				rd = expect_register(opargs[1]);
				break;
			case FORMAT_NONE:
				break;
			default:
				printf("Internal Error: Unknown Instruction format\n");
				exit(1);
		}

        // generate bytecode, technically we dont have to do this if we are in
        // pass 1 but like.. who cares.. we will just rewind() and overwrite
        // later so like does it even matter? the overhead is more than it's
        // worth
		uint32_t max_imm = (1 << 14) - 1;
		if (imm > max_imm) {
			// check for 32bit or 64bit extension
			rs2 = (imm > UINT32_MAX) + 1;
		}
		//printf("DEBUG: imm = %" PRIX64 "\n", imm);
		
		uint32_t instBC = 0;
		set_op(&instBC, opcode);
		set_rd(&instBC, rd);
		set_rs1(&instBC, rs1);
		set_rs2(&instBC, rs2);
		set_imm(&instBC, imm);

		// check for long immediates
		if (rs2 && instruction.format != FORMAT_F) { // value in rs2 when one shouldn't be expected
			set_imm(&instBC, 0); // set immediate to 0 for clarity
			switch (instruction.format) { // check imm extension is supported
				case FORMAT_J:
				case FORMAT_I:
				case FORMAT_M:
					break;
				default:
					printf("If you are seeing this something has gone seriously wrong and it's probably my fault.\n");
					exit(1);
			}
			printf("Instruction: %X (%dbit ext)\n", instBC, 32*rs2);
			emit_inst(instBC, bcFile, &pc);
			uint32_t immExt = (uint32_t)imm;
			printf("Imm extension: %X\n", immExt);
			emit_inst(immExt, bcFile, &pc);
			if (rs2 == 2) {
				immExt = (uint32_t)(imm >> 32);
				printf("Imm extension: %X\n", immExt);
				emit_inst(immExt, bcFile, &pc); // 64bit extension
			}
		} else {
			printf("Instruction: %X\n", instBC);
			emit_inst(instBC, bcFile, &pc);
		}

		// cleanup
		free(opargs_base);
		linec++;
	}

    // second pass
    if (pass < 2) {
        rewind(asmFile);
        rewind(bcFile);
        pass++;
        goto asm_pass;
    }

	free(line);
	fclose(asmFile);
	fclose(bcFile);
}
