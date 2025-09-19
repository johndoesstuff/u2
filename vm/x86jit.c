/**
  
	Welcome to hell

 */

#include "x86jit.h"
#include "../common/instruction.h"
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

static inline void emit_byte(char** jit_memory, uint8_t byte) {
	*(*jit_memory)++ = byte;
}

void emit_rex(char **buf, int w, int reg, int rm) {
	uint8_t rex = REX_BASE;
	if (w) rex |= REX_W;       // 64-bit
	if (reg & 8) rex |= REX_R; // R
	if (rm  & 8) rex |= REX_B; // B
	if (rex != REX_BASE) emit_byte(buf, rex);
}

void emit_x86ret(char** jit_memory) {
	emit_byte(jit_memory, OPCODE_RET);
}

static inline void emit_x86reg_reg(char **jit_memory, uint8_t opcode, int reg, int rm) {
	emit_rex(jit_memory, 1, reg, rm);
	emit_byte(jit_memory, opcode);
	emit_byte(jit_memory, MODRM(0b11, reg, rm));
}

// just for debugging purposes
void emit_x86ret_reg(char **jit_memory, int reg) {
	int src = VMRegMap[reg];

	emit_x86reg_reg(jit_memory, OPCODE_MOV_REG_REG, src, X86_RAX);

	emit_byte(jit_memory, OPCODE_RET); // RET
}

void emit_binary_rr(char **jit_memory, uint8_t opcode, int dst, int lhs, int rhs) {
	if (dst == X86_SPILL || lhs == X86_SPILL || rhs == X86_SPILL) {
		printf("TODO: implement spill\n");
		exit(1);
	}

	if (dst == lhs) {
		// emit rhs to dst
		emit_x86reg_reg(jit_memory, opcode, rhs, dst);
	} else if (dst == rhs) {
		// emit lhs to dst
		emit_x86reg_reg(jit_memory, opcode, lhs, dst);
	} else {
		// rd is unique, mov lhs into rd then add rhs
		emit_x86reg_reg(jit_memory, OPCODE_MOV_REG_REG, lhs, dst);
		emit_x86reg_reg(jit_memory, opcode, rhs, dst);
	}
}

void emit_mov(char** jit_memory, unsigned int rd, unsigned int rs1) {
	int dst = VMRegMap[rd];
	int src = VMRegMap[rs1];

	// register to register
	if (dst != X86_SPILL && src != X86_SPILL) {
		emit_x86reg_reg(jit_memory, OPCODE_MOV_REG_REG, src, dst);
	} else {
		printf("TODO: implement this lol\n");
		exit(1);
	}

	// TODO: implement spill
}

void emit_li(char** jit_memory, unsigned int rd, unsigned int imm) {
	int dst = VMRegMap[rd];
	
	if (dst != X86_SPILL) {
		emit_rex(jit_memory, 1, 0, dst);
		emit_byte(jit_memory, OPCODE_MOV_REG_IMM + (dst & 7));

		for (int i = 0; i < 8; i++) emit_byte(jit_memory, (imm >> (i * 8)) & 0xFF);
	} else {
		printf("TODO: implement this aswell\n");
		exit(1);
	}
}

void emit_add(char** jit_memory, unsigned int rd, unsigned int rs1, unsigned int rs2) {
	emit_binary_rr(jit_memory, OPCODE_ADD_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_sub(char** jit_memory, unsigned int rd, unsigned int rs1, unsigned int rs2) {
	emit_binary_rr(jit_memory, OPCODE_SUB_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_mul(char** jit_memory, unsigned int rd, unsigned int rs1, unsigned int rs2) {
	emit_binary_rr(jit_memory, OPCODE_MUL_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_div(char** jit_memory, unsigned int rd, unsigned int rs1, unsigned int rs2) {
	emit_binary_rr(jit_memory, OPCODE_DIV_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_and(char** jit_memory, unsigned int rd, unsigned int rs1, unsigned int rs2) {
	emit_binary_rr(jit_memory, OPCODE_AND_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_or(char** jit_memory, unsigned int rd, unsigned int rs1, unsigned int rs2) {
	emit_binary_rr(jit_memory, OPCODE_OR_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_xor(char** jit_memory, unsigned int rd, unsigned int rs1, unsigned int rs2) {
	emit_binary_rr(jit_memory, OPCODE_XOR_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_jit(char** jit_memory,
		unsigned int opcode,
		unsigned int rd,
		unsigned int rs1,
		unsigned int rs2,
		unsigned int imm) {
	Opcode op = (Opcode)opcode;
	switch (opcode) {
		// TODO: modularly enum opcodes based on common instruction.h
		case U2_MOV:
			emit_mov(jit_memory, rd, rs1);
			break;
		case U2_LI:
			emit_li(jit_memory, rd, imm);
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
		default:
			printf("Instruction not implemented yet!\n");
			exit(1);
	}
}
