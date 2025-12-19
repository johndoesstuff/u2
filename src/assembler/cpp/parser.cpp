#include <string>
#include "../common/instruction.hpp"

class Parser {
public:
	Parser(std::istream& in): in_(in);
	
	size_t lineno() const { return lineno_; };
	size_t colno() const { return colno_; };

	ParsedLine parse_line() {
		std::getline(in_, line_storage_);
		line_ = line_storage_;
		lineno_++;
		colno_ = 0;
		consume_whitespace();
		std::string_view op = consume_opcode();
		if (op.empty()) {
			// could be label
			std::string_view label = conusme_identifier();
			consume_whitespace();

		}
	}

private:
	void consume_whitespace() {
		while (colno_ < line_.size() && std::isspace(line_[colno_])) {
			colno_++;
		}
	}

	std::string_view consume_identifier() {
		size_t start_col = colno_;
		if (colno_ < line_.size() && std::isalpha(line_[colno_])) {
			colno_++;
		} else {
			return {};
		}

		while (colno_ < line_.size() && std::isalnum || line_[colno_] == '_') {
			colno_++;
		}
		return line_.substr(start_col, colno_ - start_col);
	}

	std::string_view consume_opcode() {
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

	std::string_view consume_register() {
		size_t start_col = colno_;
		
	}
	
	std::istream& in_;
	std::string_view line_;
	std::string line_storage_;

	size_t lineno_;
	size_t colno_;
}
