/*
    envvar.h:

    Copyright (C) 2005 Istvan Varga

    This file is part of Csound.

    The Csound Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#ifndef CSOUND_ENVVAR_H
#define CSOUND_ENVVAR_H

#include <stdio.h>

#include "csound.h"
#include "soundfile.h"
#include "sysdep.h"

#include <stdio.h>

#include "csound.h"
#include "soundfile.h"
#include "sysdep.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MSVC)
#define RD_OPTS  _O_RDONLY | _O_BINARY
#define WR_OPTS  _O_TRUNC | _O_CREAT | _O_WRONLY | _O_BINARY,_S_IWRITE
#elif defined(_WIN32)
#define RD_OPTS  O_RDONLY | O_BINARY
#define WR_OPTS  O_TRUNC | O_CREAT | O_WRONLY | O_BINARY, 0644
#elif defined DOSGCC
#define RD_OPTS  O_RDONLY | O_BINARY, 0
#define WR_OPTS  O_TRUNC | O_CREAT | O_WRONLY | O_BINARY, 0644
#else
#ifndef O_BINARY
# define O_BINARY (0)
#endif
#define RD_OPTS  O_RDONLY | O_BINARY, 0
#define WR_OPTS  O_TRUNC | O_CREAT | O_WRONLY | O_BINARY, 0644
#endif

/* Space for 16 global environment variables, */
/* 32 bytes for name and 480 bytes for value. */
/* Only written by csoundSetGlobalEnv().      */

static char globalEnvVars[8192] = {(char)0};

#define globalEnvVarName(x) ((char *)&(globalEnvVars[(int)(x) << 9]))
#define globalEnvVarValue(x) ((char *)&(globalEnvVars[((int)(x) << 9) + 32]))

typedef struct CSFILE_ {
    struct CSFILE_  *nxt;
    struct CSFILE_  *prv;
    int             type;
    int             fd;
    FILE            *f;
    SNDFILE         *sf;
    void            *cb;
    int             async_flag;
    int             items;
    int             pos;
    MYFLT           *buf;
    int             bufsize;
    char            fullName[1];
} CSFILE;

void close_all_files(CSOUND *csound);
void *fopen_path(CSOUND *csound, FILE **fp, char *name, char *basename,
                 char *env, int fromScore);

  /**
   * Set environment variable 'name' to 'value'.
   * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or CSOUND_MEMORY
   * if the environment variable could not be set for some reason.
   */
  int csoundSetEnv(CSOUND *csound, const char *name, const char *value);

  /**
   * Append 'value' to environment variable 'name', using ';' as
   * separator character.
   * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or CSOUND_MEMORY
   * if the environment variable could not be set for some reason.
   */
  int csoundAppendEnv(CSOUND *csound, const char *name, const char *value);

  /**
   * Prepend 'value' to environment variable 'name', using ';' as
   * separator character.
   * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or CSOUND_MEMORY
   * if the environment variable could not be set for some reason.
   */
  int csoundPrependEnv(CSOUND *csound, const char *name, const char *value);

  /**
   * Initialise environment variable database, and copy system
   * environment variables.
   * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or
   * CSOUND_MEMORY in case of an error.
   */
  int csoundInitEnv(CSOUND *csound);

  /**
   * Parse 's' as an assignment to environment variable, in the format
   * "NAME=VALUE" for replacing the previous value, or "NAME+=VALUE"
   * for appending.
   * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or
   * CSOUND_MEMORY in case of an error.
   */
  int csoundParseEnv(CSOUND *csound, const char *s);

  /** Check if file name is valid, and copy with converting pathname delimiters */
  char *csoundConvertPathname(CSOUND *csound, const char *filename);

  /**  Check if name is a full pathname for the platform we are running on. */
  int csoundIsNameFullpath(const char *name);

  /** Check if name is a relative pathname for this platform.  Bare
   *  filenames with no path information are not counted.
   */
  int csoundIsNameRelativePath(const char *name);

  /** Check if name is a "leaf" (bare) filename for this platform. */
  int csoundIsNameJustFilename(const char *name);

  /** Properly concatenates the full or relative pathname in path1 with
   *  the relative pathname or filename in path2 according to the rules
   *  for the platform we are running on.  path1 is assumed to be
   *  a directory whether it ends with DIRSEP or not.  Relative paths must
   *  conform to the conventions for the current platform (begin with ':'
   *  on MacOS 9 and not begin with DIRSEP on others).
   */
  char* csoundConcatenatePaths(CSOUND* csound, const char *path1,
                                               const char *path2);

  /** Converts a pathname to native format and returns just the part of
   *  the path that specifies the directory.  Does not return the final
   *  DIRSEP.  Returns an empty string if no path components occur before
   *  the filename.  Returns NULL if unable to carry out the operation
   *  for some reason.
   */
  char *csoundSplitDirectoryFromPath(CSOUND* csound, const char * path);

  /** Return just the final component of a full path */
  char *csoundSplitFilenameFromPath(CSOUND* csound, const char * path);

  /** Given a file name as string, return full path of directory of file;
   * Note: does not check if file exists
   */
  char *csoundGetDirectoryForPath(CSOUND* csound, const char * path);

  int csoundFindFile_Fd(CSOUND *csound, char **fullName,
                             const char *filename, int write_mode,
                             const char *envList);

  FILE *csoundFindFile_Std(CSOUND *csound, char **fullName,
                                const char *filename, const char *mode,
                                const char *envList);

  uintptr_t file_iothread(void *p);

char **csoundGetSearchPathFromEnv(CSOUND *csound, const char *envList);

#ifdef __cplusplus
}
#endif

#endif  /* CSOUND_ENVVAR_H */
