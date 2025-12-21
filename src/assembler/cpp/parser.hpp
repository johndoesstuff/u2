#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <string_view>
#include <iostream>
#include <cstdint>
#include "../../common/instruction.hpp"
#include "../../common/types.hpp"

struct ParsedLabel {
	std::string_view identifier;
};

struct ParsedInstruction {
    OPCODE opcode;
    REGISTER rd;
    REGISTER rs1;
    REGISTER rs2;
    uint32_t imm_ext;
    IMMEDIATE imm;
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
	std::string_view consume_comment();
	std::string_view consume_word();

	REGISTER parse_register(std::string_view reg);
	IMMEDIATE parse_number(std::string_view num);

	std::istream& in_;
	std::string_view line_;
	std::string line_storage_;

	size_t lineno_ = 0;
	size_t colno_ = 0;
};

// debug operators
std::ostream& operator<<(std::ostream& os, const ParsedLabel& p);
std::ostream& operator<<(std::ostream& os, const ParsedInstruction& p);
std::ostream& operator<<(std::ostream& os, const ParsedLine& p);

#endif
