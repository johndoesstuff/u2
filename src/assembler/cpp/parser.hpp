#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <string_view>
#include <istream>
#include "../../common/instruction.hpp"

class Parser {
public:
	Parser(std::istream& in);

	size_t lineno();
	size_t colno();

	ParsedLine parse_line();

private:
	void consume_whitespace();
	std::string_view consume_identifier();
	std::string_view consume_opcode();
	std::string_view consume_register();

	std::istream& in_;
	std::string_view line_;
	std::string line_storage_;

	size_t lineno_;
	size_t colno_;
}

#endif
