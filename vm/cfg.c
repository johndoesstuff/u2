#include "cfg.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * cfg.c
 *
 * CONTROL FLOW GRAPH GENERATOR for REGISTER ALLOCATION
 */

/*
    STEP 1: CREATE ARRAY OF PARSED INSTRUCTIONS
*/

ParsedArray* init_parsed_array() {
    ParsedArray* parsed_array = malloc(sizeof(ParsedArray));
    parsed_array->capacity = 16;
    parsed_array->count = 0;
    parsed_array->instructions = malloc(sizeof(ParsedInstruction) * parsed_array->capacity);
    return parsed_array;
}

void push_parsed_array(ParsedArray* parsed_array, ParsedInstruction* instruction) {
    if (parsed_array->count == parsed_array->capacity) {
        parsed_array->capacity *= 2;
        parsed_array->instructions = realloc(parsed_array->instructions, sizeof(ParsedInstruction) * parsed_array->capacity);
    }
    parsed_array->instructions[parsed_array->count++] = instruction;
}

/*
   STEP 2: BREAK PARSED INSTRUCTIONS INTO BASIC BLOCKS

   To do this we need to figure out first what basic block is, for this context
   I will assume a basic block is an atomic set of instructions (no inner
   jumpable labels) that contains a MAXIMUM of one jump instruction as the last
   statement. To break our instruction array into a set of basic blocks we must
   first figure out all instructions that are jumpable by creating a jump
   table. Because forward jumping is allowed this must be done in a 2 pass
   system.
*/

int is_jump__(uint32_t opcode) {
    return opcode == U2_JMP || opcode == U2_JE || opcode == U2_JNE || opcode == U2_JL || opcode == U2_JG;
}

// signs immediates based on extension for relative jumping (which i have
// decided i am implementing as of 5 minutes ago)
int64_t sign_ext_imm__(uint64_t imm, uint32_t imm_ext) {
    switch (imm_ext) {
        case 0: // imm is default of uint14
            return (int16_t)(((imm & 0x3FFF) ^ 0x2000) - 0x2000);
        case 1: // imm is uint32
            return (int32_t)imm;
        case 2: // imm is uint64
            return imm;
        default:
            printf("Internal Error: Those who know (skull emoji)\n");
            exit(1);
    }
}

// remember, the only goal of this function is just to generate a jump table
// from the parsed array. this just means we have to match every jump and find
// where it lands
JumpTable* jumptable_from_parsed_array(ParsedArray* parsed_array) {
    // initialize
    JumpTable* jt = malloc(sizeof(JumpTable));
    jt->count = 0;
    jt->capacity = 16;
    jt->entries = malloc(sizeof(JumpTableEntry) * jt->capacity);
    for (unsigned int i = 0; i < parsed_array->count; i++) {
        uint32_t op = parsed_array->instructions[i]->opcode;
		printf("check if op is jump: %u (%s)\n", op, instruction_from_id(op));
        if (is_jump__(op)) {
			printf("add to jt!\n");
            uint64_t imm = parsed_array->instructions[i]->imm;
            uint32_t imm_ext = parsed_array->instructions[i]->imm_ext;
            JumpTableEntry* jte = malloc(sizeof(JumpTableEntry));
            jte->source_id = i;
            jte->target_id = i + (int64_t)sign_ext_imm__(imm, imm_ext);
            if (jt->count == jt->capacity) {
                jt->capacity *= 2;
                jt->entries = realloc(jt->entries, sizeof(JumpTableEntry) * jt->capacity);
            }
            jt->entries[jt->count++] = jte;
        }
    }

    return jt;
}
