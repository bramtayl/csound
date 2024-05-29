#pragma once

#include "csound.h"

/* These are used to set/clear bits in csound->tempStatus.
   If the bit is set, it indicates that the given file is
   a temporary. */
extern const uint32_t csOrcMask;
extern const uint32_t csScoInMask;
extern const uint32_t csScoSortMask;
extern const uint32_t csMidiScoMask;
extern const uint32_t csPlayScoMask;
#pragma once

#include "csound.h"

/* These are used to set/clear bits in csound->tempStatus.
   If the bit is set, it indicates that the given file is
   a temporary. */
extern const uint32_t csOrcMask;
extern const uint32_t csScoInMask;
extern const uint32_t csScoSortMask;
extern const uint32_t csMidiScoMask;
extern const uint32_t csPlayScoMask;

int     readOptions(CSOUND *, CORFIL *, int);
void    remove_tmpfiles(CSOUND *);