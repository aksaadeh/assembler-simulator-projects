#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

bool isBlank(const char *line) {
    while (*line) {
        if (!isspace((unsigned char)*line)) return false;
        line++;
    }
    return true;
}

bool isIdentifier(const char *token) {
    if (!token || !isalpha((unsigned char)token[0])) return false;
    for (int i = 1; token[i]; i++) {
        if (!isalpha((unsigned char)token[i])) return false;
    }
    return true;
}

void removeComment(char *line) {
    char *p = strchr(line, '%');
    if (p) *p = '\0';
}

int getRegister(const char *reg) {
    if (!reg) return -1;
    if (!strcmp(reg, "$zero")) return 0;
    if (!strcmp(reg, "$at"))   return 1;
    if (!strcmp(reg, "$v0"))   return 2;
    if (!strcmp(reg, "$a0"))   return 3;
    if (!strcmp(reg, "$a1"))   return 4;
    if (!strcmp(reg, "$a2"))   return 5;
    if (!strcmp(reg, "$t0"))   return 6;
    if (!strcmp(reg, "$t1"))   return 7;
    if (!strcmp(reg, "$t2"))   return 8;
    if (!strcmp(reg, "$s0"))   return 9;
    if (!strcmp(reg, "$s1"))   return 10;
    if (!strcmp(reg, "$s2"))   return 11;
    if (!strcmp(reg, "$k0"))   return 12;
    if (!strcmp(reg, "$sp"))   return 13;
    if (!strcmp(reg, "$fp"))   return 14;
    if (!strcmp(reg, "$ra"))   return 15;
    return -1;
}

void printLittleEndian(unsigned int word) {
    unsigned char bytes[4];
    bytes[0] = (word >> 0) & 0xFF;
    bytes[1] = (word >> 8) & 0xFF;
    bytes[2] = (word >> 16) & 0xFF;
    bytes[3] = (word >> 24) & 0xFF;
    for (int i = 0; i < 4; i++) printf("%02x", bytes[i]);
    printf("\n");
}