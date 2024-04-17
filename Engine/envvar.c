/*
    envvar.c:

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

#include "csoundCore_internal.h"
#include "soundio.h"
#include "envvar.h"
#include "envvar_public.h"
#include "envvar_public.h"
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "memalloc.h"
#include "csound_orc_semantics_public.h"
#include "libsnd_u.h"

#if defined(MSVC) || defined(__wasi__)
#include <fcntl.h>
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <direct.h>
#  define getcwd(x,y) _getcwd(x,y)
#endif

#if defined(__wasi__)
#  include <unistd.h>
#  define getcwd(x,y) "/"
#endif

#include "namedins.h"

/* list of environment variables used by Csound */

static const char *envVar_list[] = {
    "CSNOSTOP",
    "CSOUND7RC",
    "CSSTRNGS",
    "CS_LANG",
    "HOME",
    "INCDIR",
    "OPCODE7DIR",
    "OPCODE7DIR64",
    "RAWWAVE_PATH",
    "SADIR",
    "SFDIR",
    "SFOUTYP",
    "SNAPDIR",
    "SSDIR",
    "MFDIR",
    NULL
};

typedef struct searchPathCacheEntry_s {
    char    *name;
    struct searchPathCacheEntry_s   *nxt;
    char    *lst[1];
} searchPathCacheEntry_t;

typedef struct nameChain_s {
    struct nameChain_s  *nxt;
    char    s[1];
} nameChain_t;

/* Space for 16 global environment variables, */
/* 32 bytes for name and 480 bytes for value. */
/* Only written by csoundSetGlobalEnv().      */

static char globalEnvVars[8192] = { (char) 0 };

#define globalEnvVarName(x)   ((char*) &(globalEnvVars[(int) (x) << 9]))
#define globalEnvVarValue(x)  ((char*) &(globalEnvVars[((int) (x) << 9) + 32]))

static int is_valid_envvar_name(const char *name)
{
    char *s;

    if (UNLIKELY(name == NULL || name[0] == '\0'))
      return 0;
    s = (char*) &(name[0]);
    if (UNLIKELY(!(isalpha(*s) || *s == '_')))
      return 0;
    while (*(++s) != '\0') {
      if (UNLIKELY(!(isalpha(*s) || isdigit(*s) || *s == '_')))
        return 0;
    }
    return 1;
}

/**
 * Get pointer to value of environment variable 'name'.
 * Return value is NULL if the variable is not set.
 */

PUBLIC const char *csoundGetEnv(CSOUND *csound, const char *name)
{
    if (csound == NULL) {
      int i;
      if (name == NULL || name[0] == '\0')
        return (const char*) NULL;
      for (i = 0; i < 16; i++) {
        if (strcmp(globalEnvVarName(i), name) == 0)
          return (const char*) globalEnvVarValue(i);
      }
      return (const char*) getenv(name);
    }

    if (csound->envVarDB == NULL) return NULL;

    return (const char*) cs_hash_table_get(csound, csound->envVarDB, (char*)name);
}

/**
 * Set the global value of environment variable 'name' to 'value',
 * or delete variable if 'value' is NULL.
 * It is not safe to call this function while any Csound instances
 * are active.
 * Returns zero on success.
 */

PUBLIC int csoundSetGlobalEnv(const char *name, const char *value)
{
    int   i;

    if (UNLIKELY(name == NULL || name[0] == '\0' || (int) strlen(name) >= 32))
      return -1;                        /* invalid name             */
    for (i = 0; i < 16; i++) {
      if ((value != NULL && globalEnvVarName(i)[0] == '\0') ||
          strcmp(name, globalEnvVarName(i)) == 0)
        break;
    }
    if (UNLIKELY(i >= 16))              /* not found / no free slot */
      return -1;
    if (value == NULL) {
      globalEnvVarName(i)[0] = '\0';    /* delete existing variable */
      return 0;
    }
    if (UNLIKELY(strlen(value) >= 480))
      return -1;                        /* string value is too long */
    strcpy(globalEnvVarName(i), name);
    strcpy(globalEnvVarValue(i), value);
    return 0;
}

