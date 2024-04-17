#pragma once

#include "csound.h"
#include "csoundCore_common.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

PUBLIC CS_PRINTF2 int csoundInitError(CSOUND *, const char *, ...);
PUBLIC CS_PRINTF3 int csoundPerfError(CSOUND *, OPDS *h, const char *, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */