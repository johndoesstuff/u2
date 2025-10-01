/**
	U2 Instruction Set
*/

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>

enum {
	FORMAT_F,    // 3 register :                      ins r0 r1 r2
	FORMAT_M,    // register to register + offset :   ins r0 r1 im
	FORMAT_R,    // register to register :            ins r0 r1
	FORMAT_I,    // immediate to register :           ins r0 im
	FORMAT_J,    // immediate opcode :                ins im
	FORMAT_D,    // register opcode :                 ins r0
	FORMAT_NONE, // opcode :                          ins
} typedef InstructionFormat;

struct {
	InstructionFormat format; 
	char* name;
} typedef Instruction;

typedef struct {
	uint32_t opcode;
	uint32_t rd;
	uint32_t rs1;
	uint32_t rs2;
	uint32_t imm_ext;
	uint64_t imm;
	Instruction obj;
} ParsedInstruction;

enum {
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
} typedef Opcode;

extern Instruction Instructions[];
extern const int Instruction_Count;

#endif
