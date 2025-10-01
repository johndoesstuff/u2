#include "cfg.h"
#include <stdlib.h>

/*
	STEP 1: CREATE ARRAY OF PARSED INSTRUCTIONS
*/

ParsedArray* init_parsed_array() {
	ParsedArray* parsed_array = malloc(sizeof(ParsedArray));
	parsed_array->capacity = 16;
	parsed_array->count = 0;
	parsed_array->instructions = malloc(sizeof(ParsedInstruction) * parsed_array->capacity);
	return parsed_array;
}

void push_parsed_array(ParsedArray* parsed_array, ParsedInstruction* instruction) {
	if (parsed_array->count == parsed_array->capacity) {
		parsed_array->capacity *= 2;
		parsed_array->instructions = realloc(parsed_array->instructions, sizeof(ParsedInstruction) * parsed_array->capacity);
	}
	parsed_array->instructions[parsed_array->count++] = instruction;
}

/*
	STEP 2: BREAK PARSED INSTRUCTIONS INTO BASIC BLOCKS
*/
