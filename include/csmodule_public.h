#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Platform-independent function to load a shared library.
 */
PUBLIC int csoundOpenLibrary(void **library, const char *libraryPath);

/**
 * Platform-independent function to unload a shared library.
 */
PUBLIC int csoundCloseLibrary(void *library);

/**
 * Platform-independent function to get a symbol address in a shared library.
 */
PUBLIC void *csoundGetLibrarySymbol(void *library, const char *symbolName);

#ifdef __cplusplus
}
#endif