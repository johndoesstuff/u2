#include "instruction.h"

Instruction Instructions[] = {
	// DATA
	[U2_MOV] = { FORMAT_R, "mov" },    // copy rs1 to rd
	[U2_LI]  = { FORMAT_I, "li" },     // load immediate to rd
	[U2_LD]  = { FORMAT_M, "ld" },     // load memory to rd
	[U2_ST]  = { FORMAT_M, "st" },     // store to memory from rd

	// ARITHMETIC
	[U2_ADD] = { FORMAT_F, "add" },    // add rs1 and rs2 and store in rd
	[U2_SUB] = { FORMAT_F, "sub" },    // subtract rs1 and rs2 and store in rd
	[U2_MUL] = { FORMAT_F, "mul" },    // multiply rs1 and rs2 and store in rd
	[U2_DIV] = { FORMAT_F, "div" },    // divide rs1 and rs2 and store in rd

	// BITWISE
	[U2_AND] = { FORMAT_F, "and" },    // and rs1 and rs2 and store in rd
	[U2_OR]  = { FORMAT_F, "or" },     // or rs1 and rs2 and store in rd
	[U2_XOR] = { FORMAT_F, "xor" },    // xor rs1 and rs2 and store in rd
	[U2_NOT] = { FORMAT_R, "not" },    // not rs1 and store in rd

	// BITWISE SHIFT
	[U2_SHL] = { FORMAT_I, "shl" },    // shift rd left by imm
	[U2_SHR] = { FORMAT_I, "shr" },    // shift rd right by imm

	// COMPARISON
	[U2_CMP] = { FORMAT_R, "cmp" },    // compare registers (TODO: figure out wtf that means and how the hell im going to implement special registers)

	// CONTROL
	[U2_JMP] = { FORMAT_J, "jmp" },    // jump to relative imm
	[U2_JE]  = { FORMAT_J, "je" },     // jump if equal
	[U2_JNE] = { FORMAT_J, "jne" },    // jump if not equal
	[U2_JL]  = { FORMAT_J, "jl" },     // jump if less than
	[U2_JG]  = { FORMAT_J, "jg" },     // jump if greater than

	// TODO: is stack built into vm?
};

const int Instruction_Count = sizeof(Instructions)/sizeof(Instructions[0]);
