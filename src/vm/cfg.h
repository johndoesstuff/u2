#ifndef CFG_H
#define CFG_H

/*
 * cfg.h
 *
 * Define structures and functions for cfg.c to generate a control flow graph
 */

#include "../common/instruction.h"
#include <stddef.h>
#include <stdint.h>

// TODO: there is a lot of ambiguity here between size_t and uint64_t, despite
// being the same under the hood since jump addresses can be signed 64bit imms
// we should use more consistency in the future to denote the relation between
// jump values and the indexing of instructions. This is a problem for way off
// in the future though as we wont start to see any side effects until using
// values above INT_MAX.

typedef struct BasicBlock {
    ParsedInstruction** instructions;
    size_t instructions_count;
    size_t instructions_capacity;
    struct BasicBlock** incoming;
    size_t incoming_count;
    size_t incoming_capacity;
    struct BasicBlock** outgoing;
    size_t outgoing_count;
    size_t outgoing_capacity;
    uint64_t leader;
} BasicBlock;

typedef struct {
    ParsedInstruction** instructions;
    size_t count;
    size_t capacity;
} ParsedArray;

typedef struct {
    uint64_t target_id;
    uint64_t resolved_target_id;
    uint64_t source_id;
} JumpTableEntry;

typedef struct {
    JumpTableEntry** entries;  // store jump table as array for cfg generation
    size_t count;
    size_t capacity;
} JumpTable;

typedef struct {
    uint64_t* leaders;
    size_t count;
    size_t capacity;
} LeaderSet;

typedef struct {
    BasicBlock** nodes;
    size_t count;
    size_t capacity;
} CFG;

// parsed array methods
ParsedArray* init_parsed_array(void);
void push_parsed_array(ParsedArray* parsed_array, ParsedInstruction* instruction);

JumpTable* jumptable_from_parsed_array(ParsedArray* parsed_array);
LeaderSet* generate_leaders(ParsedArray* parsed_array, JumpTable* jump_table);
CFG* build_cfg(ParsedArray* pa, JumpTable* jt, LeaderSet* ls);

#endif
