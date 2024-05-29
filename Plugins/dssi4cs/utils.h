/* utils.h

   Copyright 2005 Richard W.E. Furse
   with Csound adjustments by Andres Cabrera, John ffitch

   Free software by Richard W.E. Furse. Do with as you will. No
   warranty.
   Modified for csound by Andres Cabrera*/

#ifndef LADSPA_SDK_LOAD_PLUGIN_LIB
#define LADSPA_SDK_LOAD_PLUGIN_LIB

/*****************************************************************************/

#include "csdl.h"
#include "ladspa.h"

/*****************************************************************************/

/* Functions in load.c: */

/* This function call takes a plugin library filename, searches for
   the library along the LADSPA_PATH, loads it with dlopen() and
   returns a plugin handle for use with findPluginDescriptor() or
   unloadLADSPAPluginLibrary(). Errors are handled by writing a
   message to stderr and calling exit(1). It is alright (although
   inefficient) to call this more than once for the same file. */

void * dlopenLADSPA(CSOUND * csound, const char * pcFilename, int32_t iFlag);

void * loadLADSPAPluginLibrary(CSOUND * csound, const char * pcPluginFilename);

/* This function unloads a LADSPA plugin library. */
void unloadLADSPAPluginLibrary(CSOUND * csound, void * pvLADSPAPluginLibrary);

/* This function locates a LADSPA plugin within a plugin library
   loaded with loadLADSPAPluginLibrary(). Errors are handled by
   writing a message to stderr and calling exit(1). Note that the
   plugin library filename is only included to help provide
   informative error messages. */
const LADSPA_Descriptor *
findLADSPAPluginDescriptor(CSOUND * csound,
                           void * pvLADSPAPluginLibrary,
                           const char * pcPluginLibraryFilename,
                           const char * pcPluginLabel);

#endif

