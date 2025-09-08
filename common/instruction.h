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
	{ FORMAT_R, "mov" },    // copy rs1 to rd
	{ FORMAT_I, "li" },     // load immediate to rd
	{ FORMAT_R, "ld" },     // load memory to rd
	{ FORMAT_R, "st" },     // store to memory from rd
};

int Instruction_Count = sizeof(Instructions)/sizeof(Instructions[0]);
