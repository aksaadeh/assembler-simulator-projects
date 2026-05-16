#ifndef CPU_H
#define CPU_H

#include <stdint.h>

typedef struct {
    uint32_t *memory;   // memory array
    uint32_t reg[32];   // registers
    uint32_t pc;        // program counter
    uint32_t ir;        // instruction register
    int mem_size;       // memory size in words
    int halted;         // halt flag
} CPU;

void init_cpu(CPU *cpu, int mem_size);
void reset_cpu(CPU *cpu);
int load_program(CPU *cpu, const char *filename);

void print_memory(CPU *cpu);
void print_registers(CPU *cpu);
void print_pc(CPU *cpu);
void print_ir(CPU *cpu);

#endif