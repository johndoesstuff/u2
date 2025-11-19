#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>   // mmap
#include <inttypes.h>   // PRIX64
#include "../common/config.h"
#include "../common/instruction.h"

#include "x86jit.h"
#include "cfg.h"

/**
    U2 VIRTUAL MACHINE

    This is the main file for the u2 virtual machine
    u2 bytecode -> x86 execution

    Usage = u2vm bytecode.u2b
*/

typedef struct {
    uint8_t** jit_memory;   // pointer to the advance pointer, main way of
                            // interfacing with jit memory
    uint8_t* jit_base;      // pointer to the beginning of jit memory
    uint8_t* jit_advance;   // pointer to the current position in jit_memory
} Context;

typedef struct {
    uint32_t reg;
    uint32_t start;
    uint32_t end;
} RegisterLifetime;

// global for cfg pass
ParsedArray* parsed_arr;

uint32_t next_instruction(FILE* f, uint32_t* inst) {
    size_t n = fread(inst, sizeof(uint32_t), 1, f);
    if (n != 1) {
        if (feof(f)) {
            return 0;
        } else {
            printf("Error reading instruction\n");
            exit(1);
        }
    }
    return 1;
}

uint32_t get_opcode(uint32_t inst) {
    return inst >> (32 - OPCODE_BITS);
}

uint32_t get_rd(uint32_t inst) {
    return (inst >> (32 - OPCODE_BITS - REG_BITS)) & ((1u << REG_BITS) - 1);
}

uint32_t get_rs1(uint32_t inst) {
    return (inst >> (32 - OPCODE_BITS - 2*REG_BITS)) & ((1u << REG_BITS) - 1);
}

uint32_t get_rs2(uint32_t inst) {
    return (inst >> (32 - OPCODE_BITS - 3*REG_BITS)) & ((1u << REG_BITS) - 1);
}

int64_t get_imm(uint32_t inst) {
    uint32_t mask = (1u << IMM_BITS) - 1;
    int imm = inst & mask;
    // sign extend
    if (imm & (1u << (IMM_BITS - 1))) {
        imm |= ~mask;
    }
    return imm;
}

void do_pass(void (*pass_eval)(ParsedInstruction*, Context*), Context* context, FILE* fptr) {
    rewind(fptr); // reset i/o if not already
    uint32_t instruction;
    while (next_instruction(fptr, &instruction)) {
        ParsedInstruction* parsed = malloc(sizeof(ParsedInstruction));
        uint32_t opcode = get_opcode(instruction);
        uint32_t rd = get_rd(instruction);
        uint32_t rs1 = get_rs1(instruction);
        uint32_t rs2 = get_rs2(instruction);
        int64_t immediate = get_imm(instruction);
        Instruction instructionObj = Instructions[opcode];

        parsed->opcode = opcode;
        parsed->rd = rd;
        parsed->rs1 = rs1;
        parsed->rs2 = rs2;
        parsed->imm = immediate;
        parsed->imm_ext = 0;
        parsed->obj = instructionObj;

        // check for long immediates
        if (rs2 && instructionObj.format != FORMAT_F) { // value in rs2 when
                                                        // one shouldn't be
                                                        // expected
            switch (instructionObj.format) { // check imm extension is supported
                case FORMAT_J:
                case FORMAT_I:
                case FORMAT_M:
                    break;
                default:
                    printf("Immediate extension is not supported for"
                           " instructions of type %s", instructionObj.name);
                    exit(1);
            }

            // if rs2 contains 1 or 2 load next rs2 bytes into imm
            if (rs2 == 1 || rs2 == 2) {
                parsed->imm_ext = 1;
                uint32_t imm_ext;
                int captured = next_instruction(fptr, &imm_ext);
                if (!captured) {
                    printf("Expected immediate extension but instead recieved"
                           " EOF? Check rs2 value for last inst.\n");
                }
                immediate = imm_ext;
                if (rs2 == 2) {
                    parsed->imm_ext = 2;
                    captured = next_instruction(fptr, &imm_ext);
                    if (!captured) {
                        printf("Expected immediate extension but instead"
                               " recieved EOF? Check rs2 value for second to"
                               " last inst.\n");
                    }
                    immediate |= (int64_t)imm_ext << 32;
                }
            } else {
                printf("Invalid rs2 value\n");
            }
        }
        parsed->imm = immediate;
        pass_eval(parsed, context);
        free(parsed);
    }
}

