#ifndef CFG_H
#define CFG_H

#include "../common/instruction.h"
#include <stdint.h>
#include <stddef.h>

typedef struct {
    ParsedInstruction** instructions; // atomic instruction unit
    size_t count;               // # instructions
	size_t capacity;
    size_t connection_count;    // 0, 1, 2
    struct BasicBlock** connections;
} BasicBlock;

typedef struct {
    ParsedInstruction** instructions;
    size_t count;
    size_t capacity;
} ParsedArray;

typedef struct {
    uint32_t target_id;
    uint32_t source_id;
} JumpTableEntry;

typedef struct {
    JumpTableEntry** entries; // store jump table as array for cfg generation
    size_t count;
    size_t capacity;
} JumpTable;

// parsed array methods
ParsedArray* init_parsed_array(void);
void push_parsed_array(ParsedArray* parsed_array, ParsedInstruction* instruction);
JumpTable* jumptable_from_parsed_array(ParsedArray* parsed_array);

#endif
