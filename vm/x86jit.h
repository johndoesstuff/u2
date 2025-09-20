#ifndef X86JIT_H
#define X86JIT_H

#include <stdint.h>

#define REX_BASE   0x40
#define REX_W      0x08
#define REX_R      0x04
#define REX_X      0x02
#define REX_B      0x01

#define REXW       (REX_BASE | REX_W)

#define OPCODE_MOV_REG_REG  0x89
#define OPCODE_MOV_REG_IMM  0xB8

#define OPCODE_ADD_REG_REG  0x01
#define OPCODE_SUB_REG_REG  0x29

// TODO: implement horrifying ModRM shit
// /4 -> MUL
// /6 -> DIV
#define OPCODE_MUL_REG_REG  0xF7
#define OPCODE_DIV_REG_REG  0xF7

#define OPCODE_AND_REG_REG  0x21
#define OPCODE_OR_REG_REG   0x09
#define OPCODE_XOR_REG_REG  0x31

#define OPCODE_NOT_REG      0xF7

#define OPCODE_RET          0xC3

#define MODRM(mod, reg, rm) ((uint8_t)(((mod) << 6) | (((reg) & 7) << 3) | ((rm) & 7)))

enum X86Reg {
	X86_SPILL = -1,
	X86_RAX = 0,
	X86_RCX = 1,
	X86_RDX = 2,
	X86_RBX = 3,
	X86_RSP = 4, // reserved
	X86_RBP = 5, // reserved
	X86_RSI = 6,
	X86_RDI = 7,
	X86_R8  = 8,
	X86_R9  = 9,
	X86_R10 = 10,
	X86_R11 = 11,
	X86_R12 = 12,
	X86_R13 = 13,
	X86_R14 = 14,
	X86_R15 = 15
};

void emit_jit(uint8_t** jit_memory, uint32_t opcode, uint32_t rd, uint32_t rs1, uint32_t rs2, uint64_t imm);
void emit_x86ret(uint8_t** jit_memory);
void emit_x86ret_reg(uint8_t** jit_memory, uint32_t reg); // debugging

#endif
