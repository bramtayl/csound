#pragma once

typedef struct instr {
    struct instr *next;
    int n;                    /* self referencial */
    int largest;
    double *p;
} INSTR;

void print_instr_structure(void);
INSTR *find_instr(int);
