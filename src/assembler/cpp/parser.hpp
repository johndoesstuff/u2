#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <string_view>
#include <istream>
#include <cstdint>
#include "../../common/instruction.hpp"

struct ParsedLabel {
	std::string_view identifier;
};

struct ParsedInstruction {
    uint32_t opcode;
    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t imm_ext;
    uint64_t imm;
    Instruction obj;
};

enum class LineType {
	label,
	instruction,
	eof,
};

struct ParsedLine {
	LineType type;
	union {
		struct ParsedLabel label;
		struct ParsedInstruction instruction;
	};
	size_t lineno;
};

class Parser {
public:
	Parser(std::istream& in);

	size_t lineno() const;
	size_t colno() const;

	ParsedLine parse_line();

private:
	std::string_view consume_whitespace();
	std::string_view consume_char(char c);
	std::string_view consume_number();
	std::string_view consume_identifier();
	std::string_view consume_opcode();
	std::string_view consume_register();

	std::istream& in_;
	std::string_view line_;
	std::string line_storage_;

	size_t lineno_ = 0;
	size_t colno_ = 0;
};

#endif
