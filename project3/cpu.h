#ifndef CPU_H
#define CPU_H
#include <stdint.h>
#define MAX_PROCESSES 1024

typedef struct {
    int pid;
    char name[256];

    int base;      
    int limit; 

    uint32_t pc;
    uint32_t ir;

    uint32_t reg[16];

    uint32_t stack;

    int active;
} Process;

typedef struct {
    Process processes[MAX_PROCESSES];

    int ready_queue[MAX_PROCESSES];
    int rq_front;
    int rq_rear;

    int current_pid;
    int slice_counter;

    int process_count;

    int timeslice;
} Scheduler;

typedef struct {
    uint32_t *memory;
    int mem_size;

    uint32_t reg[16];
    uint32_t pc;
    uint32_t ir;

    int stack_size;

    Scheduler sched;
} CPU;

void init_cpu(CPU *cpu, int mem_size);

int load_program_multi(CPU *cpu, const char *filename);

int kill_process(CPU *cpu, int pid);

int step(CPU *cpu);
int run(CPU *cpu);

void print_memory(CPU *cpu);
void print_registers(CPU *cpu);
void print_pc(CPU *cpu);
void print_ir(CPU *cpu);
void print_jobs(CPU *cpu);

#endif
