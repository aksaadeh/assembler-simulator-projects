#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "assembler.h"

void resetSymbolTable(Assembler *assembler);
void addSymbol(Assembler *assembler, const char *label, int address);
int getSymbolAddress(Assembler *assembler, const char *label);

#endif