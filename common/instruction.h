/**
	U2 Instruction Set
*/

enum {
	FORMAT_F, // 3 register :		ins r0 r1 r2
	FORMAT_R, // register to register :	ins r0 r1
	FORMAT_I, // immediate to register :	ins r0 63
	FORMAT_J, // immediate opcode :		ins 63
	FORMAT_D, // register opcode : 		ins r0
	FORMAT_NONE, // opcode :		ins
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