/**
 * Set environment variable 'name' to 'value'.
 * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or CSOUND_MEMORY
 * if the environment variable could not be set for some reason.
 */

int csoundSetEnv(CSOUND *csound, const char *name, const char *value)
{
    searchPathCacheEntry_t  *ep, *nxt;
    char                    *oldValue;

    /* check for valid parameters */
    if (UNLIKELY(csound == NULL || !is_valid_envvar_name(name)))
      return CSOUND_ERROR;

    /* invalidate search path cache */
    ep = (searchPathCacheEntry_t*) csound->searchPathCache;
    while (ep != NULL) {
      nxt = ep->nxt;
      mfree(csound, ep);
      ep = nxt;
    }
    csound->searchPathCache = NULL;


    oldValue = cs_hash_table_get(csound, csound->envVarDB, (char*)name);
    if (oldValue != NULL) {
      mfree(csound, oldValue);
    }

    cs_hash_table_put(csound, csound->envVarDB,
                      (char*)name, cs_strdup(csound, (char*)value));

    /* print debugging info if requested */
    if (UNLIKELY(csound->oparms->odebug)) {
      csoundMessage(csound, Str("Environment variable '%s' has been set to "),
                              name);
      if (value == NULL)
        csoundMessage(csound, "NULL\n");
      else
        csoundMessage(csound, "'%s'\n", value);
    }
    /* report success */
    return CSOUND_SUCCESS;
}

/**
 * Append 'value' to environment variable 'name', using ENVSEP as
 * separator character.
 * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or CSOUND_MEMORY
 * if the environment variable could not be set for some reason.
 */

int csoundAppendEnv(CSOUND *csound, const char *name, const char *value)
{
    const char  *oldval;
    char        *newval;
    int         retval;

    /* check for valid parameters */
    if (UNLIKELY(csound == NULL || !is_valid_envvar_name(name)))
      return CSOUND_ERROR;
    /* get original value of variable */
    oldval = csoundGetEnv(csound, name);
    if (oldval == NULL)
      return csoundSetEnv(csound, name, value);
    if (value == NULL || value[0] == '\0')
      return CSOUND_SUCCESS;
    /* allocate new value (+ 2 bytes for ENVSEP and null character) */
    newval = (char*) mmalloc(csound, (size_t) strlen(oldval)
                             + (size_t) strlen(value) + (size_t) 2);
    /* append to old value */
    strcpy(newval, oldval);     /* These are safe as space calculated above */
    //    printf("%d: newval = %s\n", __LINE__, newval);
    // should be a better way
    newval[strlen(oldval)]= ENVSEP;
    newval[strlen(oldval)+1]= '\0';
    //    printf("%d: newval = %s\n", __LINE__, newval);
    strcat(newval, value);
    //    printf("%d: newval = %s\n", __LINE__, newval);
    /* set variable */
    retval = csoundSetEnv(csound, name, newval);
    mfree(csound, newval);
    /* return with error code */
    return retval;
}

/**
 * Prepend 'value' to environment variable 'name', using ENVSEP as
 * separator character.
 * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or CSOUND_MEMORY
 * if the environment variable could not be set for some reason.
 */

