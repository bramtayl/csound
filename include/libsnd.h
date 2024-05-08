#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "csound.h"

/**
 * Returns the address of the Csound audio input buffer.
 * Enables external software to write audio into Csound before calling
 * csoundPerformBuffer.
 */
PUBLIC MYFLT *csoundGetInputBuffer(CSOUND *);

/**
 * Returns the address of the Csound audio output buffer.
 * Enables external software to read audio from Csound after calling
 * csoundPerformBuffer.
 */
PUBLIC MYFLT *csoundGetOutputBuffer(CSOUND *);

#ifdef __cplusplus
}
#endif