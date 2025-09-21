#ifndef X86JIT_H
#define X86JIT_H

#include <stdint.h>

#define OPCODE_MOV_REG_REG  0x89
#define OPCODE_MOV_REG_IMM  0xB8

#define OPCODE_ADD_REG_REG  0x01
#define OPCODE_SUB_REG_REG  0x29

#define OPCODE_MUL_REG_REG  0xF7
#define OPCODE_DIV_REG_REG  0xF7

#define OPCODE_AND_REG_REG  0x21
#define OPCODE_OR_REG_REG   0x09
#define OPCODE_XOR_REG_REG  0x31

#define OPCODE_NOT_REG      0xF7

#define OPCODE_RET          0xC3

void emit_jit(uint8_t** jit_memory, uint32_t opcode, uint32_t rd, uint32_t rs1, uint32_t rs2, uint64_t imm);
void emit_x86ret(uint8_t** jit_memory);
void emit_x86ret_reg(uint8_t** jit_memory, uint32_t reg); // debugging

#endif
