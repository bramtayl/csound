#pragma once

#include "csound.h"
#include "pstream.h"

int32_t pvadsynset(CSOUND *, PVADS *), pvadsyn(CSOUND *, PVADS *);
int32_t pvscrosset(CSOUND *, PVSCROSS *), pvscross(CSOUND *, PVSCROSS *);
int32_t pvsfreadset(CSOUND *, PVSFREAD *), pvsfread(CSOUND *, PVSFREAD *);
int32_t pvsmaskaset(CSOUND *, PVSMASKA *), pvsmaska(CSOUND *, PVSMASKA *);
int32_t pvsftwset(CSOUND *, PVSFTW *), pvsftw(CSOUND *, PVSFTW *);
int32_t pvsftrset(CSOUND *, PVSFTR *), pvsftr(CSOUND *, PVSFTR *);
int32_t pvsinfo(CSOUND *, PVSINFO *);
int32_t fassign(CSOUND *, FASSIGN *);
int32_t fassign_set(CSOUND *csound, FASSIGN *p);
int32_t pvsfreadset_S(CSOUND *csound, PVSFREAD *p);