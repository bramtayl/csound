#pragma once

#include "csoundCore_common.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

PUBLIC void csoundAuxAlloc(CSOUND *, size_t, AUXCH *);

PUBLIC void fdrecord(CSOUND *, FDCH *);

PUBLIC void csound_fd_close(CSOUND *, FDCH *);

PUBLIC int csoundAuxAllocAsync(CSOUND *, size_t, AUXCH *, AUXASYNC *, aux_cb,
                               void *);

#ifdef __cplusplus
}
#endif /* __cplusplus */