#ifndef X86ENCODING_H
#define X86ENCODING_H

#include <stdint.h>

#define REX_BASE   0x40
#define REX_W      0x08
#define REX_R      0x04
#define REX_X      0x02
#define REX_B      0x01

typedef enum {
    _x86_SPILL = -1,
    _x86_RAX = 0,
    _x86_RCX = 1,
    _x86_RDX = 2,
    _x86_RBX = 3,
    _x86_RSP = 4, // reserved
    _x86_RBP = 5, // reserved
    _x86_RSI = 6,
    _x86_RDI = 7,
    _x86_R8  = 8,
    _x86_R9  = 9,
    _x86_R10 = 10,
    _x86_R11 = 11,
    _x86_R12 = 12,
    _x86_R13 = 13,
    _x86_R14 = 14,
    _x86_R15 = 15
} _x86_register;

typedef struct {
    uint8_t opcode;
    int opcode_ext;     // -2: reg/rm, -1: none, else modrm /digit
    int needs_rex_w;
    uint8_t imm_size;   // number of bytes to append
    int reg_in_opcode;  // stupid shit like b8+rd
} _x86_encoding;

void emit_byte(uint8_t** jit_memory, uint8_t byte);
void emit_x86instruction(uint8_t** jit_memory, _x86_encoding* encoding, uint32_t reg, uint32_t rm, uint64_t imm);

extern _x86_encoding __mov_r64_imm64;
extern _x86_encoding __mov_r32_imm32;
extern _x86_encoding __mov_rm64_r64;
extern _x86_encoding __ret;

#endif
