/**
  
	Welcome to hell

	TODO: Register mapping
	r1    ->   rax
	r2    ->   rbx
	r3    ->   rcx
	r4    ->   rdx
	r5    ->   rdi
	r6    ->   rsi
	r7    ->   r8
	r8    ->   r9
	r9    ->   r10
	r10   ->   r11
	r11   ->   r12
	r12   ->   r13
	r13   ->   r14
	r14   ->   r15
	r15   ->   spill
	r16   ->   spill

 */

#include "x86jit.h"
#include <stdio.h>
#include <stdlib.h>

int VMRegMap[16] = {
	X86_RAX,
	X86_RCX,
	X86_RDX,
	X86_RSI,
	X86_RDI,
	X86_R8,
	X86_R9,
	X86_R10,
	X86_R11,
	X86_SPILL,
	X86_SPILL,
	X86_SPILL,
	X86_SPILL,
	X86_SPILL,
	X86_SPILL,
	X86_SPILL,
};

uint64_t VMRegSpill[16] = {0};

void emit_byte(char** jit_memory, unsigned char byte) {
	*(*jit_memory)++ = byte;
}

void emit_rex(char **buf, int w, int reg, int rm) {
	uint8_t rex = REX_BASE;
	if (w) rex |= REX_W;       // 64-bit
	if (reg & 8) rex |= REX_R; // R
	if (rm  & 8) rex |= REX_B; // B
	if (rex != REX_BASE) emit_byte(buf, rex);
}

void emit_mov(char** jit_memory, unsigned int rd, unsigned int rs1) {
	int dst = VMRegMap[rd];
	int src = VMRegMap[rs1];

	// register to register
	if (dst != X86_SPILL && src != X86_SPILL) {
		emit_rex(jit_memory, 1, src, dst);
		emit_byte(jit_memory, OPCODE_MOV_REG_REG);
		emit_byte(jit_memory, MODRM(0b11, src, dst));
	} else {
		printf("TODO: implement this lol\n");
		exit(1);
	}

	// TODO: implement spill
}

void emit_li(char** jit_memory, unsigned int rd, unsigned int imm) {
	// TODO: implement li
	int dst = VMRegMap[rd];
	
	if (dst != X86_SPILL) {
		// emit mov imm
	} else {
		printf("TODO: implement this aswell\n");
		exit(1);
	}
}

void emit_jit(char** jit_memory,
		unsigned int opcode,
		unsigned int rd,
		unsigned int rs1,
		unsigned int rs2,
		unsigned int imm) {
	switch (opcode) {
		case 0:
			emit_mov(jit_memory, rd, rs1);
			break;
		case 1:
			emit_li(jit_memory, rd, imm);
			break;
	}
}
