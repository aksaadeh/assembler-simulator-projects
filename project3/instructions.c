#include "cpu.h"
#include <stdint.h>
#include <stdio.h>

static void enqueue(CPU *cpu, int pid) {
    cpu->sched.ready_queue[cpu->sched.rq_rear % MAX_PROCESSES] = pid;
    cpu->sched.rq_rear++;
}

static int dequeue(CPU *cpu) {
    while (cpu->sched.rq_front < cpu->sched.rq_rear) {
        int pid = cpu->sched.ready_queue[cpu->sched.rq_front % MAX_PROCESSES];
        cpu->sched.rq_front++;
        if (cpu->sched.processes[pid].active)
            return pid;
    }
    return -1;
}

static void save_current(CPU *cpu) {
    int pid = cpu->sched.current_pid;
    if (pid < 0) return;

    Process *p = &cpu->sched.processes[pid];
    p->pc = cpu->pc;
    p->ir = cpu->ir;
    for (int i = 0; i < 16; i++)
        p->reg[i] = cpu->reg[i];
}

static void restore_process(CPU *cpu, int pid) {
    Process *p = &cpu->sched.processes[pid];

    cpu->sched.current_pid = pid;
    cpu->sched.slice_counter = 0;

    cpu->pc = p->pc;
    cpu->ir = p->ir;
    for (int i = 0; i < 16; i++)
        cpu->reg[i] = p->reg[i];
}

static void dispatch(CPU *cpu) {
    if (cpu->sched.current_pid != -1)
        save_current(cpu);

    int pid = dequeue(cpu);
    if (pid == -1) {
        cpu->sched.current_pid = -1;
        return;
    }
    restore_process(cpu, pid);
}

static void terminate(CPU *cpu, int pid) {
    Process *p = &cpu->sched.processes[pid];
    if (!p->active) return;

    p->active = 0;

    for (int i = p->base; i < p->base + p->limit; i++)
        cpu->memory[i] = 0;

    cpu->sched.current_pid = -1;
}

static int runtime_error(CPU *cpu, int pid) {
    terminate(cpu, pid);
    dispatch(cpu);
    return -1;
}

int step(CPU *cpu) {
    // dispatch if needed
    if (cpu->sched.current_pid == -1)
        dispatch(cpu);

    if (cpu->sched.current_pid == -1)
        return -1;

    int pid = cpu->sched.current_pid;
    Process *p = &cpu->sched.processes[pid];

    // bounds check
    if ((int)cpu->pc < 0 || (int)cpu->pc >= p->limit)
        return runtime_error(cpu, pid);

    if (p->base + cpu->pc >= cpu->mem_size)
        return runtime_error(cpu, pid);

    uint32_t instr = cpu->memory[p->base + cpu->pc];
    cpu->ir = instr;

    uint8_t opcode = (instr >> 28) & 0xF;
    uint8_t regA   = (instr >> 24) & 0xF;
    uint8_t regB   = (instr >> 20) & 0xF;
    uint8_t regC   =  instr & 0xF;

    int32_t imm = (int32_t)(instr & 0xFFFFF);
    if (imm & 0x80000) imm |= (int32_t)0xFFF00000;

    switch (opcode) {

    case 0:
        if (regA == 0) return runtime_error(cpu, pid);
        cpu->reg[regA] = cpu->reg[regB] + cpu->reg[regC];
        cpu->pc++;
        break;

    case 1:
        if (regA == 0) return runtime_error(cpu, pid);
        cpu->reg[regA] = ~(cpu->reg[regB] & cpu->reg[regC]);
        cpu->pc++;
        break;

    case 2:
        if (regA == 0) return runtime_error(cpu, pid);
        cpu->reg[regA] = (uint32_t)((int32_t)cpu->reg[regB] + imm);
        cpu->pc++;
        break;

    case 3: { // lw
        if (regA == 0) return runtime_error(cpu, pid);
        int32_t addr = (int32_t)cpu->reg[regB] + imm + p->base;
        if (addr < p->base || addr >= p->base + p->limit)
            return runtime_error(cpu, pid);
        cpu->reg[regA] = cpu->memory[addr];
        cpu->pc++;
        break;
    }

    case 4: { // sw
        int32_t addr = (int32_t)cpu->reg[regB] + imm + p->base;
        if (addr < p->base || addr >= p->base + p->limit)
            return runtime_error(cpu, pid);
        cpu->memory[addr] = cpu->reg[regA];
        cpu->pc++;
        break;
    }

    case 5:
        if (cpu->reg[regA] == cpu->reg[regB])
            cpu->pc = (uint32_t)imm;
        else
            cpu->pc++;
        break;

    case 6:
        if (regB == 0) return runtime_error(cpu, pid);
        cpu->reg[regB] = cpu->pc + 1;
        cpu->pc = cpu->reg[regA];
        break;

    case 7: // halt
        save_current(cpu);
        terminate(cpu, pid);
        cpu->sched.current_pid = -1;
        dispatch(cpu);
        return 0;

    default:
        return runtime_error(cpu, pid);
    }

    // round-robin
    cpu->sched.slice_counter++;

    if (cpu->sched.slice_counter >= cpu->sched.timeslice) {
     save_current(cpu);
     int already_in_queue = 0;
     for (int i = cpu->sched.rq_front; i < cpu->sched.rq_rear; i++) {
        if (cpu->sched.ready_queue[i % MAX_PROCESSES] == pid) {
            already_in_queue = 1;
            break;
        }
     }

     if (cpu->sched.processes[pid].active && !already_in_queue) {
        enqueue(cpu, pid);
     }

     cpu->sched.current_pid = -1;
     dispatch(cpu);
    }

    return 0;
}

int run(CPU *cpu) {
    if (cpu->sched.current_pid == -1 && cpu->sched.rq_front == cpu->sched.rq_rear) { return -1; }
    while (cpu->sched.current_pid != -1 || cpu->sched.rq_front < cpu->sched.rq_rear) {
         if (step(cpu) != 0) return -1;
    }
    return 0;
}