#include "cfg.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/debug.h"

extern int DEV_DEBUG;

/*
 * cfg.c
 *
 * CONTROL FLOW GRAPH GENERATOR for REGISTER ALLOCATION
 */

/*
 * STEP 1: CREATE ARRAY OF PARSED INSTRUCTIONS
 */

ParsedArray* init_parsed_array() {
    ParsedArray* parsed_array = malloc(sizeof(ParsedArray));
    parsed_array->capacity = 16;
    parsed_array->count = 0;
    parsed_array->instructions = malloc(sizeof(ParsedInstruction*) * parsed_array->capacity);
    return parsed_array;
}

void push_parsed_array(ParsedArray* parsed_array, ParsedInstruction* instruction) {
    if (parsed_array->count == parsed_array->capacity) {
        parsed_array->capacity *= 2;
        parsed_array->instructions =
            realloc(parsed_array->instructions, sizeof(ParsedInstruction*) * parsed_array->capacity);
    }
    // shallow copy! this will break if we add nested fields to ParsedInstruction!
    ParsedInstruction* copy = malloc(sizeof(ParsedInstruction));
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
        fprintf(stderr, "Internal Error: Those who know (skull emoji)\n");
        exit(EXIT_FAILURE);
    }
}

JumpTableEntry* jte_from_source(uint64_t source, JumpTable* jt) {
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
JumpTable* jumptable_from_parsed_array(ParsedArray* parsed_array) {
    // initialize
    JumpTable* jt = malloc(sizeof(JumpTable));
    jt->count = 0;
    jt->capacity = 16;
    jt->entries = malloc(sizeof(JumpTableEntry*) * jt->capacity);
    for (unsigned int i = 0; i < parsed_array->count; i++) {
        uint32_t op = parsed_array->instructions[i]->opcode;
        if (is_jump__(op)) {
            uint64_t imm = parsed_array->instructions[i]->imm;
            uint32_t imm_ext = parsed_array->instructions[i]->imm_ext;
            JumpTableEntry* jte = malloc(sizeof(JumpTableEntry));
            jte->source_id = i;
            jte->target_id = (int64_t)sign_ext_imm__(imm, imm_ext);
            jte->resolved_target_id = jte->source_id + jte->target_id;
            if (jt->count == jt->capacity) {
                jt->capacity *= 2;
                jt->entries = realloc(jt->entries, sizeof(JumpTableEntry*) * jt->capacity);
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

int in_leaders(LeaderSet* ls, uint64_t pc) {
    for (size_t i = 0; i < ls->count; i++) {
        if (ls->leaders[i] == pc)
            return 1;
    }
    return 0;
}

void add_leader(LeaderSet* ls, uint64_t pc) {
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
int cmp_uint64(const void* a, const void* b) {
    uint64_t x = *(const uint64_t*)a;
    uint64_t y = *(const uint64_t*)b;
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

LeaderSet* generate_leaders(ParsedArray* pa, JumpTable* jt) {
    LeaderSet* ls = malloc(sizeof(LeaderSet));
    ls->capacity = 16;
    ls->count = 0;
    ls->leaders = malloc(sizeof(uint64_t) * ls->capacity);

    // add first instruction to leaders
    add_leader(ls, 0);

    // add jump targets (absolute)
    for (size_t i = 0; i < jt->count; i++) {
        JumpTableEntry* jte = jt->entries[i];
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

void add_cfg(CFG* cfg, BasicBlock* bb) {
    cfg->nodes[cfg->count++] = bb;
    if (cfg->count == cfg->capacity) {
        cfg->capacity *= 2;
        cfg->nodes = realloc(cfg->nodes, sizeof(BasicBlock*) * cfg->capacity);
    }
}

void add_bb(BasicBlock* bb, ParsedInstruction* pi) {
    bb->instructions[bb->instructions_count++] = pi;
    if (bb->instructions_count == bb->instructions_capacity) {
        bb->instructions_capacity *= 2;
        bb->instructions = realloc(bb->instructions, sizeof(ParsedInstruction*) * bb->instructions_capacity);
    }
}

BasicBlock* get_bb_by_leader(CFG* cfg, uint64_t leader) {
    for (size_t i = 0; i < cfg->count; i++) {
        BasicBlock* bb = cfg->nodes[i];
        if (bb->leader == leader) {
            return bb;
        }
    }
    return NULL;
}

CFG* build_cfg(ParsedArray* pa, JumpTable* jt, LeaderSet* ls) {
    CFG* cfg = malloc(sizeof(CFG));
    cfg->count = 0;
    cfg->capacity = 16;
    cfg->nodes = malloc(sizeof(BasicBlock*) * cfg->capacity);
    // build a basic block spanning each leader
    for (size_t i = 0; i < ls->count; i++) {
        BasicBlock* bb = malloc(sizeof(BasicBlock));
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

        bb->instructions = malloc(sizeof(ParsedInstruction*) * bb->instructions_capacity);
        bb->incoming = malloc(sizeof(BasicBlock*) * bb->incoming_capacity);
        bb->outgoing = malloc(sizeof(BasicBlock*) * bb->outgoing_capacity);

        bb->live_in = 0;
        bb->live_out = 0;

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
            printf_DEBUG("Added instruction %lu to bb %ld\n", j, i);
            add_bb(bb, pa->instructions[j]);
        }
        add_cfg(cfg, bb);
    }

    // connect bbs in cfg
    for (size_t i = 0; i < cfg->count; i++) {
        BasicBlock* bb = cfg->nodes[i];

        // connectivity is based on last instruction, if not jump, connect to
        // leader of pc_end+1, if unconditional jump connect to leader of jump
        // location. if conditional jump connect to both pc_end+1 and jump
        // location :P
        ParsedInstruction* li = bb->instructions[bb->instructions_count - 1];
        uint64_t pc_end = bb->leader + bb->instructions_count - 1;  // pc of li

        if (!li) {
            printf_DEBUG("Odd, li NULL at bb %ld with pc_end of %lu\n", i, pc_end);
            continue;
        }

        int jumps = is_jump__(li->opcode);
        int fallthrough = is_jump_conditional__(li->opcode) || !jumps;

        if (jumps) {
            JumpTableEntry* jte = jte_from_source(pc_end, jt);  // jump table of li
            assert(jte != NULL);
            BasicBlock* next_bb = get_bb_by_leader(cfg, jte->resolved_target_id);  // inst jumped to
            if (next_bb) {
                bb->outgoing[bb->outgoing_count++] = next_bb;
                next_bb->incoming[next_bb->incoming_count++] = bb;
            }
        }

        if (fallthrough) {
            BasicBlock* next_bb = get_bb_by_leader(cfg, pc_end + 1);  // pc after li
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

// return a 16bit bitmask of which registers are expected
uint16_t live_in_from_bb(BasicBlock* bb) {
    ParsedInstruction** instructions = bb->instructions;
    uint16_t live_in = 0;
    uint16_t defined = 0;
    for (size_t i = 0; i < bb->instructions_count; i++) {
        ParsedInstruction* instruction = instructions[i];
        InstructionFormat f = instruction->obj.format;
        if (f & (1 << 2)) {  // expects rs2
            if (!(defined & (1 << instruction->rs2)))
                live_in |= 1 << instruction->rs2;
        }
        if (f & (1 << 1)) {  // expects rs1
            if (!(defined & (1 << instruction->rs1)))
                live_in |= 1 << instruction->rs1;
        }
        if (f & (1 << 0)) {  // defines rd
            defined |= 1 << instruction->rd;
        }
    }
    return live_in;
}

// return a 16bit bitmask of which registers are expected out
uint16_t live_out_from_bb(BasicBlock* bb) {
    uint16_t live_out = 0;
    for (size_t i = 0; i < bb->outgoing_count; i++) {
        live_out |= live_in_from_bb(bb->outgoing[i]);
    }
    return live_out;
}

// return a 16bit bitmask of which registers are defined
uint16_t defined_in_bb(BasicBlock* bb) {
    ParsedInstruction** instructions = bb->instructions;
    uint16_t defined = 0;
    for (size_t i = 0; i < bb->instructions_count; i++) {
        ParsedInstruction* instruction = instructions[i];
        InstructionFormat f = instruction->obj.format;
        if (f & (1 << 0)) {  // defines rd
            defined |= 1 << instruction->rd;
        }
    }
    return defined;
}

// flow liveness across cfg
void compute_liveness(CFG* cfg) {
    int changed = 1;
    do {
        changed = 0;

        for (size_t i = 0; i < cfg->count; i++) {
            BasicBlock* bb = cfg->nodes[i];

            uint16_t old_live_in = bb->live_in;
            uint16_t old_live_out = bb->live_out;

            uint16_t new_live_out = 0;
            for (size_t j = 0; j < bb->outgoing_count; j++)
                new_live_out |= bb->outgoing[j]->live_in;
            bb->live_out = new_live_out;

            bb->live_in = live_in_from_bb(bb) | (bb->live_out & ~defined_in_bb(bb));

            if (bb->live_in != old_live_in || bb->live_out != old_live_out)
                changed = 1;
        }
    } while (changed);
}

/**
 * STEP 6: LOCAL REGISTER MAPPING
 *
 * Ok ok awesome so we have a cfg that maps how our program flows, but we still
 * need registers allocated. So we will analyze each block and map virtual to
 * physical registers
 */

void init_regmap(RegMap* m) {
    for (int i = 0; i < MAX_VREGS; i++)
        m->vreg_to_preg[i] = -1;
    for (int i = 0; i < MAX_PREGS; i++)
        m->preg_to_vreg[i] = -1;
}

int alloc_preg(RegMap* m, int vreg) {
    for (int p = 0; p < MAX_PREGS; p++) {
        if (m->preg_to_vreg[p] == -1) {
            m->preg_to_vreg[p] = vreg;
            m->vreg_to_preg[vreg] = p;
            return p;
        }
    }

    // SPILL — simplest approach:
    fprintf(stderr, "Spilling vreg %d (no free preg)\n", vreg);
    return -1;
}

void free_vreg(RegMap* m, int vreg) {
    int p = m->vreg_to_preg[vreg];
    if (p >= 0) {
        m->preg_to_vreg[p] = -1;
        m->vreg_to_preg[vreg] = -1;
    }
}

int is_live(uint16_t mask, int vreg) {
    return (mask >> vreg) & 1;
}

uint16_t live_before_inst(BasicBlock* bb, size_t inst_i) {
    uint16_t live = bb->live_out;

    // walk backward from end to inst_i+1
    for (size_t j = bb->instructions_count - 1; j > inst_i; j--) {
        ParsedInstruction* pi = bb->instructions[j];
        InstructionFormat f = pi->obj.format;

        // kill defs
        if (f & 1)
            live &= ~(1u << pi->rd);

        // add uses
        if (f & 2)
            live |= 1u << pi->rs1;
        if (f & 4)
            live |= 1u << pi->rs2;
    }

    return live;
}

void debug_print_regmap(const char* label, RegMap* m) {
    printf("  %s:\n", label);
    for (int v = 0; v < MAX_VREGS; v++) {
        if (m->vreg_to_preg[v] != -1)
            printf_DEBUG("    v%-2d -> R%-2d\n", v, m->vreg_to_preg[v]);
    }
}

void allocate_block(BasicBlock* bb) {
    printf("\n===== ALLOCATE BLOCK leader=%ld =====\n", bb->leader);

    // allocate maps
    bb->map_out = malloc(sizeof(RegMap));
    bb->map_in = malloc(sizeof(RegMap));
    init_regmap(bb->map_out);
    init_regmap(bb->map_in);

    printf("STEP 1: initialize map_out based on live_out = 0x%04x\n", bb->live_out);

    uint16_t live = bb->live_out;
    for (int v = 0; v < MAX_VREGS; v++) {
        if (is_live(live, v)) {
            printf("  live_out: allocating v%d\n", v);
            alloc_preg(bb->map_out, v);
        }
    }

    printf("Initial OUT map:\n");
    debug_print_regmap("OUT", bb->map_out);

    // working copy
    RegMap cur = *bb->map_out;

    printf("STEP 2: walk backwards through instructions\n");

    for (int i = bb->instructions_count - 1; i >= 0; i--) {
        ParsedInstruction* pi = bb->instructions[i];
        InstructionFormat f = pi->obj.format;

        printf("\n  INSTR %d: opcode=%d rd=%d rs1=%d rs2=%d\n", i, pi->opcode, pi->rd, pi->rs1, pi->rs2);

        printf("  Current regmap before processing:\n");
        debug_print_regmap("CUR", &cur);

        // USES FIRST
        if (f & (1 << 1)) {
            if (cur.vreg_to_preg[pi->rs1] == -1) {
                printf("    USE rs1=v%d -> allocating\n", pi->rs1);
                alloc_preg(&cur, pi->rs1);
            }
        }
        if (f & (1 << 2)) {
            if (cur.vreg_to_preg[pi->rs2] == -1) {
                printf("    USE rs2=v%d -> allocating\n", pi->rs2);
                alloc_preg(&cur, pi->rs2);
            }
        }

        // DEST
        if (f & (1 << 0)) {
            if (cur.vreg_to_preg[pi->rd] != -1) {
                printf("    DEST kills old v%d -> freeing\n", pi->rd);
                free_vreg(&cur, pi->rd);
            }
            printf("    DEST alloc v%d\n", pi->rd);
            alloc_preg(&cur, pi->rd);
        }

        // kill vregs that die here
        uint16_t live_before = live_before_inst(bb, i);
        printf("  live_before_inst = 0x%04x\n", live_before);

        for (int v = 0; v < MAX_VREGS; v++) {
            if (!is_live(live_before, v) && cur.vreg_to_preg[v] != -1) {
                printf("    v%d dies here -> free preg R%d\n", v, cur.vreg_to_preg[v]);
                free_vreg(&cur, v);
            }
        }
    }

    printf("\nFinal map_in for block %ld:\n", bb->leader);
    debug_print_regmap("IN", &cur);

    // Write final map as map_in
    *bb->map_in = cur;

    printf("===== END BLOCK %ld =====\n", bb->leader);
}

void fix_edges(CFG* cfg) {
    printf("\n===== FIX EDGES =====\n");

    for (size_t i = 0; i < cfg->count; i++) {
        BasicBlock* bb = cfg->nodes[i];

        printf("BLOCK %ld map_out:\n", bb->leader);
        debug_print_regmap("OUT", bb->map_out);

        for (size_t j = 0; j < bb->outgoing_count; j++) {
            BasicBlock* succ = bb->outgoing[j];

            printf("  EDGE %ld -> %ld\n", bb->leader, succ->leader);
            printf("    Succ map_in:\n");
            debug_print_regmap("IN", succ->map_in);

            for (int v = 0; v < MAX_VREGS; v++) {
                int p_out = bb->map_out->vreg_to_preg[v];
                int p_in = succ->map_in->vreg_to_preg[v];

                if (p_out >= 0 && p_in >= 0 && p_out != p_in) {
                    printf("      CONFLICT v%d: OUT=R%d, IN=R%d => INSERT mov R%d -> R%d\n", v, p_out, p_in, p_out,
                           p_in);

                    // Here you'd insert an IR instruction
                }
            }
        }
    }

    printf("===== END FIX EDGES =====\n");
}
