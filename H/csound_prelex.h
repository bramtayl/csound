#pragma once

#include "csound.h"
#include "parse_param.h"

int isDir(char *);

int csound_prelex(CSOUND*, void*);
int csound_prelex_destroy(void *);
int csound_prelex_init(void **);
void csound_preset_extra(PRE_PARM *, void *);
