#include "cfg.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * cfg.c
 *
 * CONTROL FLOW GRAPH GENERATOR for REGISTER ALLOCATION
 */

/*
 * STEP 1: CREATE ARRAY OF PARSED INSTRUCTIONS
 */

ParsedArray *init_parsed_array() {
    ParsedArray *parsed_array = malloc(sizeof(ParsedArray));
    parsed_array->capacity = 16;
    parsed_array->count = 0;
    parsed_array->instructions = malloc(sizeof(ParsedInstruction *) * parsed_array->capacity);
    return parsed_array;
}

void push_parsed_array(ParsedArray *parsed_array, ParsedInstruction *instruction) {
    if (parsed_array->count == parsed_array->capacity) {
        parsed_array->capacity *= 2;
        parsed_array->instructions =
            realloc(parsed_array->instructions, sizeof(ParsedInstruction *) * parsed_array->capacity);
    }
    // shallow copy! this will break if we add nested fields to ParsedInstruction!
    ParsedInstruction *copy = malloc(sizeof(ParsedInstruction));
    *copy = *instruction;
    parsed_array->instructions[parsed_array->count++] = copy;
}

/*
 * STEP 2: BREAK PARSED INSTRUCTIONS INTO BASIC BLOCKS

 * To do this we need to figure out first what basic block is, for this context
 * I will assume a basic block is an atomic set of instructions (no inner
 * jumpable labels) that contains a MAXIMUM of one jump instruction as the last
 * statement. To break our instruction array into a set of basic blocks we must
 * first figure out all instructions that are jumpable by creating a jump
 * table. Because forward jumping is allowed this must be done in a 2 pass
 * system.
 */

int is_jump__(uint32_t opcode) {
    return opcode == U2_JMP || opcode == U2_JE || opcode == U2_JNE || opcode == U2_JL || opcode == U2_JG;
}

int is_jump_conditional__(uint32_t opcode) {
    return opcode == U2_JE || opcode == U2_JNE || opcode == U2_JL || opcode == U2_JG;
}

// signs immediates based on extension for relative jumping (which i have
// decided i am implementing as of 5 minutes ago)
int64_t sign_ext_imm__(uint64_t imm, uint32_t imm_ext) {
    switch (imm_ext) {
    case 0:  // imm is default of uint14
        return (int16_t)(((imm & 0x3FFF) ^ 0x2000) - 0x2000);
    case 1:  // imm is uint32
        return (int32_t)imm;
    case 2:  // imm is uint64
        return imm;
    default:
        printf("Internal Error: Those who know (skull emoji)\n");
        exit(1);
    }
}

JumpTableEntry *jte_from_source(uint64_t source, JumpTable *jt) {
    for (size_t i = 0; i < jt->count; i++) {
        if (jt->entries[i]->source_id == source) {
            return jt->entries[i];
        }
    }
    return NULL;
}

// remember, the only goal of this function is just to generate a jump table
// from the parsed array. this just means we have to match every jump and find
// where it lands
JumpTable *jumptable_from_parsed_array(ParsedArray *parsed_array) {
    // initialize
    JumpTable *jt = malloc(sizeof(JumpTable));
    jt->count = 0;
    jt->capacity = 16;
    jt->entries = malloc(sizeof(JumpTableEntry *) * jt->capacity);
    for (unsigned int i = 0; i < parsed_array->count; i++) {
        uint32_t op = parsed_array->instructions[i]->opcode;
        if (is_jump__(op)) {
            uint64_t imm = parsed_array->instructions[i]->imm;
            uint32_t imm_ext = parsed_array->instructions[i]->imm_ext;
            JumpTableEntry *jte = malloc(sizeof(JumpTableEntry));
            jte->source_id = i;
            jte->target_id = (int64_t)sign_ext_imm__(imm, imm_ext);
            jte->resolved_target_id = jte->source_id + jte->target_id;
            if (jt->count == jt->capacity) {
                jt->capacity *= 2;
                jt->entries = realloc(jt->entries, sizeof(JumpTableEntry *) * jt->capacity);
            }
            jt->entries[jt->count++] = jte;
        }
    }

    return jt;
}

