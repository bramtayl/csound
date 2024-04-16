#pragma once

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

#ifdef __BUILDING_LIBCSOUND
#include "csoundCore_internal.h"
#else
#include "csoundCore.h"
#endif

PUBLIC int insert_score_event(CSOUND *, EVTBLK *, double);

PUBLIC int insert_score_event_at_sample(CSOUND *, EVTBLK *, int64_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */