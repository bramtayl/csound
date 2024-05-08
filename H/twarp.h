#pragma once

#include "csoundCore_common.h"
#include "sort.h"

int     realtset(CSOUND *, SRTBLK *);
MYFLT   realt(CSOUND *, MYFLT);
void twarp(CSOUND *csound); /* time-warp a score section acc to T-statement */