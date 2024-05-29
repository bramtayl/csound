#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

PUBLIC int32 strarg2insno(CSOUND *, void *, int);

/* convert opcode string argument to instrument number */
/* return value is -1 if the instrument cannot be found */
/* (in such cases, csoundInitError() is also called) */

/* create file name from opcode argument (string or MYFLT)      */
/*   CSOUND *csound:                                            */
/*      pointer to Csound instance                              */
/*   char *s:                                                   */
/*      output buffer, should have enough space; if NULL, the   */
/*      required amount of memory is allocated and returned     */
/*   void *p:                                                   */
/*      opcode argument, is interpreted as char* or MYFLT*,     */
/*      depending on the 'is_string' parameter                  */
/*   const char *baseName:                                      */
/*      name prefix to be used if the 'p' argument is MYFLT,    */
/*      and it is neither SSTRCOD, nor a valid index to strset  */
/*      space.                                                  */
/*      For example, if "soundin." is passed as baseName, file  */
/*      names in the format "soundin.%d" will be generated.     */
/*      baseName may be an empty string, but should not be NULL */
/*   int is_string:                                             */
/*      if non-zero, 'p' is interpreted as a char* pointer and  */
/*      is used as the file name. Otherwise, it is expected to  */
/*      point to a MYFLT value, and the following are tried:    */
/*        1. if the value is SSTRCOD, the string argument of    */
/*           the current score event is used (string p-field)   */
/*        2. if the value, rounded to the nearest integer, is a */
/*           valid index to strset space, the strset string is  */
/*           used                                               */
/*        3. the file name is generated using baseName and the  */
/*           value rounded to the nearest integer, as described */
/*           above                                              */
/*   return value:                                              */
/*      pointer to the output string; if 's' is not NULL, it is */
/*      always the same as 's', otherwise it is allocated with  */
/*      mmalloc() and the caller is responsible for freeing the */
/*      allocated memory with mfree() or mfree()         */

PUBLIC char *strarg2name(CSOUND *, char *, void *, const char *, int);

/**
 * Allocate nbytes bytes of memory that can be accessed later by calling
 * csoundQueryGlobalVariable() with the specified name; the space is
 * cleared to zero.
 * Returns CSOUND_SUCCESS on success, CSOUND_ERROR in case of invalid
 * parameters (zero nbytes, invalid or already used name), or
 * CSOUND_MEMORY if there is not enough memory.
 */
PUBLIC int csoundCreateGlobalVariable(CSOUND *, const char *name,
                                      size_t nbytes);

/**
 * Get pointer to space allocated with the name "name".
 * Returns NULL if the specified name is not defined.
 */
PUBLIC void *csoundQueryGlobalVariable(CSOUND *, const char *name);

/**
 * This function is the same as csoundQueryGlobalVariable(), except the
 * variable is assumed to exist and no error checking is done.
 * Faster, but may crash or return an invalid pointer if 'name' is
 * not defined.
 */
PUBLIC void *csoundQueryGlobalVariableNoCheck(CSOUND *, const char *name);

/**
 * Free memory allocated for "name" and remove "name" from the database.
 * Return value is CSOUND_SUCCESS on success, or CSOUND_ERROR if the name is
 * not defined.
 */
PUBLIC int csoundDestroyGlobalVariable(CSOUND *, const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */