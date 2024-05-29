#pragma once

#include "csound.h"
#include "score_param.h"

void cs_init_smacros(CSOUND *csound, PRS_PARM *qq, NAMES *nn);
int csound_prslex(CSOUND*, void*);
int csound_prslex_destroy(void *);
int csound_prslex_init(void**);
void csound_prsset_extra(PRS_PARM *, void *);
int csound_prslex(CSOUND*, void*);
int csound_prslex_destroy(void *);


