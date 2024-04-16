#pragma once

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

#include "csound.h"

/* Fast power of two function from a precomputed table */
PUBLIC MYFLT csoundPow2(CSOUND *csound, MYFLT a);

#ifdef __cplusplus
}
#endif /* __cplusplus */