int csoundPrependEnv(CSOUND *csound, const char *name, const char *value)
{
    const char  *oldval;
    char        *newval;
    int         retval;

    /* check for valid parameters */
    if (UNLIKELY(csound == NULL || !is_valid_envvar_name(name)))
      return CSOUND_ERROR;
    /* get original value of variable */
    oldval = csoundGetEnv(csound, name);
    if (oldval == NULL)
      return csoundSetEnv(csound, name, value);
    if (value == NULL || value[0] == '\0')
      return CSOUND_SUCCESS;
    /* allocate new value (+ 2 bytes for ';' and null character) */
    newval = (char*) mmalloc(csound, (size_t) strlen(oldval)
                                     + (size_t) strlen(value) + (size_t) 2);
    /* prepend to old value */
    strcpy(newval, value);
    //    printf("%d: newval = %s\n", __LINE__,  newval);
    newval[strlen(value)]= ENVSEP;
    newval[strlen(value)+1]= '\0';
    //    printf("%d: newval = %s\n", __LINE__,  newval);
    strcat(newval, oldval);
    //    printf("%d: newval = %s\n", __LINE__,  newval);
    /* set variable */
    retval = csoundSetEnv(csound, name, newval);
    mfree(csound, newval);
    /* return with error code */
    return retval;
}

/**
 * Initialise environment variable database, and copy system
 * environment variables.
 * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or
 * CSOUND_MEMORY in case of an error.
 */

int csoundInitEnv(CSOUND *csound)
{
    int i, retval;
    /* check if already initialised */
    if (csound->envVarDB != NULL)
      return CSOUND_SUCCESS;
    /* allocate table */
    csound->envVarDB = cs_hash_table_create(csound);
    /* copy standard Csound environment variables */
    for (i = 0; envVar_list[i] != NULL; i++) {
      const char  *name = envVar_list[i];
      const char  *value = getenv(name);
      if (value != NULL) {
        retval = csoundSetEnv(csound, name, value);
        if (retval != CSOUND_SUCCESS)
          return retval;
      }
    }
    /* copy any global defaults set with csoundSetGlobalEnv() */
    for (i = 0; i < 16; i++) {
      if (globalEnvVarName(i)[0] != '\0') {
        retval = csoundSetEnv(csound, globalEnvVarName(i),
                                      globalEnvVarValue(i));
        if (retval != CSOUND_SUCCESS)
          return retval;
      }
    }
    /* done */
    return CSOUND_SUCCESS;
}

/**
 * Parse 's' as an assignment to environment variable, in the format
 * "NAME=VALUE" for replacing the previous value, or "NAME+=VALUE"
 * for appending.
 * Returns CSOUND_SUCCESS on success, and CSOUND_ERROR or
 * CSOUND_MEMORY in case of an error.
 */

int csoundParseEnv(CSOUND *csound, const char *s)
{
    char  *name, *value, msg[256];
    int   append_mode, retval;

    /* copy string constant */
    name = (char*) mmalloc(csound, (size_t) strlen(s) + (size_t) 1);
    strcpy(name, s);
    /* check assignment format */
    value = strchr(name, '=');
    append_mode = 0;
    if (UNLIKELY(value == NULL || value == name)) {
      strNcpy(msg, Str(" *** invalid format for --env\n"), 256);
      retval = CSOUND_ERROR;
      goto err_return;
    }
    *(value++) = '\0';
    if (*(value - 2) == '+') {
      append_mode = 1;
      *(value - 2) = '\0';
    }
    if (UNLIKELY(!is_valid_envvar_name(name))) {
      strNcpy(msg, Str(" *** invalid environment variable name\n"), 256);
      retval = CSOUND_ERROR;
      goto err_return;
    }
    /* set variable */
    if (!append_mode)
      retval = csoundSetEnv(csound, name, value);
    else
      retval = csoundAppendEnv(csound, name, value);
    if (UNLIKELY(retval == CSOUND_MEMORY))
      strNcpy(msg, Str(" *** memory allocation failure\n"), 256);
    else
      strNcpy(msg, Str(" *** error setting environment variable\n"), 256);

 err_return:
    if (UNLIKELY(retval != CSOUND_SUCCESS))
      csoundMessage(csound, "%s", msg);
    mfree(csound, name);
    return retval;
}

