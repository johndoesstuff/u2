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

static _x86_register next_free() {
	for (int i = 0; i < regcount; i++) {
		_x86_regstate reg = regstate[i];
		if (!reg.busy) {
			return i;
		}
	}
	return _x86_SPILL;
}

static _x86_register eldest_reg() {
	int longest_lifetime = -1;
	_x86_register victim = -1;
	for (int i = 0; i < regcount; i++) {
		_x86_regstate reg = regstate[i];
		if (reg.lifetime > longest_lifetime) {
			victim = i;
			longest_lifetime = reg.lifetime;
		}
	}
	return victim;
}

static _x86_register find_reg(int vmreg) {
	if (vmreg == -1) return -1;
	for (int i = 0; i < regcount; i++) {
		_x86_regstate reg = regstate[i];
		if (reg.vmreg == vmreg) {
			return i;
		}
	}
	return _x86_SPILL;
}

static void spill_reg(uint8_t** jit_memory, _x86_register reg) {
	_x86_regstate state = regstate[reg];
	if (state.vmreg != -1 && state.dirty) {
		int vmreg = state.vmreg;
	}
}

_x86_register regalloc_u2a_x86(uint32_t vmreg) {
	// see if register has a home
	_x86_register reg = find_reg(vmreg);
	if (reg != -1) return reg;
	// search for unused registers
	reg = next_free();
	if (reg != -1) return reg;
	// no register is unused, spill oldest register
	_x86_register victim = eldest_reg();
	// TODO: actually implement this on the stack
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
