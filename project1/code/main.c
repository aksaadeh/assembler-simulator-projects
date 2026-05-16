#include <stdio.h>
#include <stdlib.h>
#include "assembler.h"

int main(int argc, char *argv[]) {
    if (argc != 2) { printf("Argument Error\n"); return 1; }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) { printf("File Error\n"); return 1; }

    Assembler assembler = { .fp = fp, .symbolCount = 0, .currentLine = 0 };
    assemble(&assembler);

    fclose(fp);
    return 0;
}