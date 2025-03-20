#ifndef VIRUS_H
#define VIRUS_H

#include <stdio.h>

// Structure definition
typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

// Function prototypes
virus* readVirus(FILE* file);
void freeVirus(virus* v);
void printVirus(virus* v, FILE* output);

#endif // VIRUS_H
