#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>

void reset_cpu(CPU *cpu) {
    for (int i = 0; i < cpu->mem_size; i++)
        cpu->memory[i] = 0;

    for (int i = 0; i < 16; i++)
        cpu->reg[i] = 0;

    cpu->pc = 0;
    cpu->ir = 0;
    cpu->halted = 0;
   // cpu->reg[13] = cpu->mem_size - 1;
}

void init_cpu(CPU *cpu, int mem_size) {
    cpu->memory = malloc(mem_size * sizeof(uint32_t));
    if (!cpu->memory) exit(1);
    cpu->mem_size = mem_size; //store size
    reset_cpu(cpu);
}

int load_program(CPU *cpu, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    uint32_t *temp = malloc(cpu->mem_size * sizeof(uint32_t));
    if (!temp) {
        fclose(f);
        return 0;
    }

    unsigned char bytes[4];
    int i = 0;

    while (fread(bytes, 1, 4, f) == 4) {
       if (i >= cpu->mem_size) {
        fclose(f);
        free(temp);
        return 0;
       }
       temp[i++] =
           (bytes[0]) |
           (bytes[1] << 8) |
           (bytes[2] << 16) |
           (bytes[3] << 24);
    }

    fclose(f);

    for (int j = 0; j < i; j++)
        cpu->memory[j] = temp[j];

    for (int j = i; j < cpu->mem_size; j++)
        cpu->memory[j] = 0;

    free(temp);

    for (int j = 0; j < 16; j++)
        cpu->reg[j] = 0;

    cpu->reg[13] = cpu->mem_size - 1;
    cpu->pc = 0;
    cpu->ir = 0;
    cpu->halted = 0;

    return 1;
}

static void print_word(uint32_t w) {
    printf("%02x%02x%02x%02x\n",
           w & 0xFF,
           (w >> 8) & 0xFF,
           (w >> 16) & 0xFF,
           (w >> 24) & 0xFF);
}

void print_memory(CPU *cpu) {
    for (int i = 0; i < cpu->mem_size; i++)
        print_word(cpu->memory[i]);
}

void print_registers(CPU *cpu) {
    for (int i = 0; i < 16; i++)
        print_word(cpu->reg[i]);
}

void print_pc(CPU *cpu) {
    print_word(cpu->pc);
}

void print_ir(CPU *cpu) {
    print_word(cpu->ir);
}