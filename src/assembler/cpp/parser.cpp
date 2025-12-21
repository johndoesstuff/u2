#include <string>
#include <stdexcept>
#include <iostream>

#include "../../common/instruction.hpp"
#include "parser.hpp"

Parser::Parser(std::istream& in): in_(in), lineno_(0), colno_(0) {};

size_t Parser::lineno() const { return lineno_; };
size_t Parser::colno() const { return colno_; };

static void parse_error(const std::string& err, size_t lineno, size_t colno, const std::string& line) {
	std::string err_loc(colno, ' ');
	err_loc += '^';
	throw std::runtime_error(
			"Parse error on line " + 
			std::to_string(lineno) + ":" + 
			std::to_string(colno) + "\n" + 
			line + "\n" + 
			err_loc + "\n" + 
			err);
}

std::ostream& operator<<(std::ostream& os, const ParsedLabel& p) {
	return os << p.identifier;
}

std::ostream& operator<<(std::ostream& os, const ParsedInstruction& p) {
	os << "[ op: " << p.opcode;
	os << ", rd: " << p.rd;
	os << ", rs1: " << p.rs1;
	os << ", rs2: " << p.rs2;
	os << ", imm: " << p.imm;
	os << "]";
	return os;
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
			std::string err_string {"Unknown opcode: '"};
			err_string += label;
			err_string += "'";
			std::string line {line_};
			parse_error(err_string, lineno_, colno_, line);
		}
		consume_whitespace();
		consume_comment();
		p.type = LineType::label;
		p.label = ParsedLabel {label};
	} else {
		p.type = LineType::instruction;
		p.instruction.opcode = opcode_from_str(op);
		InstructionFormat format = instruction_from_opcode(p.instruction.opcode).format;
		consume_whitespace();

		if (has(format, InstructionFormat::Rd)) {
			std::string_view reg_str = consume_register();
			if (reg_str.empty()) {
				std::string err_string {"Expected register but found: '"};
				err_string += consume_word();
				err_string += "'";
				std::string line {line_};
				parse_error(err_string, lineno_, colno_, line);
			}
			p.instruction.rd = parse_register(reg_str);
		}
		consume_whitespace();

		if (has(format, InstructionFormat::Rs1)) {
			std::string_view reg_str = consume_register();
			if (reg_str.empty()) {
				std::string err_string {"Expected register but found: '"};
				err_string += consume_word();
				err_string += "'";
				std::string line {line_};
				parse_error(err_string, lineno_, colno_, line);
			}
			p.instruction.rs1 = parse_register(reg_str);
		}
		consume_whitespace();

		if (has(format, InstructionFormat::Rs2)) {
			std::string_view reg_str = consume_register();
			if (reg_str.empty()) {
				std::string err_string {"Expected register but found: '"};
				err_string += consume_word();
				err_string += "'";
				std::string line {line_};
				parse_error(err_string, lineno_, colno_, line);
			}
			p.instruction.rs2 = parse_register(reg_str);
		}
		consume_whitespace();

		if (has(format, InstructionFormat::Imm)) {
			std::string_view imm_str = consume_identifier();
			if (imm_str.empty()) {
				imm_str = consume_number();
				if (imm_str.empty()) {
					std::string err_string {"Expected immediate but found: '"};
					err_string += consume_word();
					err_string += "'";
					std::string line {line_};
					parse_error(err_string, lineno_, colno_, line);
				}
				IMMEDIATE p_num = parse_number(imm_str);
				p.instruction.imm = p_num;
			} else p.instruction.imm = IMMEDIATE(0); // unknown until labels are resolved
		}
		consume_whitespace();
		consume_comment();
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

IMMEDIATE Parser::parse_number(std::string_view num) {
    size_t col = 0;
    int base = 10;

    if (col < num.size() && num[col] == '0') {
        col++;
        if (col < num.size()) {
            char c = num[col];
            if (c == 'x' || c == 'X') {
                base = 16;
                col++;
            } else if (c == 'b' || c == 'B') {
                base = 2;
                col++;
            }
        }
    }

    IMMEDIATE value(0);
    while (col < num.size()) {
        char c = num[col];
        int digit = -1;

        if (std::isdigit(c))
            digit = c - '0';
        else if (base == 16 && c >= 'a' && c <= 'f')
            digit = 10 + (c - 'a');
        else if (base == 16 && c >= 'A' && c <= 'F')
            digit = 10 + (c - 'A');

        if (digit < 0 || digit >= base)
            break;

        value = value * base + digit;
        col++;
    }

    return value;
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

std::string_view Parser::consume_word() {
	size_t start_col = colno_;
	while (colno_ < line_.size() && !std::isspace(line_[colno_])) {
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

// -1 for invalid, 0-15 for registers r1-r16
REGISTER Parser::parse_register(std::string_view reg) {
	if (reg[0] != 'r') return REGISTER(-1);
	int reg_num {0};
	for (size_t i = 1; i < reg.size() && std::isdigit(reg[i]); i++) {
		reg_num *= 10;
		reg_num += reg[i] - '0';
	}
	if (reg_num < 1 || reg_num > 16) {
		return REGISTER(-1);
	}
	REGISTER regf (reg_num);
	return regf;
}

std::string_view Parser::consume_comment() {
	size_t start_col = colno_;
	if (consume_char(';').empty()) return {};
	colno_ = line_.size() - 1;
	return line_.substr(start_col, colno_ - start_col);
}
