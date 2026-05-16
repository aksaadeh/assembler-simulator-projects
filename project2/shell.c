#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cpu.h"
#include "instructions.h"

void trim(char *s) {
    char *start = s;
    while (isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);

    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

void run_shell(CPU *cpu) {
    char line[256];

    printf("> ");

    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\r\n")] = 0;
        trim(line);

        if (line[0] == '\0') continue;

        char *token = strtok(line, " \t");

        while (token != NULL) {

            if (strcmp(token, "load") == 0) {
                char *arg = strtok(NULL, " \t");

                while (!arg) {
                    if (!fgets(line, sizeof(line), stdin)) break;
                    line[strcspn(line, "\r\n")] = 0;
                    trim(line);
                    if (line[0] != '\0')
                        arg = strtok(line, " \t");
                }

                if (!arg || !load_program(cpu, arg)) {
                    printf("File Error\n");
                }

                printf("> ");
            }
            else if (strcmp(token, "mem") == 0) {
                print_memory(cpu);
                printf("> ");
            }

            else if (strcmp(token, "reg") == 0) {
                print_registers(cpu);
                printf("> ");
            }

            else if (strcmp(token, "pc") == 0) {
                print_pc(cpu);
                printf("> ");
            }

            else if (strcmp(token, "ir") == 0) {
                print_ir(cpu);
                printf("> ");
            }

            else if (strcmp(token, "step") == 0) {
                if (step(cpu) != 0)
                    printf("Runtime Error\n");
                printf("> ");
            }

            else if (strcmp(token, "run") == 0) {
                if (run(cpu) != 0)
                    printf("Runtime Error\n");
                printf("> ");
            }

            else if (strcmp(token, "exit") == 0) {
                return;
            }

            else {
                printf("Command Error\n");
                printf("> ");
            }

            token = strtok(NULL, " \t");
        }
    }
}