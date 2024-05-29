#pragma once

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

#include "csound.h"

PUBLIC char *cs_strdup(CSOUND *, char *);

/**
 * Free the resources associated with the TREE *tree
 * This function should be called whenever the TREE was
 * created with csoundParseOrc and memory can be deallocated.
 **/
PUBLIC void csoundDeleteTree(CSOUND *csound, TREE *tree);

#ifdef __cplusplus
}
#endif /* __cplusplus */