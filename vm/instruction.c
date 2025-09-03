/**
	U2 Instruction Set
*/

enum {
	FORMAT_R, // register to register
	FORMAT_I, // immediate to register
	FORMAT_J, // immediate opcode
	FORMAT_D, // register opcode
	FORMAT_NONE, // opcode
} typedef InstructionFormat

struct {
	InstructionFormat format; 
	char* name;
} typedef Instruction;

Instruction Instructions[] = {
	{ FORMAT_R, "MOV" },
	{ FORMAT_D, "DREF" },
}
