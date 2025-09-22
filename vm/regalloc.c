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

_x86_regstate regstate[] = {
	{ .vmreg = -1, .busy = 0, .dirty = 0, .lifetime = 0 }, // RAX
	{ .vmreg = -1, .busy = 0, .dirty = 0, .lifetime = 0 }, // RCX
	{ .vmreg = -1, .busy = 0, .dirty = 0, .lifetime = 0 }, // RDX
	{ .vmreg = -1, .busy = 0, .dirty = 0, .lifetime = 0 }, // RSI
	{ .vmreg = -1, .busy = 0, .dirty = 0, .lifetime = 0 }, // RDI
	{ .vmreg = -1, .busy = 0, .dirty = 0, .lifetime = 0 }, // R8
	{ .vmreg = -1, .busy = 0, .dirty = 0, .lifetime = 0 }, // R9
	{ .vmreg = -1, .busy = 0, .dirty = 0, .lifetime = 0 }, // R10
	{ .vmreg = -1, .busy = 0, .dirty = 0, .lifetime = 0 }, // R11
};

static int regcount = sizeof(u2a_regset)/sizeof(_x86_register);

static void _x86_register next_free(uint32_t vmreg) {
	// search for unused registers
	for (int i = 0; i < regcount; i++) {
		_x86_regstate reg = regstate[i];
		if (!reg.busy) {
			reg.busy = 1;
			reg.dirty = 1;
			reg.vmreg = vmreg;
			return;
		}
	}

	// no register is unused, spill oldest register
	int longest_lifetime = -1;
	_x86_register victim = -1;
	for (int i = 0; i < regcount; i++) {
		_x86_regstate reg = regstate[i];
		if (reg.lifetime > longest_lifetime) {
			victim = i;
		}
	}
}

_x86_register regalloc_u2a_x86(uint32_t reg) {
	// TODO: write a fucking register allocator
	return u2a_regmap[reg];
}

static uint64_t u2a_regspill[16] = {0};
