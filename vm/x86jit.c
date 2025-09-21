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

static inline void emit_byte(uint8_t** jit_memory, uint8_t byte) {
	*(*jit_memory)++ = byte;
}

void emit_rex(uint8_t** buf, uint32_t w, uint32_t reg, uint32_t rm) {
	uint8_t rex = REX_BASE;
	if (w) rex |= REX_W;       // 64-bit
	if (reg & 8) rex |= REX_R; // R
	if (rm  & 8) rex |= REX_B; // B
	if (rex != REX_BASE) emit_byte(buf, rex);
}

void emit_x86ret(uint8_t** jit_memory) {
	emit_byte(jit_memory, OPCODE_RET);
}

static inline void emit_x86reg(uint8_t** jit_memory, uint8_t opcode, uint32_t reg) {
	emit_rex(jit_memory, 1, reg, 0);
	emit_byte(jit_memory, opcode);
	emit_byte(jit_memory, 0xD0 | (reg & 7));
}

static inline void emit_x86reg_reg(uint8_t** jit_memory, uint8_t opcode, uint32_t reg, uint32_t rm) {
	emit_rex(jit_memory, 1, reg, rm);
	emit_byte(jit_memory, opcode);
	emit_byte(jit_memory, MODRM(0b11, reg, rm));
}

// just for debugging purposes
void emit_x86ret_reg(uint8_t** jit_memory, uint32_t reg) {
	int src = VMRegMap[reg - 1];

	emit_x86reg_reg(jit_memory, OPCODE_MOV_REG_REG, src, X86_RAX);

	emit_byte(jit_memory, OPCODE_RET); // RET
}

void emit_binary_rr(uint8_t** jit_memory, uint8_t opcode, uint32_t dst, uint32_t lhs, uint32_t rhs) {
	if (dst == X86_SPILL || lhs == X86_SPILL || rhs == X86_SPILL) {
		printf("TODO: implement spill\n");
		exit(1);
	}

	if (dst == lhs) {
		// emit rhs to dst
		emit_x86reg_reg(jit_memory, opcode, rhs, dst);
	} else { // dst == rhs could optimize but only if commutative
		// rd is unique, mov lhs into rd then add rhs
		emit_x86reg_reg(jit_memory, OPCODE_MOV_REG_REG, lhs, dst);
		emit_x86reg_reg(jit_memory, opcode, rhs, dst);
	}
}

void emit_unary_rr(uint8_t** jit_memory, uint8_t opcode, uint32_t dst, uint32_t src) {
	if (dst == X86_SPILL || src == X86_SPILL) {
		printf("TODO: implement spill\n");
		exit(1);
	}

	if (dst == src) {
		// emit rhs to dst
		emit_x86reg(jit_memory, opcode, dst);
	} else {
		// rd is unique, mov lhs into rd then add rhs
		emit_x86reg_reg(jit_memory, OPCODE_MOV_REG_REG, src, dst);
		emit_x86reg(jit_memory, opcode, dst);
	}
}

void emit_mov(uint8_t** jit_memory, uint32_t rd, uint32_t rs1) {
	int dst = VMRegMap[rd];
	int src = VMRegMap[rs1];

	// genius optimization
	if (dst == src) return;

	// register to register
	if (dst != X86_SPILL && src != X86_SPILL) {
		emit_x86reg_reg(jit_memory, OPCODE_MOV_REG_REG, src, dst);
	} else {
		printf("TODO: implement this lol\n");
		exit(1);
	}

	// TODO: implement spill
}

void emit_li(uint8_t** jit_memory, uint32_t rd, uint64_t imm) {
	int dst = VMRegMap[rd];

	// on x86 registers r8, r9, r10, ... are mapped to rax, rbx, rcx, ... with extra
	// things for some reason
	int reg86_special = dst >= 8;
	int reg86_base = dst % 8;
	
	if (dst != X86_SPILL) {
		if (imm <= UINT32_MAX) {
			// 32bit load imm
			if (reg86_special) emit_byte(jit_memory, 0x41);
			emit_byte(jit_memory, 0xB8 | reg86_base);
			for (int i = 0; i < 4; i++) emit_byte(jit_memory, (imm >> (i * 8)) & 0xFF);
		} else {
			// 64bit load imm
			emit_rex(jit_memory, 1, 0, dst);
			emit_byte(jit_memory, 0xB8 | reg86_base);
			for (int i = 0; i < 8; i++) emit_byte(jit_memory, (imm >> (i * 8)) & 0xFF);
		}
	} else {
		printf("TODO: implement this aswell\n");
		exit(1);
	}
}

void emit_ld(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint64_t imm) {
	
}

void emit_st(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint64_t imm) {
	
}

void emit_add(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
	emit_binary_rr(jit_memory, OPCODE_ADD_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_sub(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
	emit_binary_rr(jit_memory, OPCODE_SUB_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_mul(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
	emit_binary_rr(jit_memory, OPCODE_MUL_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_div(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
	emit_binary_rr(jit_memory, OPCODE_DIV_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_and(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
	emit_binary_rr(jit_memory, OPCODE_AND_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_or(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
	emit_binary_rr(jit_memory, OPCODE_OR_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_xor(uint8_t** jit_memory, uint32_t rd, uint32_t rs1, uint32_t rs2) {
	emit_binary_rr(jit_memory, OPCODE_XOR_REG_REG, 
			VMRegMap[rd], VMRegMap[rs1], VMRegMap[rs2]);
}

void emit_not(uint8_t** jit_memory, uint32_t rd, uint32_t rs1) {
	emit_unary_rr(jit_memory, OPCODE_NOT_REG, 
			VMRegMap[rd], VMRegMap[rs1]);
}

void emit_jit(uint8_t** jit_memory,
		uint32_t opcode,
		uint32_t rd,
		uint32_t rs1,
		uint32_t rs2,
		uint64_t imm) {
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
			printf("Instruction not implemented yet!\n");
			exit(1);
	}
}
