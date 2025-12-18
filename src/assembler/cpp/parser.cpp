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
	}

	std::string_view consume_opcode() {
		
	}
	
	std::istream& in_
	std::string_view line_;
	std::string line_storage_

	size_t lineno_;
	size_t colno_;
}