/*
 * STEP 3: GENERATE LEADERS FOR BASIC BLOCKS
 *
 * Ok so we have our jump table, now we need to actually break it into a cfg.
 * To do this we first need to figure out where each block begins and ends
 * sequentially. To do this we get the set of leaders then sort and deduplicate
 * them. A leader is either the first instruction in the program, a jump
 * target, or an instruction after a jump. We use these to start the basic
 * blocks and consume instructions until either:
 *
 * A: Our next instruction is a leader (basic blocks can't have internal jumping!)
 *
 * B: We run into a jump instruction
 *
 * We can generate the full cfg based on which of these conditions we have hit
 * and also which type of jump instruction we have hit!
 */

int in_leaders(LeaderSet *ls, uint64_t pc) {
    for (size_t i = 0; i < ls->count; i++) {
        if (ls->leaders[i] == pc)
            return 1;
    }
    return 0;
}

void add_leader(LeaderSet *ls, uint64_t pc) {
    if (in_leaders(ls, pc))
        return;  // O(n) but i just dont care, ok actually
                 // i kinda do a little.. TODO: fix this
    ls->leaders[ls->count++] = pc;
    if (ls->count == ls->capacity) {
        ls->capacity *= 2;
        ls->leaders = realloc(ls->leaders, sizeof(uint64_t) * ls->capacity);
    }
}

// needed for qsort
int cmp_uint64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

LeaderSet *generate_leaders(ParsedArray *pa, JumpTable *jt) {
    LeaderSet *ls = malloc(sizeof(LeaderSet));
    ls->capacity = 16;
    ls->count = 0;
    ls->leaders = malloc(sizeof(uint64_t) * ls->capacity);

    // add first instruction to leaders
    add_leader(ls, 0);

    // add jump targets (absolute)
    for (size_t i = 0; i < jt->count; i++) {
        JumpTableEntry *jte = jt->entries[i];
        add_leader(ls, jte->resolved_target_id);
        if (jte->source_id + 1 < pa->count)
            add_leader(ls, jte->source_id + 1);  // add instruction after jump if
                                                 // not at last line
    }

    // dont forget to sort!
    qsort(ls->leaders, ls->count, sizeof(uint64_t), cmp_uint64);

    return ls;
}

/*
 * STEP 4: BASIC BLOCKS
 *
 * Awesome so we have the leaders, each leader builds a basic block by spanning
 * the instructions until the index of the next leader is reached. Each basic
 * block also needs to connect to the other basic blocks it's capable of
 * reaching either by jump table or by falling through. For register allocation
 * specifically it's also useful to keep track of incoming connections (we will
 * need to analyze the flow of registers through each block later to solve an
 * elaborate graph coloring problem! oh boy!)
 */

void add_cfg(CFG *cfg, BasicBlock *bb) {
    cfg->nodes[cfg->count++] = bb;
    if (cfg->count == cfg->capacity) {
        cfg->capacity *= 2;
        cfg->nodes = realloc(cfg->nodes, sizeof(BasicBlock *) * cfg->capacity);
    }
}

void add_bb(BasicBlock *bb, ParsedInstruction *pi) {
    bb->instructions[bb->instructions_count++] = pi;
    if (bb->instructions_count == bb->instructions_capacity) {
        bb->instructions_capacity *= 2;
        bb->instructions = realloc(bb->instructions, sizeof(ParsedInstruction *) * bb->instructions_capacity);
    }
}

BasicBlock *get_bb_by_leader(CFG *cfg, uint64_t leader) {
    for (size_t i = 0; i < cfg->count; i++) {
        BasicBlock *bb = cfg->nodes[i];
        if (bb->leader == leader) {
            return bb;
        }
    }
    return NULL;
}

