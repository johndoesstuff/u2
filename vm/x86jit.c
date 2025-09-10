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
void emit_byte(char** jit_memory, unsigned char byte) {
	*(*jit_memory)++ = byte;
}

void emit_mov(char** jit_memory, unsigned int rd, unsigned int rs1) {
	// TODO: make this work lol
	emit_byte(jit_memory, REXW);
	emit_byte(jit_memory, OPCODE_MOV_REG_REG);
	return;
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
	}
}
