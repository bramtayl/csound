#pragma once
#include "csound.h"
#include "bowed.h"
#include "brass.h"
#include "clarinet.h"

MYFLT BowTabl_lookup(CSOUND *,BowTabl*, MYFLT sample);

void make_DLineA(CSOUND *,DLineA *, int32 max_length);
/* void DLineA_clear(DLineA *); */
int32_t DLineA_setDelay(CSOUND *,DLineA *, MYFLT length);
MYFLT DLineA_tick(DLineA *, MYFLT sample);
void LipFilt_setFreq(CSOUND*,LipFilt*, MYFLT frequency);
MYFLT LipFilt_tick(LipFilt*, MYFLT mouthSample,MYFLT boreSample);
void make_OneZero(OneZero*);
MYFLT OneZero_tick(OneZero*, MYFLT);
void OneZero_setCoeff(OneZero*, MYFLT);
// void OneZero_print(CSOUND*, OneZero*);