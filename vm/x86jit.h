#ifndef X86JIT_H
#define X86JIT_H

#define REX_BASE   0x40
#define REX_W      0x08
#define REX_R      0x04
#define REX_X      0x02
#define REX_B

#define REXW       (REX_BASE | REX_W)

#define OPCODE_MOV_REG_IMM  0xB8
#define OPCODE_MOV_REG_REG  0x89
#define OPCODE_ADD_REG_REG  0x01
#define OPCODE_RET          0xC3

#define MODRM(mod, reg, rm) ((uint8_t)(((mod) << 6) | (((reg) & 7) << 3) | ((rm) & 7)))

void emit_jit(char** jit_memory, unsigned int opcode, unsigned int rd, unsigned int rs1, unsigned int rs2, unsigned int imm);

#endif
