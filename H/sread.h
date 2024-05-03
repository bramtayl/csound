#pragma once

#include "csound.h"

MYFLT   stof(CSOUND *, char *);
void sfree(CSOUND *csound);
int sread(CSOUND *csound);
void sread_initstr(CSOUND *csound, CORFIL *sco);