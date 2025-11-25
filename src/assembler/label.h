/*
 * label.h
 *
 * Store relevant information about labels, assembler is based on a 2 pass
 * system: first pass determines label positioning and second pass emits
 * bytecode accounting for label positioning for relative addresses
 */

#include <stdlib.h>
#include <string.h>

typedef struct {
    char* name;
    uint32_t pc;
} Label;

typedef struct {
    Label* labels;
    int count;
    int capacity;
} LabelTable;

LabelTable* new_label_table() {
    LabelTable* t = malloc(sizeof(LabelTable));
    t->count = 0;
    t->capacity = 16;
    t->labels = malloc(sizeof(Label) * t->capacity);
    return t;
}

void add_label(LabelTable* t, const char* name, uint32_t pc) {
    if (t->count == t->capacity) {
        t->capacity *= 2;
        t->labels = realloc(t->labels, sizeof(Label) * t->capacity);
    }
    t->labels[t->count].name = strdup(name);
    t->labels[t->count].pc = pc;
    t->count++;
}

int find_label(LabelTable* t, const char* name) {
    for (int i = 0; i < t->count; i++) {
        if (strcmp(t->labels[i].name, name) == 0)
            return t->labels[i].pc;
    }
    return -1;
}
