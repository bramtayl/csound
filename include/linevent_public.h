#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

// formerly csound->InputMessage(
PUBLIC void csoundInputMessageInternal(CSOUND *csound, const char *message);

#ifdef __cplusplus
}
#endif /* __cplusplus */