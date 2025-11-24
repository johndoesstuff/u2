#include "../common/config.h"
#include "../common/instruction.h"
#include "label.h"     // LabelTable for resolving label addresses
#include <ctype.h>     // tolower
#include <inttypes.h>  // PRIX64
#include <stdint.h>    // uint32_t, uint64_t, ...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // ftruncate

#include "error.c"

#include <errno.h>

extern int errno;

/**
    U2 ASSEMBLER

    This is the main file for the u2 assembler
    u2 assembly (*.u2a) -> u2 bytecode (*.u2b)

    Usage = u2asm asm.u2a bytecode.u2b
 */

// helper function to count the number of args in a line
int count_args(char *line) {
    int count = 0;
    for (char *clone = line; *clone != '\0'; clone++) {
        while (isspace(*clone))
            clone++;
        if (*clone == ';')
            return count;
        count++;
    }
    return count;
}

// each u2 bytecode instruction is 32 bits and is broken into different
// components for different bit ranges, these functions set those bit ranges to
// desired values for each parameter. for more information see vm/vm.txt
void set_bit_range(uint32_t *instruction, uint32_t value, int start, int length) {
    uint32_t mask = ((1 << length) - 1u) << start;
    *instruction &= ~mask;
    *instruction |= (value & ((1 << length) - 1)) << start;
}

void set_op(uint32_t *instruction, uint32_t opcode) {
    set_bit_range(instruction, opcode, 26, 6);
}

void set_rd(uint32_t *instruction, uint32_t rd) {
    set_bit_range(instruction, rd, 22, 4);
}

void set_rs1(uint32_t *instruction, uint32_t rs1) {
    set_bit_range(instruction, rs1, 18, 4);
}

void set_rs2(uint32_t *instruction, uint32_t rs2) {
    set_bit_range(instruction, rs2, 14, 4);
}

void set_imm(uint32_t *instruction, uint64_t imm) {
    set_bit_range(instruction, (uint32_t)imm, 0, 14);
}

void emit_inst(uint32_t inst, FILE *fptr, uint32_t *pc) {
    // important to note is pc is relative to each 32bit segment, it would be
    // silly to give pc byte-level precision since all instructions are 4bytes
    (*pc)++;
    fwrite(&inst, sizeof(uint32_t), 1, fptr);
}

// expect_register is responsible for returning a uint32_t from a register
// name. registers can be named r1-r16 but the actual number of a register is
// only 0-15. as such registers are just stripped of 'r' and decremented
uint32_t expect_register(char *reg) {
    char *regc = reg;
    if (reg == NULL) {
        printf("Internal Error: Invalid Register\n");
        exit(1);
    }
    if (tolower(*regc) != 'r') {
        printf("Invalid Register '%s', expected [r1-r16]\n", reg);
        exit(1);
    }
    regc++;

    char *endptr;
    int64_t ret = strtoull(regc, &endptr, 10);
    if (*endptr != '\0') {
        printf("Invalid Register, unexpected character '%c' in %s\n", *endptr, reg);
        exit(1);
    }

    // atoi fail or invalid reg range
    if (ret > 16 || ret <= 0) {
        printf("Invalid Register '%s', expected [r1-r16]\n", reg);
        exit(1);
    }
    return (uint32_t)ret;
}

