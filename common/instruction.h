/**
	U2 Instruction Set
*/

enum {
	FORMAT_R, // register to register
	FORMAT_I, // immediate to register
	FORMAT_J, // immediate opcode
	FORMAT_D, // register opcode
	FORMAT_NONE, // opcode
} typedef InstructionFormat;

struct {
	InstructionFormat format; 
	char* name;
} typedef Instruction;

Instruction Instructions[] = {
	{ FORMAT_R, "mov" },
	{ FORMAT_D, "dref" },
};

int Instruction_Count = sizeof(Instructions)/sizeof(Instructions[0]);