void _DEBUG_print_parsed_instruction(ParsedInstruction* parsed) {
    printf("ParsedInstruction {\n");
    printf("\topcode: %u\n", parsed->opcode);
    printf("\trd: %u\n", parsed->rd);
    printf("\trs1: %u\n", parsed->rs1);
    printf("\trs2: %u\n", parsed->rs2);
    printf("\timm_ext: %u\n", parsed->imm_ext);
    printf("\timm: %ld (%lX)\n", parsed->imm, parsed->imm);
    printf("}\n");
}

void cfg_pass(ParsedInstruction* parsed, Context* context) {
    push_parsed_array(parsed_arr, parsed);
    //_DEBUG_print_parsed_instruction(parsed);
    (void)context;
}

void jit_pass(ParsedInstruction* parsed, Context* context) {
    // debug
    /*switch (parsed->obj.format) {
        case FORMAT_F:
            printf(" r%d r%d r%d", parsed->rd + 1, parsed->rs1 + 1, parsed->rs2 + 1);
            break;
        case FORMAT_R:
            printf(" r%d r%d", parsed->rd + 1, parsed->rs1 + 1);
            break;
        case FORMAT_I:
            printf(" r%d %" PRIX64, parsed->rd + 1, parsed->imm);
            break;
        case FORMAT_J:
            printf(" %" PRIX64, parsed->imm);
            break;
        case FORMAT_D:
            printf(" r%d", parsed->rd + 1);
            break;
        case FORMAT_NONE:
            break;
        default:
            printf(" [unknown]");
            exit(1);
            break;
    }
    printf("\n");*/

    emit_jit(context->jit_memory, parsed->opcode, parsed->rd, parsed->rs1, parsed->rs2, parsed->imm);
}

void free_context(Context* context) {
    free(context);
}

int main(int argc, char** argv) {
    // correct usage check
    if (argc < 2) {
        printf("Usage: u2vm bytecode.u2b\n");
        exit(1);
    }

    // try to open file
    char* bytecodePath = argv[1];
    FILE* bytecodeFile = fopen(bytecodePath, "rb");
    if (bytecodeFile == NULL) {
        printf("Could not find file of path %s\n", bytecodePath);
        exit(1);
    }

    // prepare memory for jit execution
    uint8_t *jit_base = mmap(NULL,    // address
            4096,             // size
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,               // fd
            0);               // offset
    if (jit_base == MAP_FAILED) {
        printf("Could not allocate memory for jit compilation!\n");
        exit(1);
    }
    uint8_t* jit_advance = jit_base;
    uint8_t** jit_memory = &jit_advance;
    init_jit(jit_memory);
    Context* context = malloc(sizeof(Context));
    context->jit_memory = jit_memory;
    context->jit_base = jit_base;
    context->jit_advance = jit_advance;

    // init global for cfg pass
    parsed_arr = init_parsed_array();
    do_pass(cfg_pass, context, bytecodeFile);
    JumpTable* jt = jumptable_from_parsed_array(parsed_arr);
    (void)jt; // avoid -Wextra

    do_pass(jit_pass, context, bytecodeFile);

    // return from jit
    free_jit(jit_memory);
    emit_x86ret_reg(jit_memory, 1);

    // dump machine code because god knows im not getting this right my first
    // try or my second or third or fourth
    size_t emitted_size = *jit_memory - jit_base;
    printf("===== x86 dump =====\n");
    for (size_t i = 0; i < emitted_size; i++) {
        printf("%02X ", (unsigned char)jit_base[i]);
    }
    printf("\n\n");

    // try to execute jit memory
    uint64_t (*func)() = (uint64_t (*)())jit_base;
    uint64_t result = func();

    printf("%" PRIX64 "\n", result);

    fclose(bytecodeFile);
    free_context(context);
    return 0;
}