char **csoundGetSearchPathFromEnv(CSOUND *csound, const char *envList)
{
    searchPathCacheEntry_t  *p;
    nameChain_t             *env_lst = NULL, *path_lst = NULL, *tmp, *prv, *nxt;
    char                    *s;
    int                     i, j, k, len, pathCnt = 0, totLen = 0;

    /* check if the specified environment variable list was already parsed */
    p = (searchPathCacheEntry_t*) csound->searchPathCache;
    while (p != NULL) {
      if (sCmp(p->name, envList) == 0)
        return (&(p->lst[0]));
      p = p->nxt;
    }
    /* not found, need to create new entry */
    len = (int) strlen(envList);
    /* split environment variable list to tokens */
    for (i = j = 0; i <= len; i++) {
      if (envList[i] == ';' || envList[i] == ':' || envList[i] == '\0') {
        if (i > j) {
          tmp = (nameChain_t*)mmalloc(csound, sizeof(nameChain_t) + (i-j));
          for (k = 0; j < i; j++, k++)
            tmp->s[k] = envList[j];
          tmp->s[k] = '\0';
          tmp->nxt = NULL;
          if (env_lst != NULL) {
            /* search for duplicate entry */
            prv = nxt = env_lst;
            do {
              if (sCmp(env_lst->s, tmp->s) == 0)
                break;
              prv = nxt;
            } while ((nxt = prv->nxt) != NULL);
            if (nxt == NULL)
              prv->nxt = tmp;
            else
              mfree(csound, tmp);       /* and remove if there is any */
          }
          else
            env_lst = tmp;
        }
        j = i + 1;
      }
    }
    /* expand environment variables to path list */
    while (env_lst != NULL) {
      nxt = env_lst->nxt;
      s = (char*) csoundGetEnv(csound, env_lst->s);
      mfree(csound, env_lst);
      env_lst = nxt;
      if (s != NULL && s[0] != '\0')
        len = (int) strlen(s);
      else
        len = -1;
      // **** THIS CODE DOES NOT CHECK FOR WINDOWS STYLE C:\foo ****
      for (i = j = 0; i <= len; i++) {
        if (i==0 && isalpha(s[i]) && s[i+1]==':') i++;
        else if (s[i] == ';' || s[i] == ':' || s[i] == '\0') {
          if (i > j) {
            tmp = (nameChain_t*) mmalloc(csound, sizeof(nameChain_t)
                                                 + (i - j) + 1);
            /* copy with converting pathname delimiters */
            /* FIXME: should call csoundConvertPathname instead */
            for (k = 0; j < i; j++, k++)
              tmp->s[k] = (s[j] == '/' || s[j] == '\\' ? DIRSEP : s[j]);
            while (tmp->s[--k] == DIRSEP);
            tmp->s[++k] = DIRSEP;
            tmp->s[++k] = '\0';
            tmp->nxt = path_lst;
            path_lst = tmp;
            /* search for duplicate entry */
            for (prv = tmp; (tmp = tmp->nxt) != NULL; prv = tmp)
              if (sCmp(path_lst->s, tmp->s) == 0)
                break;
            if (tmp != NULL) {
              /* and remove if there is any */
              prv->nxt = tmp->nxt;
              mfree(csound, tmp);
            }
            else {
              /* calculate storage requirement */
              pathCnt++; totLen += (k + 1);
            }
          }
          j = i + 1;
          if (i+2<=len && s[i+2]==':' && isalpha(s[i+1])) i+=2;
        }
      }
    }
    totLen += ((int) strlen(envList) + 1);
    /* create path cache entry */
    p = (searchPathCacheEntry_t*) mmalloc(csound,
                                                 sizeof(searchPathCacheEntry_t)
                                                 + sizeof(char*) * pathCnt
                                                 + sizeof(char) * totLen);
    s = (char*) &(p->lst[pathCnt + 1]);
    p->name = s;
    strcpy(p->name, envList);
    s += ((int) strlen(envList) + 1);
    p->nxt = (searchPathCacheEntry_t*) csound->searchPathCache;
    if (UNLIKELY(csound->oparms->odebug))
      csoundDebugMsg(csound, Str("Creating search path cache for '%s':"),
                               p->name);
    for (i = 0; (i < pathCnt) && (path_lst != NULL); i++) {
      p->lst[i] = s;
      strcpy(s, path_lst->s);
      s += ((int) strlen(path_lst->s) + 1);
      nxt = path_lst->nxt;
      mfree(csound, path_lst);
      path_lst = nxt;
      if (UNLIKELY(csound->oparms->odebug))
        csoundDebugMsg(csound, "%5d: \"%s\"", (i + 1), p->lst[i]);
    }
    p->lst[i] = NULL;
    /* link into database */
    csound->searchPathCache = (void*) p;
    /* return with pathname list */
    return (&(p->lst[0]));
}

