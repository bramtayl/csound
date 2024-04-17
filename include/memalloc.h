#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

PUBLIC void *mmalloc(CSOUND *, size_t);
PUBLIC void *mcalloc(CSOUND *, size_t);
PUBLIC void *mrealloc(CSOUND *, void *, size_t);
PUBLIC void mfree(CSOUND *, void *);

#ifdef __cplusplus
}
#endif