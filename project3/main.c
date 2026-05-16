#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>

extern void run_shell(CPU *cpu);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Argument Error\n");
        return 1;
    }

    int mem_size = atoi(argv[1]);
    int stack_size = atoi(argv[2]);
    int timeslice = atoi(argv[3]);

    if (mem_size <= 0 || stack_size <= 0 || timeslice <= 0) {
        printf("Argument Error\n");
        return 1;
    }

    CPU cpu;
    init_cpu(&cpu, mem_size);

    cpu.stack_size = stack_size;  
    cpu.sched.timeslice = timeslice;

    run_shell(&cpu);

    free(cpu.memory);
    return 0;
}