#include "cpu.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Argument Error\n");
        return 1;
    }

    int mem_size = atoi(argv[1]);
    if (mem_size <= 0) {
        printf("Argument Error\n");
        return 1;
    }

    CPU cpu;
    init_cpu(&cpu, mem_size);

    run_shell(&cpu);

    free(cpu.memory);
    return 0;
}