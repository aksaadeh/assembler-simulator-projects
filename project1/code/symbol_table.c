#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"

void resetSymbolTable(Assembler *assembler) {
    for (int i = 0; i < assembler->symbolCount; i++)
        free(assembler->table[i].label);
    free(assembler->table);
    assembler->table = NULL;
    assembler->symbolCount = 0;
}

void addSymbol(Assembler *assembler, const char *label, int address) {
    // check duplicates
    for (int i = 0; i < assembler->symbolCount; i++) {
        if (strcmp(assembler->table[i].label, label) == 0)
            return; // duplicate handled in Pass1
    }

    Symbol *newTable = realloc(assembler->table, sizeof(Symbol) * (assembler->symbolCount + 1));
    if (!newTable) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    assembler->table = newTable;

    assembler->table[assembler->symbolCount].label = strdup(label);
    assembler->table[assembler->symbolCount].address = address;
    assembler->symbolCount++;
}

int getSymbolAddress(Assembler *assembler, const char *label) {
    for (int i = 0; i < assembler->symbolCount; i++) {
        if (strcmp(assembler->table[i].label, label) == 0)
            return assembler->table[i].address;
    }

    if (assembler->currentLine <= 0)
        assembler->currentLine = 1;

    syntaxError(assembler, assembler->currentLine);
    return -1;
}