#pragma once

#include "csoundCore_common.h"

/* convert 6 bits of input and write to output file */
int encode_byte(FILE *infl, FILE *outfl);
/* convert an entire input file */
void encode_file(char *inflname, FILE *outfl, int style);
extern int     maxlinepos;                /* max line width */