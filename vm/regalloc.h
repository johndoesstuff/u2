#ifndef REGALLOC_H
#define REGALLOC_H

#include "x86encoding.h"

extern int VMRegMap[16];
_x86_register regalloc_u2a_x86(uint32_t reg);

typedef struct {
	int busy;
	int owner;
	int disty;
} _x86_regstate;

extern _x86_regstate regstate[16];

#endif
