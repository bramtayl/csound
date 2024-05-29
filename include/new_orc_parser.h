#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse the given orchestra from an ASCII string into a TREE.
 * This can be called during performance to parse new code.
 */
PUBLIC TREE *csoundParseOrc(CSOUND *csound, const char *str);

#ifdef __cplusplus
}
#endif