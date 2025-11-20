#include "regalloc.h"
#include <stdint.h>

_x86_register u2a_regset[] = {
    _x86_RAX,
    _x86_RCX,
    _x86_RDX,
    _x86_RSI,
    _x86_RDI,
    _x86_R8,
    _x86_R9,
    _x86_R10,
    _x86_R11,
};

static int regcount = sizeof(u2a_regset)/sizeof(_x86_register);

static void spill_reg(uint8_t** jit_memory, _x86_register reg) {
    // TODO
    (void)jit_memory;
    (void)reg;
}

_x86_register regalloc_u2a_x86(uint32_t reg) {
    // TODO
    (void)reg;
    return _x86_RAX;
}

void init_reg_spill_stack(uint8_t** jit_memory) {
    // sub rsp, 16 * 8 (alloc 16 regs of 8 bytes on rsp)
    emit_byte(jit_memory, 0x48);
    emit_byte(jit_memory, 0x81);
    emit_byte(jit_memory, 0xec);
    emit_byte(jit_memory, 0x80);
    emit_byte(jit_memory, 0x00);
    emit_byte(jit_memory, 0x00);
    emit_byte(jit_memory, 0x00);
}

void free_reg_spill_stack(uint8_t** jit_memory) {
    // add rsp, 16 * 8 (free 16 regs of 8 bytes on rsp)
    emit_byte(jit_memory, 0x48);
    emit_byte(jit_memory, 0x81);
    emit_byte(jit_memory, 0xc4);
    emit_byte(jit_memory, 0x80);
    emit_byte(jit_memory, 0x00);
    emit_byte(jit_memory, 0x00);
    emit_byte(jit_memory, 0x00);
}
