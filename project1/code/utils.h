#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

bool isBlank(const char *line);
bool isIdentifier(const char *token);
void removeComment(char *line);
int getRegister(const char *reg);
void printLittleEndian(unsigned int word);

#endif