/**
	U2 Instruction Set
*/

enum {
	FORMAT_F,    // 3 register :            ins r0 r1 r2
	FORMAT_R,    // register to register :  ins r0 r1
	FORMAT_I,    // immediate to register : ins r0 im
	FORMAT_J,    // immediate opcode :      ins im
	FORMAT_D,    // register opcode :       ins r0
	FORMAT_NONE, // opcode :                ins
} typedef InstructionFormat;

struct {
	InstructionFormat format; 
	char* name;
} typedef Instruction;

Instruction Instructions[] = {
	// DATA
	{ FORMAT_R, "mov" },    // copy rs1 to rd
	{ FORMAT_I, "li" },     // load immediate to rd
	{ FORMAT_I, "ld" },     // load memory to rd
	{ FORMAT_I, "st" },     // store to memory from rd

	// ARITHMETIC
	{ FORMAT_F, "add" },    // add rs1 and rs2 and store in rd
	{ FORMAT_F, "sub" },    // subtract rs1 and rs2 and store in rd
	{ FORMAT_F, "mul" },    // multiply rs1 and rs2 and store in rd
	{ FORMAT_F, "div" },    // divide rs1 and rs2 and store in rd

	// BITWISE
	{ FORMAT_F, "and" },    // and rs1 and rs2 and store in rd
	{ FORMAT_F, "or" },     // or rs1 and rs2 and store in rd
	{ FORMAT_F, "xor" },    // xor rs1 and rs2 and store in rd
	{ FORMAT_R, "or" },     // not rs1 and store in rd

	// BITWISE SHIFT
	{ FORMAT_I, "shl" },    // shift rd left by imm
	{ FORMAT_I, "shr" },    // shift rd right by imm

	// COMPARISON
	{ FORMAT_R, "cmp" },    // compare registers (TODO: figure out wtf that means and how the hell im going to implement special registers)

	// CONTROL
	{ FORMAT_J, "jmp" },    // jump to imm
	{ FORMAT_J, "je" },    // jump if equal
	{ FORMAT_J, "jne" },    // jump if not equal
	{ FORMAT_J, "jl" },    // jump if less than
	{ FORMAT_J, "jg" },    // jump if greater than

	// TODO: is stack build into vm?
};

int Instruction_Count = sizeof(Instructions)/sizeof(Instructions[0]);