// expect_immediate handles all possible immediate values, this includes hex,
// binary, and labels, and will resolve them into a *signed* 64 bit integer
// (since immediates can be stored up to 64bits and can be negative for
// relative addressing)
int64_t expect_immediate(char *immediate, LabelTable *labels, int pass, uint64_t pc) {
    if (immediate == NULL) {
        printf("Internal Error: Invalid Immediate\n");
        exit(1);
    }

    int is_neg = *immediate == '-';
    char *immediate_num = immediate;
    if (is_neg)
        immediate_num++;

    char *endptr;
    int base = 10;

    // lets cover all our bases! heh..
    if (strlen(immediate_num) > 2 && immediate_num[0] == '0') {
        if (immediate_num[1] == 'x' || immediate_num[1] == 'X') {
            base = 16;
            immediate_num += 2;  // skip 0x
        } else if (immediate_num[1] == 'b' || immediate_num[1] == 'B') {
            base = 2;
            immediate_num += 2;  // skip 0b
        }
    }

    int64_t val = strtoll(immediate_num, &endptr, base);
    if (errno == ERANGE) {
        perror("Immediate out of range");
        exit(1);
    }

    if (*endptr != '\0') {
        // wait wait it could be a label.. we should wait for 2nd pass until
        // making any final decisions
        if (pass != 2)
            return 0;

        int fl = find_label(labels, immediate);
        if (fl < 0) {  // we've done all we can.. give up :(
            printf("Invalid Immediate, unexpected character '%c' in %s\n", *endptr, immediate);
            exit(1);
        }
        // reformat label pc to be relative to current position (all labels are
        // relative addressing) the issue with this is negative immediates
        // arent supported so we need to make our 14 bit imms signed for jumps
        // and whatnot
        int64_t rel = (int64_t)fl - (int64_t)pc;
        return rel;  // since this is technically a signed cast to
                     // unsigned this needs to be resolved later
                     // during bytecode emission!  immediates should
                     // be handled as signed values for their relative
                     // size (14bit signed, 32bit signed and 64bit
                     // signed respectively)
    }

    return is_neg ? -val : val;
}

// get the first index in a char* of a char
int get_first_char(char *str, char ch) {
    // return -1 if not found
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == ch)
            return i;
    }
    return -1;
}

