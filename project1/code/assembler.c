#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "symbol_table.h"
#include "utils.h"

void initAssembler(Assembler *assembler, FILE *fp) {
    assembler->fp = fp;
    assembler->currentLine = 0;
    assembler->symbolCount = 0;
    assembler->table = NULL;
    assembler->duplicateLabelLine = 0;
}

void freeAssembler(Assembler *assembler) {
    for (int i = 0; i < assembler->symbolCount; i++)
        free(assembler->table[i].label);
    free(assembler->table);
}

void syntaxError(Assembler *assembler, int line) {
    printf("Syntax Error on line %d\n", line);
    exit(1);
}

void assemble(Assembler *assembler) {
    char *line = NULL;
    size_t len = 0;
    int address = 0;

    resetSymbolTable(assembler);

    int firstErrorLine = 0;

    // PASS 1: Symbol Table
    while (getline(&line, &len, assembler->fp) != -1) {
        assembler->currentLine++;
        removeComment(line);
        if (isBlank(line)) continue;

        char *copy = strdup(line);
        char *token = strtok(copy, " \t\r\n");

        if (token && strchr(token, ':')) {
            token[strlen(token) - 1] = '\0';

            if (!isIdentifier(token)) {
                if (firstErrorLine == 0 || assembler->currentLine < firstErrorLine)
                    firstErrorLine = assembler->currentLine;
            }

            // Check duplicate
            for (int i = 0; i < assembler->symbolCount; i++) {
                if (strcmp(assembler->table[i].label, token) == 0) {
                    if (firstErrorLine == 0 || assembler->currentLine < firstErrorLine)
                        firstErrorLine = assembler->currentLine;
                }
            }

            // Only add if not duplicate
            int exists = 0;
            for (int i = 0; i < assembler->symbolCount; i++) {
                if (strcmp(assembler->table[i].label, token) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (!exists)
                addSymbol(assembler, token, address);
        }

        free(copy);
        address++;
    }

    rewind(assembler->fp);
    assembler->currentLine = 0;
    address = 0;

    unsigned int *output = NULL;
    int outSize = 0, outCap = 0;
    int inDataSegment = 0;

    // PASS 2: Assemble
    while (getline(&line, &len, assembler->fp) != -1) {
        assembler->currentLine++;
        int len_line = strlen(line);
        if (len_line == 0 || line[len_line - 1] != '\n') {
            if (firstErrorLine == 0 || assembler->currentLine < firstErrorLine)
                firstErrorLine = assembler->currentLine;
            continue;
        }

        removeComment(line);
        if (isBlank(line)) continue;

        size_t linelen = strlen(line);
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            line[--linelen] = '\0';

        if (isBlank(line)) continue;

        char *tokens[10];
        int tokCount = 0;

        char *tok = strtok(line, " \t");
        while (tok) {
            if (tokCount >= 10) {
                if (firstErrorLine == 0 || assembler->currentLine < firstErrorLine)
                    firstErrorLine = assembler->currentLine;
                break;
            }
            tokens[tokCount++] = tok;
            tok = strtok(NULL, " \t");
        }

        if (tokCount == 0) continue;

        int index = 0;
        if (strchr(tokens[0], ':')) index = 1;

        if (index >= tokCount) {
            if (firstErrorLine == 0 || assembler->currentLine < firstErrorLine)
                firstErrorLine = assembler->currentLine;
            continue;
        }

        char *op = tokens[index++];
        int operands = tokCount - index;

        if (!strcmp(op, ".word")) {
            inDataSegment = 1;
        } else if (inDataSegment) {
            if (firstErrorLine == 0 || assembler->currentLine < firstErrorLine)
                firstErrorLine = assembler->currentLine;
            continue;
        }

        unsigned int word = 0;
        int error = 0;

        // R-TYPE
        if (!strcmp(op, "add") || !strcmp(op, "nand")) {
            if (operands != 3) error = 1;
            int rA = getRegister(tokens[index]);
            int rB = getRegister(tokens[index + 1]);
            int rC = getRegister(tokens[index + 2]);
            if (rA < 0 || rB < 0 || rC < 0) error = 1;
            int opcode = (!strcmp(op, "add")) ? 0 : 1;
            word = (opcode << 28) | (rA << 24) | (rB << 20) | (rC & 0xF);
        }

        // I-TYPE
        else if (!strcmp(op, "addi") || !strcmp(op, "lw") ||
                 !strcmp(op, "sw") || !strcmp(op, "beq")) {

            if (operands != 3) error = 1;
            int opcode = (!strcmp(op, "addi")) ? 2 :
                         (!strcmp(op, "lw"))   ? 3 :
                         (!strcmp(op, "sw"))   ? 4 : 5;
            int rA = getRegister(tokens[index]);
            int rB = getRegister(tokens[index + 1]);
            if (rA < 0 || rB < 0) error = 1;
            int imm;
            char *immStr = tokens[index + 2];
            if (isIdentifier(immStr)) {
                int addr = getSymbolAddress(assembler, immStr);
                if (addr < 0) error = 1;  // undefined label
                imm = addr;
            } else {
                char *end;
                long val = strtol(immStr, &end, 10);
                if (*end != '\0' || val < -524288 || val > 524287) error = 1;
                imm = (int)val;
            }
            word = (opcode << 28) | (rA << 24) | (rB << 20) | (imm & 0xFFFFF);
        }

        // J-TYPE
        else if (!strcmp(op, "jalr")) {
            if (operands != 2) error = 1;
            int rA = getRegister(tokens[index]);
            int rB = getRegister(tokens[index + 1]);
            if (rA < 0 || rB < 0) error = 1;
            word = (6 << 28) | (rA << 24) | (rB << 20);
        }

        // O-TYPE
        else if (!strcmp(op, "halt")) {
            if (operands != 0) error = 1;
            word = (7 << 28);
        }

        // .WORD
        else if (!strcmp(op, ".word")) {
            if (operands != 1) error = 1;
            char *valStr = tokens[index];
            int value;
            if (isIdentifier(valStr)) {
                int addr = getSymbolAddress(assembler, valStr);
                if (addr < 0) error = 1;
                value = addr;
            } else {
                char *end;
                long val = strtol(valStr, &end, 10);
                if (*end != '\0' || val < -2147483648L || val > 2147483647L) error = 1;
                value = (int)val;
            }
            word = (unsigned int)value;
        }

        else {
            error = 1;
        }

        if (error) {
            if (firstErrorLine == 0 || assembler->currentLine < firstErrorLine)
                firstErrorLine = assembler->currentLine;
            continue;
        }
        if (firstErrorLine == 0) {
            if (outSize >= outCap) {
                outCap = (outCap == 0) ? 16 : outCap * 2;
                output = realloc(output, outCap * sizeof(unsigned int));
            }
            output[outSize++] = word;
        }

        address++;
    }

    // PRINT FIRST ERROR IF ANY
    if (firstErrorLine > 0) {
        syntaxError(assembler, firstErrorLine);
    }

    // PRINT OUTPUT
    for (int i = 0; i < outSize; i++) {
        printLittleEndian(output[i]);
    }

    free(output);
    free(line);
}