/**
    U2 Instruction Set
*/

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>

/**
 * InstructionFormat has 4 relevant bits, from 0-3 containing information about
 * which parts of the 32bit instruction are expected. 
 *
 * bit 0 -> rd: will the instruction store information in a register?
 *
 * bit 1 -> rs1: will the instruction require a register argument?
 *
 * bit 2 -> rs2: will the instruction require a second register argument?
 *
 * bit 3 -> immediate: will the instruction require a constant value?
 */
typedef uint8_t InstructionFormat;

typedef struct {
    InstructionFormat format;
    char* name;
} Instruction;

typedef struct {
    uint32_t opcode;
    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t imm_ext;
    uint64_t imm;
    Instruction obj;
} ParsedInstruction;

typedef enum {
    U2_MOV,
    U2_LI,
    U2_LD,
    U2_ST,
    U2_ADD,
    U2_SUB,
    U2_MUL,
    U2_DIV,
    U2_AND,
    U2_OR,
    U2_XOR,
    U2_NOT,
    U2_SHL,
    U2_SHR,
    U2_CMP,
    U2_JMP,
    U2_JE,
    U2_JNE,
    U2_JL,
    U2_JG,
} Opcode;

extern Instruction Instructions[];
extern const int Instruction_Count;
char* instruction_from_id(int id);

#endif
