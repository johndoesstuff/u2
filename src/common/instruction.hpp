/**
    U2 Instruction Set
*/

#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <cstdint>
#include <string_view>

enum class InstructionFormat : uint8_t {
    Rd    = 1 << 0,
    Rs1   = 1 << 1,
    Rs2   = 1 << 2,
    Imm   = 1 << 3,
};

constexpr InstructionFormat make_format(
    std::initializer_list<InstructionFormat> flags
) {
    uint8_t value = 0;
    for (auto f : flags)
        value |= static_cast<uint8_t>(f);
    return static_cast<InstructionFormat>(value);
}

enum class Opcode {
    MOV,
    LI,
    LD,
    ST,
    ADD,
    SUB,
    MUL,
    DIV,
    AND,
    OR,
    XOR,
    NOT,
    SHL,
    SHR,
    CMP,
    JMP,
    JE,
    JNE,
    JL,
    JG,
};

struct Instruction {
    InstructionFormat format;
	std::string_view name;
} typedef Instruction;

inline constexpr Instruction INSTRUCTION_SET[] = {
    /* ===== DATA ===== */
	/* MOV */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			}), "mov"},  // copy rs1 to rd
    /* LI */ {make_format({
			InstructionFormat::Imm,
			InstructionFormat::Rd,
			}), "li"},    // load immediate to rd
    /* LD */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Imm,
			}), "ld"},    // load memory to rd
                          // TODO: how tf is memory going to work???
    /* ST */ {make_format({
			InstructionFormat::Rs1,
			InstructionFormat::Rs2,
			InstructionFormat::Imm,
			}), "st"},    // store to memory from rs1
	/* ===== Arithmetic ===== */
    /* ADD */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Rs2,
			}), "add"},  // add rs1 and rs2 and store in rd
    /* SUB */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Rs2,
			}), "sub"},  // subtract rs1 and rs2 and store in rd
    /* MUL */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Rs2,
			}), "mul"},  // multiply rs1 and rs2 and store in rd
    /* DIV */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Rs2,
			}), "div"},  // divide rs1 and rs2 and store in rd
    /* ===== BITWISE ===== */
    /* AND */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Rs2,
			}), "and"},  // and rs1 and rs2 and store in rd
    /* OR */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Rs2,
			}), "or"},    // or rs1 and rs2 and store in rd
    /* XOR */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Rs2,
			}), "xor"},  // xor rs1 and rs2 and store in rd
    /* NOT */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			}), "not"},  // not rs1 and store in rd
    /* ===== BITWISE SHIFT ===== */
    /* SHL */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Imm,
			}), "shl"},  // shift rs1 left by imm and store in rd
    /* SHR */ {make_format({
			InstructionFormat::Rd,
			InstructionFormat::Rs1,
			InstructionFormat::Imm,
			}), "shr"},  // shift rs1 right by imm and store in rd
    /* ===== COMPARISON ===== */
    /* CMP */ {make_format({
			InstructionFormat::Rs1,
			InstructionFormat::Rs2,
			}), "cmp"},  // compare registers (TODO: figure out wtf
                         // that means and how the hell im going to
                         // implement special registers)
    /* ===== CONTROL ===== */
    /* JMP */ {make_format({
			InstructionFormat::Imm,
			}), "jmp"},  // jump to relative imm
    /* JE */ {make_format({
			InstructionFormat::Imm,
			}), "je"},    // jump if equal
    /* JNE */ {make_format({
			InstructionFormat::Imm,
			}), "jne"},  // jump if not equal
    /* JL */ {make_format({
			InstructionFormat::Imm,
			}), "jl"},    // jump if less than
    /* JG */ {make_format({
			InstructionFormat::Imm,
			}), "jg"},    // jump if greater than

    // TODO: is stack built into vm?
};

// allow indexing of instructions from enums
constexpr size_t to_index(Opcode op) {
    return static_cast<size_t>(op);
}

constexpr int INSTRUCTION_COUNT = sizeof(INSTRUCTION_SET) / sizeof(INSTRUCTION_SET[0]);

constexpr const Instruction& instruction_from_opcode(Opcode op) {
	return INSTRUCTION_SET[to_index(op)];
}

constexpr int opcode_from_str(std::string_view st) {
	for (size_t i = 0; i < INSTRUCTION_COUNT; i++) {
		if (INSTRUCTION_SET[i].name == st) {
			return i;
		}
	}
	return -1;
}

// very important that opcodes match instruction set
static_assert(
    sizeof(INSTRUCTION_SET) / sizeof(INSTRUCTION_SET[0]) ==
    static_cast<size_t>(Opcode::JG) + 1,
    "Opcode enum and instruction table are out of sync"
);

#endif
