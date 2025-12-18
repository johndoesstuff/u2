#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

struct AssemblerFlags {
	bool dev_debug = false;
};

void usage(std::iostream& s) {
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
				usage(std:cerr);
				exit(EXIT_FAILURE);
			}
		}
	}
}
