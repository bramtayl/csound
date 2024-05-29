#pragma once

#include "csound.h"             // for CSOUND
#include "csoundCore_common.h"  // for INSDS
#include "sysdep.h"             // for CS_NOINLINE

void auxchfree(CSOUND *, INSDS *);
void fdchclose(CSOUND *, INSDS *);
CS_NOINLINE void fdchprint(CSOUND *csound, INSDS *ip);
CS_NOINLINE void auxchprint(CSOUND *csound, INSDS *ip);