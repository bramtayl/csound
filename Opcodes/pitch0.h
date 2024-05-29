#pragma once

#include "csound.h"
#include "pitch.h"

int32_t cpuperc(CSOUND *, CPU_PERC *p);
int32_t cpuperc_S(CSOUND *, CPU_PERC *p);
int32_t instcount(CSOUND *, INSTCNT *p);
int32_t instcount_S(CSOUND *, INSTCNT *p);
int32_t maxalloc(CSOUND *, CPU_PERC *p);
int32_t mute_inst(CSOUND *, MUTE *p);
int32_t maxalloc_S(CSOUND *, CPU_PERC *p);
int32_t mute_inst_S(CSOUND *, MUTE *p);
int32_t pfun(CSOUND *, PFUN *p);
int32_t pfunk_init(CSOUND *, PFUNK *p);
int32_t pfunk(CSOUND *, PFUNK *p);