#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
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

void flush_line() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

char *get_arg(char *arg) {
    static char buffer[256];

    while (!arg) {
        if (!fgets(buffer, sizeof(buffer), stdin))
            return NULL;

        if (!strchr(buffer, '\n'))
            flush_line();

        buffer[strcspn(buffer, "\r\n")] = 0;
        trim(buffer);

        if (buffer[0] != '\0') //if not empty take the first
            arg = strtok(buffer, " \t");
    }
    return arg;
}

void run_shell(CPU *cpu) {
    char line[256];

    printf("> ");

    while (1) {
        if (!fgets(line, sizeof(line), stdin))
            break; //empty

        if (!strchr(line, '\n'))
            flush_line();

        line[strcspn(line, "\r\n")] = 0;
        trim(line);

        if (line[0] == '\0')
            continue;

        char *token = strtok(line, " \t");

        while (token != NULL) {
            if (strcmp(token, "load") == 0) {
                char *arg = strtok(NULL, " \t");
                arg = get_arg(arg);

                if (!arg || !load_program_multi(cpu, arg))
                    printf("File Error\n");

                printf("> ");
                break; 
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

            else if (strcmp(token, "jobs") == 0) {
                print_jobs(cpu);
                printf("> ");
            }

            else if (strcmp(token, "kill") == 0) {
                char *arg = strtok(NULL, " \t");
                arg = get_arg(arg);

                int valid = 1;
                for (int i = 0; arg && arg[i]; i++) {
                  if (!isdigit(arg[i])) {
                    valid = 0;
                    break;
                  }
                }

                if (!arg || !valid || !kill_process(cpu, atoi(arg))) {
                  printf("ID Error\n");
                }
                printf("> ");
                break; 
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