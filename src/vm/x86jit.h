#ifndef X86JIT_H
#define X86JIT_H

#include <stdint.h>

void init_jit(uint8_t** jit_memory);
void free_jit(uint8_t** jit_memory);
void emit_jit(uint8_t** jit_memory, uint32_t opcode, uint32_t rd, uint32_t rs1, uint32_t rs2, uint64_t imm);
void emit_x86ret(uint8_t** jit_memory);
void emit_x86ret_reg(uint8_t** jit_memory, uint32_t reg); // debugging

#endif