CFG *build_cfg(ParsedArray *pa, JumpTable *jt, LeaderSet *ls) {
    CFG *cfg = malloc(sizeof(CFG));
    cfg->count = 0;
    cfg->capacity = 16;
    cfg->nodes = malloc(sizeof(BasicBlock *) * cfg->capacity);
    // build a basic block spanning each leader
    for (size_t i = 0; i < ls->count; i++) {
        BasicBlock *bb = malloc(sizeof(BasicBlock));
        bb->instructions_count = 0;
        bb->incoming_count = 0;
        bb->outgoing_count = 0;

        bb->instructions_capacity = 16;
        bb->incoming_capacity = 2;  // likely no situation will ever arise where
                                    // there are more than 2 incoming or
                                    // outgoing connections, as that would imply
                                    // a conditional jump with 2 targets, but oh
                                    // well..
        bb->outgoing_capacity = 2;

        bb->instructions = malloc(sizeof(ParsedInstruction *) * bb->instructions_capacity);
        bb->incoming = malloc(sizeof(BasicBlock *) * bb->incoming_capacity);
        bb->outgoing = malloc(sizeof(BasicBlock *) * bb->outgoing_capacity);

        uint64_t pc_start = ls->leaders[i];
        uint64_t pc_end;  // inclusive
        if (ls->count != i + 1)
            pc_end = ls->leaders[i + 1] - 1;  // if next leader isnt defined set end
                                              // of basic block to last instruction
        else
            pc_end = pa->count - 1;

        bb->leader = pc_start;  // keeping track of bb leader is important for linking bbs

        // add instructions from pc_start:pc_end to bb
        for (uint64_t j = pc_start; j <= pc_end; j++) {
            printf("Added instruction %lu (%p) to bb %ld\n", j, pa->instructions[j], i);
            add_bb(bb, pa->instructions[j]);
        }
        add_cfg(cfg, bb);
    }

    // connect bbs in cfg
    for (size_t i = 0; i < cfg->count; i++) {
        BasicBlock *bb = cfg->nodes[i];

        // connectivity is based on last instruction, if not jump, connect to
        // leader of pc_end+1, if unconditional jump connect to leader of jump
        // location. if conditional jump connect to both pc_end+1 and jump
        // location :P
        ParsedInstruction *li = bb->instructions[bb->instructions_count - 1];
        uint64_t pc_end = bb->leader + bb->instructions_count - 1;  // pc of li

        if (!li) {
            printf("Odd, li NULL at bb %ld with pc_end of %lu\n", i, pc_end);
            continue;
        }

        printf("%p\n", (void *)li);

        int jumps = is_jump__(li->opcode);
        int fallthrough = is_jump_conditional__(li->opcode) || !jumps;

        if (jumps) {
            JumpTableEntry *jte = jte_from_source(pc_end, jt);  // jump table of li
            assert(jte != NULL);
            BasicBlock *next_bb = get_bb_by_leader(cfg, jte->resolved_target_id);  // inst jumped to
            if (next_bb) {
                bb->outgoing[bb->outgoing_count++] = next_bb;
                next_bb->incoming[next_bb->incoming_count++] = bb;
            }
        }

        if (fallthrough) {
            BasicBlock *next_bb = get_bb_by_leader(cfg, pc_end + 1);  // pc after li
            if (next_bb) {
                bb->outgoing[bb->outgoing_count++] = next_bb;
                next_bb->incoming[next_bb->incoming_count++] = bb;
            }
        }
    }
    return cfg;
}

/*
 * STEP 5:
 *
 * Now that we have a cfg to use it for register allocation we need to check
 * which registers are expected and produced by each basic block
 */

uint8_t *live_in_from_bb(BasicBlock *bb) {
    ParsedInstruction **instructions = bb->instructions;
    for (size_t i = 0; i < bb->instructions_count; i++) {
    }
}
