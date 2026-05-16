#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_cpu(CPU *cpu, int mem_size) {
    cpu->memory = malloc(mem_size * sizeof(uint32_t));
    if (!cpu->memory) exit(1);

    cpu->mem_size = mem_size;
    cpu->stack_size = 0;
    cpu->pc = 0;
    cpu->ir = 0;

    for (int i = 0; i < mem_size; i++)
        cpu->memory[i] = 0;

    for (int i = 0; i < 16; i++)
        cpu->reg[i] = 0;

    cpu->sched.process_count = 0;
    cpu->sched.rq_front = 0;
    cpu->sched.rq_rear = 0;
    cpu->sched.current_pid = -1;
    cpu->sched.slice_counter = 0;
    cpu->sched.timeslice = 0;
}

int load_program_multi(CPU *cpu, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    uint32_t *temp = malloc(cpu->mem_size * sizeof(uint32_t));
    if (!temp) { fclose(f); return 0; }

    unsigned char buf[4];
    int code_size = 0;

    while (fread(buf, 1, 4, f) == 4) {
        if (code_size >= cpu->mem_size) {
            free(temp); fclose(f); return 0;
        }
        temp[code_size++] =
            (uint32_t)buf[0]         |
            ((uint32_t)buf[1] <<  8) |
            ((uint32_t)buf[2] << 16) |
            ((uint32_t)buf[3] << 24);
    }
    fclose(f);

    //find a place in mem thats is big enough and put the prog
    int total = code_size + cpu->stack_size; //mem it needs
    if (total > cpu->mem_size) { free(temp); return 0; }
    int base = -1;

    for (int i = 0; i <= cpu->mem_size - total; i++) {
        int ok = 1; //assume free

        for (int j = 0; j < cpu->sched.process_count; j++) {
            Process *pr = &cpu->sched.processes[j];
            if (!pr->active) continue;

            int pstart = pr->base;
            int pend = pr->base + pr->limit;

            if (i + total > pstart && i < pend) {
                ok = 0;
                i  = pend - 1; //skip ahead
                break;
            }
        }

        if (ok) { base = i; break; } //found a valid location
    }

    if (base == -1) { free(temp); return 0; }


    //write to mem
    for (int i = 0; i < code_size; i++)
        cpu->memory[base + i] = temp[i];

    for (int i = code_size; i < total; i++)
        cpu->memory[base + i] = 0; //stack to 0

    free(temp);

    //create process
    int pid = cpu->sched.process_count++;
    Process *p = &cpu->sched.processes[pid]; //get process struct


    //store id and filename
    p->pid = pid;
    strncpy(p->name, filename, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';

    p->base = base; //code address
    p->limit = total;
    p->active = 1;
    p->pc = 0;  
    p->ir = 0;
    for (int i = 0; i < 16; i++)
        p->reg[i] = 0;

        //add to queue
    p->stack = (uint32_t)(base + total - 1);//top of mem
    p->reg[13] = p->stack;
    cpu->sched.ready_queue[cpu->sched.rq_rear % MAX_PROCESSES] = pid;
    cpu->sched.rq_rear++;

    //no process was running, run it
    if (cpu->sched.current_pid == -1) {
        cpu->sched.rq_front++; 
        cpu->sched.current_pid = pid;
        cpu->sched.slice_counter = 0;
        cpu->pc = p->pc;
        cpu->ir = p->ir;
        for (int i = 0; i < 16; i++)
            cpu->reg[i] = p->reg[i];
    }

    return 1;
}

int kill_process(CPU *cpu, int pid) {
    if (pid < 0 || pid >= cpu->sched.process_count)
        return 0;

    Process *p = &cpu->sched.processes[pid];
    if (!p->active)
        return 0;

    for (int i = p->base; i < p->base + p->limit; i++)
        cpu->memory[i] = 0; // clear all mem used by it

    p->active = 0;

    if (cpu->sched.current_pid == pid) {
        cpu->sched.current_pid = -1;
        cpu->sched.slice_counter = 0;
        cpu->pc = 0;
        cpu->ir = 0;
        for (int i = 0; i < 16; i++)
            cpu->reg[i] = 0;


            //dispatch
        while (cpu->sched.rq_front < cpu->sched.rq_rear) {
            int next = cpu->sched.ready_queue[cpu->sched.rq_front % MAX_PROCESSES];
            cpu->sched.rq_front++; //remove it from q
            if (cpu->sched.processes[next].active) {
                Process *np = &cpu->sched.processes[next];
                cpu->sched.current_pid = next;
                cpu->sched.slice_counter = 0;
                cpu->pc = np->pc;
                cpu->ir = np->ir;
                for (int i = 0; i < 16; i++)
                    cpu->reg[i] = np->reg[i];
                break;
            }
        }
    }

    return 1;
}

static void print_word(uint32_t w) {
    printf("%02x%02x%02x%02x\n",
           w & 0xFF,
           (w >>  8) & 0xFF,
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

void print_pc(CPU *cpu) { print_word(cpu->pc); }
void print_ir(CPU *cpu) { print_word(cpu->ir); }
void print_jobs(CPU *cpu) {
    int first = 1;

    // 1. current process is HEAD — print it first
    int cur = cpu->sched.current_pid;
    if (cur != -1 && cpu->sched.processes[cur].active) {
        Process *p = &cpu->sched.processes[cur];
        uint32_t ca = (uint32_t)p->base;
        uint32_t sa = p->stack;

        printf("Process ID: %d\n", p->pid);
        printf("Process Name: %s\n", p->name);
        printf("Code Address: %02x%02x%02x%02x\n",
               ca & 0xFF, (ca >> 8) & 0xFF,
               (ca >> 16) & 0xFF, (ca >> 24) & 0xFF);
        printf("Stack Address: %02x%02x%02x%02x\n",
               sa & 0xFF, (sa >> 8) & 0xFF,
               (sa >> 16) & 0xFF, (sa >> 24) & 0xFF);
        first = 0;
    }

    // 2. then the rest of the queue (tail)
    for (int i = cpu->sched.rq_front; i < cpu->sched.rq_rear; i++) {
        int pid = cpu->sched.ready_queue[i % MAX_PROCESSES];
        Process *p = &cpu->sched.processes[pid];
        if (!p->active) continue;

        if (!first) printf("\n");
        first = 0;

        uint32_t ca = (uint32_t)p->base;
        uint32_t sa = p->stack;

        printf("Process ID: %d\n", p->pid);
        printf("Process Name: %s\n", p->name);
        printf("Code Address: %02x%02x%02x%02x\n",
               ca & 0xFF, (ca >> 8) & 0xFF,
               (ca >> 16) & 0xFF, (ca >> 24) & 0xFF);
        printf("Stack Address: %02x%02x%02x%02x\n",
               sa & 0xFF, (sa >> 8) & 0xFF,
               (sa >> 16) & 0xFF, (sa >> 24) & 0xFF);
    }
}