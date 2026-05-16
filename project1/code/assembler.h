#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>

typedef struct {
    char *label;    
    int address;
} Symbol;

typedef struct {
    Symbol *table;  
    int symbolCount;
    FILE *fp;
    int currentLine;
    int duplicateLabelLine; 
} Assembler;

void assemble(Assembler *assembler);
void syntaxError(Assembler *assembler, int line);
void initAssembler(Assembler *assembler, FILE *fp);
void freeAssembler(Assembler *assembler);

#endif