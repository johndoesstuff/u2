#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "parser.hpp"

struct AssemblerFlags {
	bool dev_debug = false;
};

void usage(std::ostream& s) {
	s << "Usage: u2asm [flags] assembly.u2a bytecode.u2b" << std::endl;
}

int main(int argc, char* argv[]) {
	std::string asm_path, bc_path;
	AssemblerFlags assembler_flags;

	for (int i = 1; i < argc; i++) {
		// flags
		std::string arg(argv[i]);
		if (!arg.empty() && arg[0] == '-') {
			if (arg == "--dev") {
				assembler_flags.dev_debug = true;
			} else {
				std::cerr << "Unknown flag: " << arg << std::endl;
				usage(std::cerr);
				exit(EXIT_FAILURE);
			}
		} else {
			if (asm_path.empty()) {
				asm_path = arg;
			} else if (bc_path.empty()) {
				bc_path = arg;
			} else {
				std::cerr << "Too many arguments." << std::endl;
				usage(std::cerr);
				exit(EXIT_FAILURE);
			}
		}
	}

	if (asm_path.empty()) {
		std::cerr << "Missing assembly file." << std::endl;
		usage(std::cerr);
		exit(EXIT_FAILURE);
	} else if (bc_path.empty()) {
		std::cerr << "Missing output file." << std::endl;
		usage(std::cerr);
		exit(EXIT_FAILURE);
	}

	std::ifstream asm_file(asm_path);
	if (asm_file.fail()) {
		std::cerr << "Could not find file " << asm_path << std::endl;
		usage(std::cerr);
		exit(EXIT_FAILURE);
	}
	std::ofstream bc_file(bc_path);
	if (bc_file.fail()) {
		std::cerr << "Could not write to file " << bc_path << std::endl;
		usage(std::cerr);
		exit(EXIT_FAILURE);
	}

	std::string line;
	while (std::getline(asm_file, line)) {
		std::cout << line << std::endl;
	}
}
