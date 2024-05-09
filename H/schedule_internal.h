#pragma once

#include "csound.h"
#include "schedule.h"

int32_t schedule(CSOUND *, SCHED *), schedule_S(CSOUND *, SCHED *);
int32_t schedule_N(CSOUND *, SCHED *), schedule_SN(CSOUND *, SCHED *);
int32_t ifschedule(CSOUND *, WSCHED *), kschedule(CSOUND *, WSCHED *);
int32_t triginset(CSOUND *, TRIGINSTR *), ktriginstr(CSOUND *, TRIGINSTR *);
int32_t trigseq_set(CSOUND *, TRIGSEQ *), trigseq(CSOUND *, TRIGSEQ *);
int32_t lfoset(CSOUND *, LFO *);
int32_t lfok(CSOUND *, LFO *), lfoa(CSOUND *, LFO *);
int32_t schedule_array(CSOUND *csound, SCHED *p);

