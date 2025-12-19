#include <string>
#include <stdexcept>
#include <iostream>

#include "../../common/instruction.hpp"
#include "parser.hpp"

Parser::Parser(std::istream& in): in_(in), lineno_(0), colno_(0) {};

size_t Parser::lineno() const { return lineno_; };
size_t Parser::colno() const { return colno_; };

static void parse_error(const std::string& err, size_t lineno, size_t colno) {
	throw std::runtime_error("Parse error on line " + std::to_string(lineno) + ":" + std::to_string(colno) + "\n" + err);
}

std::ostream& operator<<(std::ostream& os, const ParsedLabel& p) {
	return os << p.identifier;
}

std::ostream& operator<<(std::ostream& os, const ParsedInstruction& p) {
	return os << p.opcode;
}

std::ostream& operator<<(std::ostream& os, const ParsedLine& p) {
	os << "ParsedLine: { ";
	os << "type: ";
	if (p.type == LineType::eof) return os << "eof }";
	else if (p.type == LineType::label) os << "label: " << p.label;
	else if (p.type == LineType::instruction) os << "instruction: " << p.instruction;
	return os << ", lineno: " << p.lineno << " }";
}

ParsedLine Parser::parse_line() {
	ParsedLine p{};
	if (!std::getline(in_, line_storage_)) {
		p.type = LineType::eof;
		return p;
	}
	line_ = line_storage_;
	lineno_++;
	colno_ = 0;
	p.lineno = lineno_;
	consume_whitespace();
	std::string_view op = consume_opcode();
	if (op.empty()) {
		// could be label
		std::string_view label = consume_identifier();
		consume_whitespace();
		if (consume_char(':').empty()) {
			std::string err_string {"Unknown opcode"};
			err_string += label;
			parse_error(err_string, lineno_, colno_);
		}
		p.type = LineType::label;
		p.label = ParsedLabel {label};
	} else {
		p.type = LineType::instruction;
		p.instruction.opcode = opcode_from_str(op);
	}
	return p;
}

std::string_view Parser::consume_whitespace() {
	size_t start_col = colno_;
	while (colno_ < line_.size() && std::isspace(line_[colno_])) {
		colno_++;
	}
	return line_.substr(start_col, colno_ - start_col);
}

std::string_view Parser::consume_char(char ch) {
	size_t start_col = colno_;
	if (colno_ < line_.size() && line_[colno_] == ch) {
		colno_++;
	}
	return line_.substr(start_col, colno_ - start_col);
}

std::string_view Parser::consume_number() {
	size_t start_col = colno_;
	// 0x for hex, 0b for binary, 071 is valid
	if (!consume_char('0').empty()) {
		if (!consume_char('x').empty() || !consume_char('X').empty()) {
			while (colno_ < line_.size() && (
						std::isdigit(line_[colno_])
						|| (line_[colno_] <= 'F' && line_[colno_] >= 'A')
						|| (line_[colno_] <= 'f' && line_[colno_] >= 'a'))) {
				colno_++;
			}
			return line_.substr(start_col, colno_ - start_col);
		} else if (!consume_char('b').empty() || !consume_char('B').empty()) {
			while (colno_ < line_.size() && (line_[colno_] == '0' || line_[colno_] == '1')) {
				colno_++;
			}
			return line_.substr(start_col, colno_ - start_col);
		}
	}
	while (colno_ < line_.size() && std::isdigit(line_[colno_])) {
		colno_++;
	}
	return line_.substr(start_col, colno_ - start_col);
}

std::string_view Parser::consume_identifier() {
	size_t start_col = colno_;
	if (colno_ < line_.size() && std::isalpha(line_[colno_])) {
		colno_++;
	} else {
		return {};
	}

	while (colno_ < line_.size() && (std::isalnum(line_[colno_]) || line_[colno_] == '_')) {
		colno_++;
	}
	return line_.substr(start_col, colno_ - start_col);
}

std::string_view Parser::consume_opcode() {
	size_t start_col = colno_;
	std::string_view identifier = consume_identifier();
	for (size_t i = 0; i < INSTRUCTION_COUNT; i++) {
		if (INSTRUCTION_SET[i].name == identifier) {
			return identifier;
		}
	}
	colno_ = start_col;
	return {};
}

std::string_view Parser::consume_register() {
	size_t start_col = colno_;
	if (consume_char('r').empty()) return {};
	size_t reg_num {0};
	while (colno_ < line_.size() && std::isdigit(line_[colno_])) {
		reg_num *= 10;
		reg_num += line_[colno_] - '0';
		colno_++;
	}
	if (reg_num < 1 || reg_num > 16) {
		colno_ = start_col;
		return {};
	}
	return line_.substr(start_col, colno_ - start_col);
}
