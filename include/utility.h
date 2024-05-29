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

/**
 * Sorts score file 'inFile' and writes the result to 'outFile'.
 * The Csound instance should be initialised
 * before calling this function, and csoundReset() should be called
 * after sorting the score to clean up. On success, zero is returned.
 */
PUBLIC int csoundScoreSort(CSOUND *, FILE *inFile, FILE *outFile);

/**
 * Extracts from 'inFile', controlled by 'extractFile', and writes
 * the result to 'outFile'. The Csound instance should be initialised
 * before calling this function, and csoundReset()
 * should be called after score extraction to clean up.
 * The return value is zero on success.
 */
PUBLIC int csoundScoreExtract(CSOUND *, FILE *inFile, FILE *outFile,
                              FILE *extractFile);

/**
 * Returns a NULL terminated list of registered utility names.
 * The caller is responsible for freeing the returned array with
 * csoundDeleteUtilityList(), however, the names should not be
 * changed or freed.
 * The return value may be NULL in case of an error.
 */
PUBLIC char **csoundListUtilities(CSOUND *);

/**
 * Releases an utility list previously returned by csoundListUtilities().
 */
PUBLIC void csoundDeleteUtilityList(CSOUND *, char **lst);

/**
 * Get utility description.
 * Returns NULL if the utility was not found, or it has no description,
 * or an error occured.
 */
PUBLIC const char *csoundGetUtilityDescription(CSOUND *, const char *utilName);

/**
 * Run utility with the specified name and command line arguments.
 * Should be called after loading utility plugins.
 * Use csoundReset() to clean up after calling this function.
 * Returns zero if the utility was run successfully.
 */
PUBLIC int csoundRunUtility(CSOUND *, const char *name, int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */
