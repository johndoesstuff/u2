#include "instruction.h"

Instruction Instructions[] = {
    // DATA
    [U2_MOV] = {0b0011, "mov"},  // copy rs1 to rd
    [U2_LI] = {0b1001, "li"},    // load immediate to rd
    [U2_LD] = {0b1011, "ld"},    // load memory to rd
				 // TODO: how tf is memory going to work???
    [U2_ST] = {0b1110, "st"},    // store to memory from rs1

    // ARITHMETIC
    [U2_ADD] = {0b0111, "add"},  // add rs1 and rs2 and store in rd
    [U2_SUB] = {0b0111, "sub"},  // subtract rs1 and rs2 and store in rd
    [U2_MUL] = {0b0111, "mul"},  // multiply rs1 and rs2 and store in rd
    [U2_DIV] = {0b0111, "div"},  // divide rs1 and rs2 and store in rd

    // BITWISE
    [U2_AND] = {0b0111, "and"},  // and rs1 and rs2 and store in rd
    [U2_OR] = {0b0111, "or"},    // or rs1 and rs2 and store in rd
    [U2_XOR] = {0b0111, "xor"},  // xor rs1 and rs2 and store in rd
    [U2_NOT] = {0b0011, "not"},  // not rs1 and store in rd

    // BITWISE SHIFT
    [U2_SHL] = {0b1011, "shl"},  // shift rs1 left by imm and store in rd
    [U2_SHR] = {0b1011, "shr"},  // shift rs1 right by imm and store in rd

    // COMPARISON
    [U2_CMP] = {0b0110, "cmp"},  // compare registers (TODO: figure out wtf
				 // that means and how the hell im going to
				 // implement special registers)

    // CONTROL
    [U2_JMP] = {0b1000, "jmp"},  // jump to relative imm
    [U2_JE] = {0b1000, "je"},    // jump if equal
    [U2_JNE] = {0b1000, "jne"},  // jump if not equal
    [U2_JL] = {0b1000, "jl"},    // jump if less than
    [U2_JG] = {0b1000, "jg"},    // jump if greater than

    // TODO: is stack built into vm?
};

const int Instruction_Count = sizeof(Instructions) / sizeof(Instructions[0]);

char* instruction_from_id(int id) {
    if (id >= Instruction_Count)
        return "Unknown";
    else
        return Instructions[id].name;
}