/** Check if file name is valid, and copy with converting pathname delimiters */
char *csoundConvertPathname(CSOUND *csound, const char *filename)
{
    char  *name;
    int   i = 0;

/* FIXMEs:  need to convert ':' from Mac pathnames (but be careful of not
   messing up Windows drive names!); need to be careful of
   relative paths containing "./", "../", or multiple colons "::"; need to
   collapse multiple slashes "//" or "\\\\" ??  */
    if (filename == NULL || filename[0] == '\0')
      return NULL;
    name = (char*) mmalloc(csound, (size_t) strlen(filename) + (size_t) 1);
    do {
      if (filename[i] != '/' && filename[i] != '\\')
        name[i] = filename[i];
      else
        name[i] = DIRSEP;
    } while (filename[i++] != '\0');
    if (name[i - 2] == DIRSEP
#ifdef _WIN32
        || (isalpha(name[0]) && name[1] == ':' && name[2] == '\0')
#endif
        ) {
      mfree(csound, name);
      return NULL;
    }
    return name;
}

/**  Check if name is a full pathname for the platform we are running on. */
int csoundIsNameFullpath(const char *name)
{
#ifdef _WIN32
    if (isalpha(name[0]) && name[1] == ':') return 1;
#endif
    if (name[0] == DIRSEP) /* ||
        (name[0] == '.' && (name[1] == DIRSEP ||
                            (name[1] == '.' && name[2] == DIRSEP)))) */
      return 1;
    return 0;
}

/** Check if name is a relative pathname for this platform.  Bare
 *  filenames with no path information are not counted.
 */
int csoundIsNameRelativePath(const char *name)
{
    if (name[0] != DIRSEP && strchr(name, DIRSEP) != NULL)
      return 1;
    return 0;
}

/** Check if name is a "leaf" (bare) filename for this platform. */
int csoundIsNameJustFilename(const char *name)
{
    if (strchr(name, DIRSEP) != NULL) return 0;
#ifdef _WIN32
    if (name[2] == ':') return 0;
#endif
    return 1;
}

/** Properly concatenates the full or relative pathname in path1 with
 *  the relative pathname or filename in path2 according to the rules
 *  for the platform we are running on.  path1 is assumed to be
 *  a directory whether it ends with DIRSEP or not.  Relative paths must
 *  conform to the conventions for the current platform (begin with ':'
 *  on MacOS 9 and not begin with DIRSEP on others).
 */
char* csoundConcatenatePaths(CSOUND* csound, const char *path1,
                             const char *path2)
{
    char *result;
    const char *start2;
    char separator[2];
    int  len1 = strlen(path1);
    int  len2 = strlen(path2);

    /* cannot join two full pathnames -- so just return path2 ? */
    if (csoundIsNameFullpath(path2)) {
        result = (char*) mmalloc(csound, (size_t)len2+1);
        strcpy(result, path2);
        return result;
    }

    start2 = path2;
    /* ignore "./" at the beginning */
    if (path2[0] == '.' && path2[1] == DIRSEP) start2 = path2 + 2;

    result = (char*) mmalloc(csound, (size_t)len1+(size_t)len2+2);
    strcpy(result, path1);
    /* check for final DIRSEP in path1 */
    if (path1[len1-1] != DIRSEP) {
        separator[0] = DIRSEP; separator[1] = '\0';
        strcat(result, separator);
    }
    strcat(result, start2);

    return result;
}

