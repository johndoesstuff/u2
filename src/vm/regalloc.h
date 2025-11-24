#ifndef REGALLOC_H
#define REGALLOC_H

#include "x86encoding.h"

_x86_register regalloc_u2a_x86(uint32_t reg);
_x86_register getreg_u2a_x86(uint32_t reg);

typedef struct {
    int vmreg;
    int busy;
    int owner;
    int dirty;
    int lifetime;
} _x86_regstate;

extern _x86_regstate regstate[16];
void init_reg_spill_stack(uint8_t **jit_memory);
void free_reg_spill_stack(uint8_t **jit_memory);

#endif
