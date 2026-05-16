#include "cpu.h"
#include <stdio.h>
#include <stdint.h>

int step(CPU *cpu) {
    if (cpu->halted) return 0;

    if (cpu->pc < 0 || cpu->pc >= cpu->mem_size)
        return -1;

    uint32_t instr = cpu->memory[cpu->pc];
    cpu->ir = instr;

    uint8_t opcode = (instr >> 28) & 0xF;
    uint8_t regA   = (instr >> 24) & 0xF;
    uint8_t regB   = (instr >> 20) & 0xF;
    uint8_t regC = instr & 0xF;
    int32_t imm    = instr & 0xFFFFF;
    if (imm & 0x80000) imm |= 0xFFF00000;

    switch (opcode) {
    case 0: // ADD
        if (regA == 0) return -1; // writing $zero
        cpu->reg[regA] = cpu->reg[regB] + cpu->reg[regC];
        cpu->pc += 1;
        break;
    case 1: // NAND
        if (regA == 0) return -1;
        cpu->reg[regA] = ~(cpu->reg[regB] & cpu->reg[regC]);
        cpu->pc += 1;
        break;
    case 2: // ADDI
        if (regA == 0) return -1;
        cpu->reg[regA] = cpu->reg[regB] + imm;
        cpu->pc += 1;
        break;
    case 3: // LW
    {
        int32_t addr = cpu->reg[regB] + imm;
        if (addr < 0 || addr >= cpu->mem_size) return -1;
        if (regA == 0) return -1;
        cpu->reg[regA] = cpu->memory[addr];
        cpu->pc += 1;
        break;
    }
    case 4: // SW
    {
        int32_t addr = cpu->reg[regB] + imm;
        if (addr < 0 || addr >= cpu->mem_size) return -1;
        cpu->memory[addr] = cpu->reg[regA];
        cpu->pc += 1;
        break;
    }
    case 5: // BEQ
        if (cpu->reg[regA] == cpu->reg[regB])
            cpu->pc = imm;
        else
            cpu->pc += 1;
        break;
    case 6: // JALR
        if (regB == 0) return -1;
        uint32_t next = cpu->pc + 1;
        cpu->reg[regB] = next;
        cpu->pc = cpu->reg[regA];
        break;
    case 7: // HALT
        cpu->halted = 1;
        break;
    default:
        return -1;
    }
    return 0;
}

int run(CPU *cpu) {
    while (!cpu->halted) {
        if (step(cpu) != 0) return -1;
    }
    return 0;
}