#ifndef CFG_H
#define CFG_H

#include "../common/instruction.h"

typedef struct {
	ParsedInstruction** instructions; // atomic instruction unit
	unsigned int count;               // # instructions
	unsigned int capacity;
	unsigned int connection_count;    // 0, 1, 2
	struct BasicBlock** connections;
} BasicBlock;

typedef struct {
	ParsedInstruction** instructions;
	unsigned int count;
	unsigned int capacity;
} ParsedArray;

// parsed array methods
ParsedArray* init_parsed_array(void);
void push_parsed_array(ParsedArray* parsed_array, ParsedInstruction* instruction);

#endif
