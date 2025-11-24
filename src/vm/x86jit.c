/**

    Welcome to hell

    The purpose of this file is to provide an
    interface to emit u2a instructions to x86
    instructions. This is the front end of that
    process, for actual encodings visit
    x86encoding.c

 */

#include "x86jit.h"
#include "../common/instruction.h"
#include "regalloc.h"
#include "x86encoding.h"
#include <stdio.h>
#include <stdlib.h>

/**
    To convert u2 bytecode to x86 all u2 registers
    r1-r16 must be mapped to valid (and
    non-destructive) x86 registers. Spill registers
    are handled as memory instead of registers.
*/

void emit_x86ret(uint8_t** jit_memory) {
    emit_x86instruction(jit_memory, &__ret, 0, 0, 0);
}

void emit_x86ret_reg(uint8_t** jit_memory, uint32_t rd) {
    int dst = regalloc_u2a_x86(rd - 1);
    emit_x86instruction(jit_memory, &__mov_rm64_r64, dst, 0, 0);
    emit_x86instruction(jit_memory, &__ret, dst, 0, 0);
}

void emit_mov(uint8_t** jit_memory, uint32_t rd, uint32_t rs1) {
    int dst = regalloc_u2a_x86(rd);
    int src = regalloc_u2a_x86(rs1);

    // genius optimization
    if (dst == src)
        return;

    // register to register
    if (dst != _x86_SPILL && src != _x86_SPILL) {
        emit_x86instruction(jit_memory, &__mov_rm64_r64, dst, src, 0);
    } else {
        printf("TODO: implement this lol\n");
        exit(1);
    }

    // TODO: implement spill
}

void emit_li(uint8_t** jit_memory, uint32_t rd, uint64_t imm) {
    int dst = regalloc_u2a_x86(rd);

    if (dst != _x86_SPILL) {
        if (imm <= UINT32_MAX) {
            // 32bit load imm
            emit_x86instruction(jit_memory, &__mov_r32_imm32, dst, 0, imm);
        } else {
            // 64bit load imm
            emit_x86instruction(jit_memory, &__mov_r64_imm64, dst, 0, imm);
        }
    } else {
        printf("TODO: implement this aswell\n");
        exit(1);
    }
}

void emit_ld(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint64_t imm) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
    (void)imm;
}

void emit_st(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint64_t imm) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
    (void)imm;
}

void emit_add(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
    (void)rs2;
}

void emit_sub(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
    (void)rs2;
}

void emit_mul(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
    (void)rs2;
}

void emit_div(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
    (void)rs2;
}

void emit_and(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
    (void)rs2;
}

void emit_or(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
    (void)rs2;
}

void emit_xor(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
    (void)rs2;
}

void emit_not(uint8_t** jit_memory, uint32_t rd, uint32_t rs1) {
    // TODO
    (void)jit_memory;
    (void)rd;
    (void)rs1;
}

void init_jit(uint8_t** jit_memory) {
    init_reg_spill_stack(jit_memory);
}

void free_jit(uint8_t** jit_memory) {
    free_reg_spill_stack(jit_memory);
}

void emit_jit(uint8_t** jit_memory, uint32_t opcode, uint32_t rd, uint32_t rs1, uint32_t rs2, uint64_t imm) {
    Opcode op = (Opcode)opcode;
    switch (op) {
    // TODO: modularly enum opcodes based on common instruction.h
    case U2_MOV:
        emit_mov(jit_memory, rd, rs1);
        break;
    case U2_LI:
        emit_li(jit_memory, rd, imm);
        break;
    case U2_LD:
        emit_ld(jit_memory, rd, rs1, imm);
        break;
    case U2_ST:
        emit_st(jit_memory, rd, rs1, imm);
        break;
    case U2_ADD:
        emit_add(jit_memory, rd, rs1, rs2);
        break;
    case U2_SUB:
        emit_sub(jit_memory, rd, rs1, rs2);
        break;
    case U2_MUL:
        emit_mul(jit_memory, rd, rs1, rs2);
        break;
    case U2_DIV:
        emit_div(jit_memory, rd, rs1, rs2);
        break;
    case U2_AND:
        emit_and(jit_memory, rd, rs1, rs2);
        break;
    case U2_OR:
        emit_or(jit_memory, rd, rs1, rs2);
        break;
    case U2_XOR:
        emit_xor(jit_memory, rd, rs1, rs2);
        break;
    case U2_NOT:
        emit_not(jit_memory, rd, rs1);
        break;
    default:
        printf("Instruction %u (%s) not implemented yet!\n", op, instruction_from_id(op));
        exit(1);
    }
}