/** Converts a pathname to native format and returns just the part of
 *  the path that specifies the directory.  Does not return the final
 *  DIRSEP.  Returns an empty string if no path components occur before
 *  the filename.  Returns NULL if unable to carry out the operation
 *  for some reason.
 */
char *csoundSplitDirectoryFromPath(CSOUND* csound, const char * path)
{
    char *convPath;
    char *lastIndex;
    char *partialPath;
    int  len;

    if ((convPath = csoundConvertPathname(csound, path)) == NULL)
        return NULL;
    lastIndex = strrchr(convPath, DIRSEP);

    if (lastIndex == NULL) {  /* no DIRSEP before filename */
#ifdef _WIN32  /* e.g. C:filename */
        if (isalpha(convPath[0]) && convPath[1] == ':') {
            partialPath = (char*) mmalloc(csound, (size_t) 3);
            partialPath[0] = convPath[0];
            partialPath[1] = convPath[1];
            partialPath[2] = '\0';
            mfree(csound, convPath);
            return partialPath;
        }
#endif
        partialPath = (char*) mmalloc(csound, (size_t) 1);
        partialPath[0] = '\0';
    }
    else {
        len = lastIndex - convPath;
        partialPath = (char*) mmalloc(csound, len+1);
        strNcpy(partialPath, convPath, len+1);
        //partialPath[len] = '\0';
   }
   mfree(csound, convPath);
   return partialPath;
}

/** Return just the final component of a full path */
char *csoundSplitFilenameFromPath(CSOUND* csound, const char * path)
{
    char *convPath;
    char *lastIndex;
    char *filename;
    int  len;

    if ((convPath = csoundConvertPathname(csound, path)) == NULL)
      return NULL;
    lastIndex = strrchr(convPath, DIRSEP);
    len = strlen(lastIndex);
    filename = (char*) mmalloc(csound, len+1);
    strcpy(filename, lastIndex+1);
    mfree(csound, convPath);
    return filename;
}

/* given a file name as string, return full path of directory of file;
 * Note: does not check if file exists
 */
char *csoundGetDirectoryForPath(CSOUND* csound, const char * path) {
#ifdef BARE_METAL
  (void) csound;
  (void) path;
  return NULL;
#else  
    char *partialPath, *tempPath, *lastIndex;
    char *retval;
    char *cwd;
    int  len;

    if (path == NULL) return NULL;

    tempPath = csoundConvertPathname(csound, path);
    if (tempPath == NULL) return NULL;
    lastIndex = strrchr(tempPath, DIRSEP);

    if (tempPath && csoundIsNameFullpath(tempPath)) {
      /* check if root directory */
      if (lastIndex == tempPath) {
        partialPath = (char *)mmalloc(csound, 2);
        partialPath[0] = DIRSEP;
        partialPath[1] = '\0';

        mfree(csound, tempPath);

        return partialPath;
      }

#  ifdef _WIN32
      /* check if root directory of Windows drive */
      if ((lastIndex - tempPath) == 2 && tempPath[1] == ':') {
        partialPath = (char *)mmalloc(csound, 4);
        partialPath[0] = tempPath[0];
        partialPath[1] = tempPath[1];
        partialPath[2] = tempPath[2];
        partialPath[3] = '\0';

        mfree(csound, tempPath);

        return partialPath;
      }
#  endif
      len = (lastIndex - tempPath);

      partialPath = (char *)mcalloc(csound, len + 1);
      strNcpy(partialPath, tempPath, len+1);

      mfree(csound, tempPath);

      return partialPath;
    }

    /* do we need to worry about ~/ on *nix systems ? */
    /* we have a relative path or just a filename */
    len = 32;
    cwd = mmalloc(csound, len);
 again:
    if (UNLIKELY(getcwd(cwd, len)==NULL)) {
      // Should check ERANGE==errno
      //csoundDie(csound, Str("Current directory path name too long\n"));
      len =len+len; cwd = mrealloc(csound, cwd, len);
      if (UNLIKELY(len>1024*1024))
        csoundDie(csound, Str("Current directory path name too long\n"));
      goto again;
    }

    if (lastIndex == NULL) {
      return cwd;
    }

    len = (lastIndex - tempPath);  /* could be 0 on OS 9 */

    partialPath = (char *)mcalloc(csound, len + 1);
    strNcpy(partialPath, tempPath, len+1);

    retval = csoundConcatenatePaths(csound, cwd, partialPath);

    mfree(csound, cwd);
    mfree(csound, partialPath);
    mfree(csound, tempPath);

    return retval;
#endif  
}

