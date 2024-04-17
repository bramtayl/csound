#pragma once

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

#include "csoundCore.h"

PUBLIC int insert_score_event(CSOUND *, EVTBLK *, double);

PUBLIC int insert_score_event_at_sample(CSOUND *, EVTBLK *, int64_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */