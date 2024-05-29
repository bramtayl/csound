#pragma once

#include <stdio.h>

typedef struct instr {
    struct instr *next;
    int n;                    /* self referencial */
    int largest;
    double *p;
} INSTR;

extern int debug;
extern FILE *myout;
extern double pt[13];

void print_instr_structure(void);
INSTR *find_instr(int);