FILE *csoundFindFile_Std(CSOUND *csound, char **fullName,
                                const char *filename, const char *mode,
                                const char *envList)
{
    FILE  *f;
    char  *name, *name2, **searchPath;

    *fullName = NULL;
    if ((name = csoundConvertPathname(csound, filename)) == NULL)
      return (FILE*) NULL;
    if (mode[0] != 'w') {
      /* read: try the specified name first */
      f = fopen(name, mode);
      if (f != NULL) {
        *fullName = name;
        return f;
      }
      /* if full path, and not found: */
      if (csoundIsNameFullpath(name)) {
        mfree(csound, name);
        return (FILE*) NULL;
      }
    }
    else if (csoundIsNameFullpath(name)) {
      /* if write and full path: */
      f = fopen(name, mode);
      if (f != NULL)
        *fullName = name;
      else
        mfree(csound, name);
      return f;
    }
    /* search paths defined by environment variable list */
    if (envList != NULL && envList[0] != '\0' &&
        (searchPath = csoundGetSearchPathFromEnv((CSOUND*) csound, envList))
        != NULL) {
      //len = (int) strlen(name) + 1;
      while (*searchPath != NULL) {
        name2 = csoundConcatenatePaths(csound, *searchPath, name);
        f = fopen(name2, mode);
        if (f != NULL) {
          mfree(csound, name);
          *fullName = name2;
          return f;
        }
        mfree(csound, name2);
        searchPath++;
      }
    }
    /* if write mode, try current directory last */
    if (mode[0] == 'w') {
      f = fopen(name, mode);
      if (f != NULL) {
        *fullName = name;
        return f;
      }
    }
    /* not found */
    mfree(csound, name);
    return (FILE*) NULL;
}

int csoundFindFile_Fd(CSOUND *csound, char **fullName,
                             const char *filename, int write_mode,
                             const char *envList)
{
    char  *name, *name2, **searchPath;
    int   fd;

    *fullName = NULL;
    if ((name = csoundConvertPathname(csound, filename)) == NULL)
      return -1;
    if (!write_mode) {
      /* read: try the specified name first */
      fd = open(name, RD_OPTS);
      if (fd >= 0) {
        *fullName = name;
        return fd;
      }
      /* if full path, and not found: */
      if (csoundIsNameFullpath(name)) {
        mfree(csound, name);
        return -1;
      }
    }
    else if (csoundIsNameFullpath(name)) {
      /* if write and full path: */
      fd = open(name, WR_OPTS);
      if (fd >= 0)
        *fullName = name;
      else
        mfree(csound, name);
      return fd;
    }
    /* search paths defined by environment variable list */
    if (envList != NULL && envList[0] != '\0' &&
        (searchPath = csoundGetSearchPathFromEnv((CSOUND*) csound, envList))
        != NULL) {
      //len = (int) strlen(name) + 1;
      while (*searchPath != NULL) {
        name2 = csoundConcatenatePaths(csound, *searchPath, name);
        if (!write_mode)
          fd = open(name2, RD_OPTS);
        else
          fd = open(name2, WR_OPTS);
        if (fd >= 0) {
          mfree(csound, name);
          *fullName = name2;
          return fd;
        }
        mfree(csound, name2);
        searchPath++;
      }
    }
    /* if write mode, try current directory last */
    if (write_mode) {
      fd = open(name, WR_OPTS);
      if (fd >= 0) {
        *fullName = name;
        return fd;
      }
    }
    /* not found */
    mfree(csound, name);
    return -1;
}

/* Close all open files; called by csoundReset(). */

void close_all_files(CSOUND *csound)
{
    while (csound->open_files != NULL)
      csoundFileClose(csound, csound->open_files);
    if (csound->file_io_start) {
#ifndef __EMSCRIPTEN__
      csoundJoinThread(csound->file_io_thread);
#endif
      if (csound->file_io_threadlock != NULL)
        csoundDestroyThreadLock(csound->file_io_threadlock);
    }
}

/* The fromScore parameter should be 1 if opening a score include file,
   0 if opening an orchestra include file */
void *fopen_path(CSOUND *csound, FILE **fp, char *name, char *basename,
                 char *env, int fromScore)
{
    void *fd;
    int  csftype = (fromScore ? CSFTYPE_SCO_INCLUDE : CSFTYPE_ORC_INCLUDE);

    /* First try to open name given */
    fd = csoundFileOpenWithType(csound, fp, CSFILE_STD, name, "r", NULL,
                           csftype, 0);
    if (fd != NULL)
      return fd;
    /* if that fails try in base directory */
    if (basename != NULL) {
      char *dir, *name_full;
      if ((dir = csoundSplitDirectoryFromPath(csound, basename)) != NULL) {
        name_full = csoundConcatenatePaths(csound, dir, name);
        fd = csoundFileOpenWithType(csound, fp, CSFILE_STD, name_full, "r", NULL,
                               csftype, 0);
        mfree(csound, dir);
        mfree(csound, name_full);
        if (fd != NULL)
          return fd;
      }
    }
    /* or use env argument */
    fd = csoundFileOpenWithType(csound, fp, CSFILE_STD, name, "r", env,
                           csftype, 0);
    return fd;
}

static int read_files(CSOUND *csound){
    CSFILE *current = (CSFILE *) csound->open_files;
    if (current == NULL) return 0;
    while (current) {
      if (current->async_flag == ASYNC_GLOBAL) {
        int m = current->pos, l, n = current->items;
        int items = current->bufsize;
        MYFLT *buf = current->buf;
        switch (current->type) {
        case CSFILE_FD_R:
          break;
        case CSFILE_FD_W:
          break;
        case CSFILE_STD:
          break;
        case CSFILE_SND_R:
          if (n == 0) {
            n = sflib_read_MYFLT(current->sf, buf, items);
            m = 0;
          }
          l = csoundWriteCircularBuffer(csound,current->cb,&buf[m],n);
          m += l;
          n -= l;
          current->items = n;
          current->pos = m;
          break;
        case CSFILE_SND_W:
          items = csoundReadCircularBuffer(csound, current->cb, buf, items);
          if (items == 0) { csoundSleep(10); break;}
          sflib_write_MYFLT(current->sf, buf, items);
          break;
        }
      }
      current = current->nxt;
    }
    return 1;
}




uintptr_t file_iothread(void *p){
    int res = 1;
    CSOUND *csound = p;
    int wakeup = (int) (1000*csound->ksmps/csound->esr);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    if (wakeup == 0) wakeup = 1;
    while (res){
      csoundSleep(wakeup);
      csoundWaitThreadLockNoTimeout(csound->file_io_threadlock);
      res = read_files(csound);
      csoundNotifyThreadLock(csound->file_io_threadlock);
    }
    csound->file_io_start = 0;
    return (uintptr_t)NULL;
}
