void emit_jit(char** jit_memory,
		unsigned int opcode,
		unsigned int rd,
		unsigned int rs1,
		unsigned int rs2,
		unsigned int imm) {
	switch (opcode) {
		case 0:
			emit_mov(jit_memory, rd, rs1);
			break;
	}
}
