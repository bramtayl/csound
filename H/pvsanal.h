#pragma once

#include "csound.h"
#include "pstream.h"

double besseli(double);
int32_t pvsanalset(CSOUND *, PVSANAL *), pvsanal(CSOUND *, PVSANAL *);
int32_t pvsynthset(CSOUND *, PVSYNTH *), pvsynth(CSOUND *, PVSYNTH *);