int main(int argc, char **argv) {
    // correct usage check
    if (argc < 3) {
        printf("Usage: u2asm assembly.u2a bytecode.u2b\n");
        exit(1);
    }

    // try to open file
    char *asmPath = argv[1];
    FILE *asmFile = fopen(asmPath, "r");
    if (asmFile == NULL) {
        fprintf(stderr, "Error opening file '%s': %s\n", asmPath, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // try to open output
    char *bcPath = argv[2];
    FILE *bcFile = fopen(bcPath, "wb");
    if (bcFile == NULL) {
        fprintf(stderr, "Error opening file '%s': %s\n", bcPath, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // initialize
    char *line = NULL;
    size_t linec = 0;
    size_t len;
    ssize_t read;
    LabelTable *labels = new_label_table();
    /*
     * 2 PASS SYSTEM
     *
     * First pass will just inc pc and set label addr for LabelTable* labels
     * Se/cond pass will use labels from labels to resolve addresses
     *
     * This system is necessary to resolve forward declared labels
     */
    int pass = 1;

asm_pass:
    uint32_t pc = 0;
    while ((read = getline(&line, &len, asmFile)) != -1) {
        // remove \n from line
        line[read - 1] = '\0';

        // ignore past ;
        int comment = get_first_char(line, ';');
        if (comment != -1) {
            line[comment] = '\0';
        }

        // get op components
        char **opargs = malloc(sizeof(char *) * (count_args(line) + 1));
        char **opargs_base = opargs;
        int opargsc = 0;
        char *pch = line;

        // ignore whitespace and tokenize arguments
        while (*pch) {
            while (isspace(*pch))
                pch++;
            if (*pch == '\0')
                break;
            opargs[opargsc++] = pch;

            while (*pch && !isspace(*pch))
                pch++;

            if (*pch) {
                *pch = '\0';
                pch++;
            }
            printf("Found arg: %s\n", opargs[opargsc - 1]);
        }

        // if line is empty ignore
        if (opargsc == 0)
            continue;

        // might be label, not op. check if last character is a ':'
        if (opargs[0][strlen(opargs[0]) - 1] == ':') {
            if (opargsc != 1) {
                // why would you include something after the label??
                printf("Adding instructions in the same line as a label isn't"
                       " currently supported.\n");
                exit(1);
            }
            if (pass == 1) {
                // remove ending ':'
                char *label_str = strdup(opargs[0]);
                label_str[strlen(label_str) - 1] = '\0';
                add_label(labels, label_str, pc);
                printf("Added label %s\n", label_str);
            }
            continue;
        }

        // search for op
        int opcode = -1;
        for (int i = 0; i < Instruction_Count; i++) {
            // convert to lower (ops arent case sensitive)
            for (char *t = opargs[0]; *t; ++t)
                *t = tolower(*t);
            if (strcmp(Instructions[i].name, opargs[0]) == 0) {
                opcode = i;
                break;
            }
        }

        if (opcode == -1) {
            printf("Unknown Instruction \"%s\"\n", opargs[0]);
            exit(1);
        }

        // correct format?
        Instruction instruction = Instructions[opcode];
        switch (instruction.format) {
        case FORMAT_F:
            if (opargsc != 4)
                error_argnum(opargsc, 4, instruction.name, linec);
            break;
        case FORMAT_M:
            if (opargsc != 4)
                error_argnum(opargsc, 4, instruction.name, linec);
            break;
        case FORMAT_R:
            if (opargsc != 3)
                error_argnum(opargsc, 3, instruction.name, linec);
            break;
        case FORMAT_I:
            if (opargsc != 3)
                error_argnum(opargsc, 3, instruction.name, linec);
            break;
        case FORMAT_J:
            if (opargsc != 2)
                error_argnum(opargsc, 2, instruction.name, linec);
            break;
        case FORMAT_D:
            if (opargsc != 2)
                error_argnum(opargsc, 2, instruction.name, linec);
            break;
        case FORMAT_NONE:
            if (opargsc != 1)
                error_argnum(opargsc, 1, instruction.name, linec);
            break;
        }

        uint32_t rd = 0;
        uint32_t rs1 = 0;
        uint32_t rs2 = 0;
        int64_t imm = 0;

        switch (instruction.format) {
        case FORMAT_F:
            rd = expect_register(opargs[1]);
            rs1 = expect_register(opargs[2]);
            rs2 = expect_register(opargs[3]);
            break;
        case FORMAT_M:
            rd = expect_register(opargs[1]);
            rs1 = expect_register(opargs[2]);
            imm = expect_immediate(opargs[3], labels, pass, pc);
            break;
        case FORMAT_R:
            rd = expect_register(opargs[1]);
            rs1 = expect_register(opargs[2]);
            break;
        case FORMAT_I:
            rd = expect_register(opargs[1]);
            imm = expect_immediate(opargs[2], labels, pass, pc);
            break;
        case FORMAT_J:
            imm = expect_immediate(opargs[1], labels, pass, pc);
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

        // do we need to extend immediate?
        int32_t max_14imm = (1LL << 13) - 1;
        int32_t min_14imm = -(1LL << 13);
        int imm_size = 0;  // 0 = 14b, 1 = 32b, 2 = 64b
        if (imm <= max_14imm && imm >= min_14imm) {
            imm_size = 0;
        } else if (imm <= INT32_MAX && imm >= INT32_MIN) {
            imm_size = 1;
        } else {
            imm_size = 2;
        }
        rs2 = imm_size;  // imm size is safe to store in rs2 because no
                         // instruction uses both imm and rs2 that would require
                         // imm extension, for more info see InstructionFormat
                         // at common/instruction.h

        uint32_t instBC = 0;
        set_op(&instBC, opcode);
        set_rd(&instBC, rd);
        set_rs1(&instBC, rs1);
        set_rs2(&instBC, rs2);
        set_imm(&instBC, imm & 0x3FFF);

        // check for long immediates
        if (imm_size) {
            set_imm(&instBC, 0);           // set immediate to 0 for clarity (extended
                                           // imm means imm will not be read from this
                                           // instruction)
            switch (instruction.format) {  // check imm extension is supported
            case FORMAT_J:
            case FORMAT_I:
            case FORMAT_M:
                break;
            default:
                printf("If you are seeing this something has gone seriously"
                       " wrong and it's probably my fault.\n");
                exit(1);
            }
            printf("Instruction: %X (%dbit ext)\n", instBC, 32 * imm_size);
            emit_inst(instBC, bcFile, &pc);
            int32_t imm_ext = (int32_t)(imm & 0xFFFFFFFF);
            printf("Imm extension: %X\n", imm_ext);
            emit_inst((uint32_t)imm_ext, bcFile, &pc);
            if (imm_size == 2) {
                imm_ext = (int32_t)((imm >> 32) & 0xFFFFFFFF);
                printf("Imm extension: %X\n", imm_ext);
                emit_inst((uint32_t)imm_ext, bcFile, &pc);  // 64bit extension
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
        ftruncate(fileno(bcFile), 0);  // just to make sure we dont somehow write extra bytes
        pass++;
        goto asm_pass;
    }

    free(line);
    fclose(asmFile);
    fclose(bcFile);
}
