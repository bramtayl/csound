#pragma once

#include "csound.h"
#include "clarinet.h"
#include "flute.h"
#include "bowed.h"
#include "brass.h"

int32_t clarinset(CSOUND *, CLARIN *), clarin(CSOUND *, CLARIN *);
int32_t fluteset(CSOUND *, FLUTE *), flute(CSOUND *, FLUTE *);
int32_t bowedset(CSOUND *, BOWED *), bowed(CSOUND *, BOWED *);
int32_t brassset(CSOUND *, BRASS *), brass(CSOUND *, BRASS *);