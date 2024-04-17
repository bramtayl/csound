#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

/**
 * Register utility with the specified name.
 * Returns zero on success.
 */
PUBLIC int csoundAddUtility(CSOUND *, const char *name,
                            int (*UtilFunc)(CSOUND *, int, char **));

/**
 * Set description text for the specified utility.
 * Returns zero on success.
 */
PUBLIC int csoundSetUtilityDescription(CSOUND *, const char *utilName,
                                       const char *utilDesc);

#ifdef __cplusplus
}
#endif /* __cplusplus */
