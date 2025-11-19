/**
  
    Welcome to true hell

    This file provides the backend for x86jit.c
    and translates x86 instructions into
    emitted bytecode. For u2a to x86 instructions
    visit x86jit.c

 */

#include "x86encoding.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Encodings!

    Encoding structure defined in x86encoding.h
    and u2b register mapping defined in x86jit.c
*/

_x86_encoding __mov_r32_imm32 = {
    .opcode        = 0xB8,
    .opcode_ext    = -1,
    .needs_rex_w   = 0,
    .imm_size      = 4,
    .reg_in_opcode = 1
};

_x86_encoding __mov_r64_imm64 = {
    .opcode        = 0xB8,
    .opcode_ext    = -1,
    .needs_rex_w   = 1,
    .imm_size      = 8,
    .reg_in_opcode = 1
};

_x86_encoding __mov_rm64_r64 = {
    .opcode        = 0x89,
    .opcode_ext    = -2,
    .needs_rex_w   = 1,
    .imm_size      = 0,
    .reg_in_opcode = 0
};

_x86_encoding __ret = {
    .opcode        = 0xC3,
    .opcode_ext    = -1,
    .needs_rex_w   = 0,
    .imm_size      = 0,
    .reg_in_opcode = 0
};

void emit_byte(uint8_t** jit_memory, uint8_t byte) {
    *(*jit_memory)++ = byte;
}

void emit_rex(uint8_t** jit_memory, uint32_t w, uint32_t reg, uint32_t rm) {
    uint8_t rex = REX_BASE;
    if (w) rex |= REX_W;       // 64-bit
    if (reg & 8) rex |= REX_R; // R
    if (rm  & 8) rex |= REX_B; // B
    if (rex != REX_BASE) emit_byte(jit_memory, rex);
}

void emit_modrm(uint8_t** jit_memory, uint8_t mod, uint8_t reg, uint8_t rm) {
    uint8_t modrm = (uint8_t)(((mod & 0x3) << 6) |
            ((reg & 0x7) << 3) |
            (rm & 0x7));
    emit_byte(jit_memory, modrm);
}

void emit_x86instruction(uint8_t** jit_memory, _x86_encoding* encoding, uint32_t reg, uint32_t rm, uint64_t imm) {
    if (encoding->needs_rex_w || reg >= _x86_R8 || rm >= _x86_R8) {
        emit_rex(jit_memory, encoding->needs_rex_w, reg, rm);
    }

    if (encoding->reg_in_opcode) {
        emit_byte(jit_memory, encoding->opcode | (reg & 7));
    } else {
        emit_byte(jit_memory, encoding->opcode);
    }

    if (encoding->opcode_ext >= 0) {
        emit_modrm(jit_memory, 0b11, encoding->opcode_ext, rm & 7);
    } else if (encoding->opcode_ext == -2) {
        emit_modrm(jit_memory, 0b11, reg & 7, rm & 7);
    }

    for (int i = 0; i < encoding->imm_size; i++) {
        emit_byte(jit_memory, (imm >> (i * 8)) & 0xFF);
    }
}
