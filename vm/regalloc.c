#include "regalloc.h"
#include <stdint.h>

int u2a_regmap[16] = {
	_x86_RAX,
	_x86_RCX,
	_x86_RDX,
	_x86_RSI,
	_x86_RDI,
	_x86_R8,
	_x86_R9,
	_x86_R10,
	_x86_R11,
	_x86_SPILL,
	_x86_SPILL,
	_x86_SPILL,
	_x86_SPILL,
	_x86_SPILL,
	_x86_SPILL,
	_x86_SPILL,
};

_x86_register regalloc_u2a_x86(uint32_t reg) {
	// TODO: write a fucking register allocator
	return u2a_regmap[reg];
}

static uint64_t u2a_regspill[16] = {0};
