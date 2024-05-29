#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

/**
 * Search for input file 'filename'.
 * If the file name specifies full path (it begins with '.', the pathname
 * delimiter character, or a drive letter and ':' on Windows), that exact
 * file name is tried without searching.
 * Otherwise, the file is searched relative to the current directory first,
 * and if it is still not found, a pathname list that is created the
 * following way is searched:
 *   1. if envList is NULL or empty, no directories are searched
 *   2. envList is parsed as a ';' separated list of environment variable
 *      names, and all environment variables are expanded and expected to
 *      contain a ';' separated list of directory names
 *   2. all directories in the resulting pathname list are searched, starting
 *      from the last and towards the first one, and the directory where the
 *      file is found first will be used
 * The function returns a pointer to the full name of the file if it is
 * found, and NULL if the file could not be found in any of the search paths,
 * or an error has occured. The caller is responsible for freeing the memory
 * pointed to by the return value, by calling mfree().
 */
PUBLIC char *csoundFindInputFile(CSOUND *csound, const char *filename,
                                 const char *envList);

/**
 * Search for a location to write file 'filename'.
 * If the file name specifies full path (it begins with '.', the pathname
 * delimiter character, or a drive letter and ':' on Windows), that exact
 * file name is tried without searching.
 * Otherwise, a pathname list that is created the following way is searched:
 *   1. if envList is NULL or empty, no directories are searched
 *   2. envList is parsed as a ';' separated list of environment variable
 *      names, and all environment variables are expanded and expected to
 *      contain a ';' separated list of directory names
 *   2. all directories in the resulting pathname list are searched, starting
 *      from the last and towards the first one, and the directory that is
 *      found first where the file can be written to will be used
 * Finally, if the file cannot be written to any of the directories in the
 * search paths, writing relative to the current directory is tried.
 * The function returns a pointer to the full name of the file if a location
 * suitable for writing the file is found, and NULL if the file cannot not be
 * written anywhere in the search paths, or an error has occured.
 * The caller is responsible for freeing the memory pointed to by the return
 * value, by calling mfree().
 */
PUBLIC char *csoundFindOutputFile(CSOUND *csound, const char *filename,
                                  const char *envList);

/**
 * Allocate a file handle for an existing file already opened with open(),
 * fopen(), or sf_open(), for later use with csoundFileClose() or
 * csoundGetFileName(), or storing in an FDCH structure.
 * Files registered this way are also
 * automatically closed by csoundReset().
 * Parameters and return value are similar to csoundFileOpenithType(), except
 * fullName is the name that will be returned by a later call to
 * csoundGetFileName().
 */
PUBLIC void *csoundCreateFileHandle(CSOUND *, void *fd, int type,
                                    const char *fullName);

/**
 * Get the full name of a file previously opened with csoundFileOpen().
 */
PUBLIC char *csoundGetFileName(void *fd);

/**
 * Close a file previously opened with csoundFileOpen().
 */
PUBLIC int csoundFileClose(CSOUND *, void *fd);

/**
 * Open a file and return handle.
 *
 * CSOUND *csound:
 *   Csound instance pointer
 * void *fd:
 *   pointer a variable of type int, FILE*, or SNDFILE*, depending on 'type',
 *   for storing handle to be passed to file read/write functions
 * int type:
 *   file type, one of the following:
 *     CSFILE_FD_R:     read file using low level interface (open())
 *     CSFILE_FD_W:     write file using low level interface (open())
 *     CSFILE_STD:      use ANSI C interface (fopen())
 *     CSFILE_SND_R:    read sound file
 *     CSFILE_SND_W:    write sound file
 * const char *name:
 *   file name
 * void *param:
 *   parameters, depending on type:
 *     CSFILE_FD_R:     unused (should be NULL)
 *     CSFILE_FD_W:     unused (should be NULL)
 *     CSFILE_STD:      mode parameter (of type char*) to be passed to fopen()
 *     CSFILE_SND_R:    SF_INFO* parameter for sf_open(), with defaults for
 *                      raw file; the actual format paramaters of the opened
 *                      file will be stored in this structure
 *     CSFILE_SND_W:    SF_INFO* parameter for sf_open(), output file format
 * const char *env:
 *   list of environment variables for search path (see csoundFindInputFile()
 *   for details); if NULL, the specified name is used as it is, without any
 *   conversion or search.
 * int csFileType:
 *   A value from the enumeration CSOUND_FILETYPES (see CsoundCore.h)
 * int isTemporary:
 *   1 if this file will be deleted when Csound is finished.
 *   Otherwise, 0.
 * return value:
 *   opaque handle to the opened file, for use with csoundGetFileName() or
 *   csoundFileClose(), or storing in FDCH.fd.
 *   On failure, NULL is returned.
 */
PUBLIC void *csoundFileOpenWithType(CSOUND *csound, void *fd, int type,
                                    const char *name, void *param,
                                    const char *env, int csFileType,
                                    int isTemporary);

PUBLIC void *csoundFileOpenWithType_Async(CSOUND *csound, void *fd, int type,
                                          const char *name, void *param,
                                          const char *env, int csFileType,
                                          int buffsize, int isTemporary);

PUBLIC unsigned int csoundReadAsync(CSOUND *csound, void *handle, MYFLT *buf,
                                    int items);

PUBLIC unsigned int csoundWriteAsync(CSOUND *csound, void *handle, MYFLT *buf,
                                     int items);

PUBLIC int csoundFSeekAsync(CSOUND *csound, void *handle, int pos, int whence);

/**
 * Get pointer to the value of environment variable 'name', searching
 * in this order: local environment of 'csound' (if not NULL), variables
 * set with csoundSetGlobalEnv(), and system environment variables.
 * If 'csound' is not NULL, should be called after csoundCompile().
 * Return value is NULL if the variable is not set.
 */
PUBLIC const char *csoundGetEnv(CSOUND *csound, const char *name);

/**
 * Set the global value of environment variable 'name' to 'value',
 * or delete variable if 'value' is NULL.
 * It is not safe to call this function while any Csound instances
 * are active.
 * Returns zero on success.
 */
PUBLIC int csoundSetGlobalEnv(const char *name, const char *value);

#ifdef __cplusplus
}
#endif /* __cplusplus */