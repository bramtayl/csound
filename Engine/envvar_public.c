#include "envvar_public.h"

#include <ctype.h>

#include "csoundCore_internal.h"
#include "envvar.h"
#include "libsnd_u.h"
#include "memalloc.h"
#include "csound_threads.h"
#include "circularbuffer.h"
#include "text.h"

/**
 * Search for input file 'filename'.
 * If the file name specifies full path (it begins with '.', the pathname
 * delimiter character, or a drive letter and ':' on Windows), that exact
 * file name is tried without searching.
 * Otherwise, the file is searched relative to the current directory first,
 * and if it is still not found, a pathname list that is created the
 * following way is searched:
 *   1. if envList is NULL or empty, no directories are searched
 *   2. envList is parsed as a ';' or ':' separated list of environment
 *      variable names, and all environment variables are expanded and
 *      expected to contain a ';' or ':' separated list of directory names
 *   2. all directories in the resulting pathname list are searched, starting
 *      from the last and towards the first one, and the directory where the
 *      file is found first will be used
 * The function returns a pointer to the full name of the file if it is
 * found, and NULL if the file could not be found in any of the search paths,
 * or an error has occured. The caller is responsible for freeing the memory
 * pointed to by the return value, by calling mfree().
 */
char *csoundFindInputFile(CSOUND *csound, const char *filename,
                          const char *envList) {
  char *name_found;
  int fd;

  if (csound == NULL) return NULL;
  fd = csoundFindFile_Fd(csound, &name_found, filename, 0, envList);
  if (fd >= 0) close(fd);
  return name_found;
}

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
char *csoundFindOutputFile(CSOUND *csound, const char *filename,
                           const char *envList) {
  char *name_found;
  int fd;

  if (csound == NULL) return NULL;
  fd = csoundFindFile_Fd(csound, &name_found, filename, 1, envList);
  if (fd >= 0) {
    close(fd);
    csoundWarning(csound,
                  Str("Output file already exists: Will be overwritten.\n"));
  }
  return name_found;
}

/**
 * Allocate a file handle for an existing file already opened with open(),
 * fopen(), or sflib_open(), for later use with csoundFileClose() or
 * csoundGetFileName(), or storing in an FDCH structure.
 * Files registered this way (or opened with csoundFileOpen()) are also
 * automatically closed by csoundReset().
 * Parameters and return value are similar to csoundFileOpen(), except
 * fullName is the name that will be returned by a later call to
 * csoundGetFileName().
 */

void *csoundCreateFileHandle(CSOUND *csound, void *fd, int type,
                             const char *fullName) {
  CSFILE *p = NULL;
  int nbytes = (int)sizeof(CSFILE);

  /* name should not be empty */
  if (fullName == NULL || fullName[0] == '\0') return NULL;
  nbytes += (int)strlen(fullName);
  /* allocate file structure */
  p = (CSFILE *)mcalloc(csound, (size_t)nbytes);
  if (p == NULL) return NULL;
  p->nxt = (CSFILE *)csound->open_files;
  p->prv = (CSFILE *)NULL;
  p->type = type;
  p->fd = -1;
  p->f = (FILE *)NULL;
  p->sf = (SNDFILE *)NULL;
  p->cb = NULL;
  strcpy(&(p->fullName[0]), fullName);
  /* open file */
  switch (type) {
    case CSFILE_FD_R:
    case CSFILE_FD_W:
      p->fd = *((int *)fd);
      break;
    case CSFILE_STD:
      p->f = *((FILE **)fd);
      break;
    case CSFILE_SND_R:
    case CSFILE_SND_W:
      p->sf = *((SNDFILE **)fd);
      break;
    default:
      csoundErrorMsg(csound,
                     Str("internal error: csoundCreateFileHandle(): "
                         "invalid type: %d"),
                     type);
      mfree(csound, p);
      return NULL;
  }
  /* link into chain of open files */
  if (csound->open_files != NULL) ((CSFILE *)csound->open_files)->prv = p;
  csound->open_files = (void *)p;
  /* return with opaque file handle */
  p->cb = NULL;
  return (void *)p;
}

/**
 * Get the full name of a file previously opened with csoundFileOpen().
 */

char *csoundGetFileName(void *fd) { return &(((CSFILE *)fd)->fullName[0]); }

/**
 * Close a file previously opened with csoundFileOpen().
 */

int csoundFileClose(CSOUND *csound, void *fd) {
  CSFILE *p = (CSFILE *)fd;
  int retval = -1;
  if (p->async_flag == ASYNC_GLOBAL) {
    csoundWaitThreadLockNoTimeout(csound->file_io_threadlock);
    /* close file */
    switch (p->type) {
      case CSFILE_FD_R:
      case CSFILE_FD_W:
        retval = close(p->fd);
        break;
      case CSFILE_STD:
        retval = fclose(p->f);
        break;
      case CSFILE_SND_R:
      case CSFILE_SND_W:
        if (p->sf) retval = sflib_close(p->sf);
        p->sf = NULL;
        if (p->fd >= 0) retval |= close(p->fd);
        break;
    }
    /* unlink from chain of open files */
    if (p->prv == NULL)
      csound->open_files = (void *)p->nxt;
    else
      p->prv->nxt = p->nxt;
    if (p->nxt != NULL) p->nxt->prv = p->prv;
    if (p->buf != NULL) mfree(csound, p->buf);
    p->bufsize = 0;
    csoundDestroyCircularBuffer(csound, p->cb);
    csoundNotifyThreadLock(csound->file_io_threadlock);
  } else {
    /* close file */
    switch (p->type) {
      case CSFILE_FD_R:
      case CSFILE_FD_W:
        retval = close(p->fd);
        break;
      case CSFILE_STD:
        retval = fclose(p->f);
        break;
      case CSFILE_SND_R:
      case CSFILE_SND_W:
        retval = sflib_close(p->sf);
        if (p->fd >= 0) retval |= close(p->fd);
        break;
    }
    /* unlink from chain of open files */
    if (p->prv == NULL)
      csound->open_files = (void *)p->nxt;
    else
      p->prv->nxt = p->nxt;
    if (p->nxt != NULL) p->nxt->prv = p->prv;
  }
  /* free allocated memory */
  mfree(csound, fd);

  /* return with error value */
  return retval;
}

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
 *     CSFILE_SND_R:    SFLIB_INFO* parameter for sflib_open(), with defaults
 * for raw file; the actual format paramaters of the opened file will be stored
 * in this structure CSFILE_SND_W:    SFLIB_INFO* parameter for sflib_open(),
 * output file format const char *env: list of environment variables for search
 * path (see csoundFindInputFile() for details); if NULL, the specified name is
 * used as it is, without any conversion or search. int csFileType: A value from
 * the enumeration CSOUND_FILETYPES (see soundCore.h) int isTemporary: 1 if this
 * file will be deleted when Csound is finished. Otherwise, 0. return value:
 *   opaque handle to the opened file, for use with csoundGetFileName() or
 *   csoundFileClose(), or storing in FDCH.fd.
 *   On failure, NULL is returned.
 */

void *csoundFileOpenWithType(CSOUND *csound, void *fd, int type,
                             const char *name, void *param, const char *env,
                             int csFileType, int isTemporary) {
  CSFILE *p = NULL;
  char *fullName = NULL;
  FILE *tmp_f = NULL;
  SFLIB_INFO sfinfo;
  int tmp_fd = -1, nbytes = (int)sizeof(CSFILE);

  /* check file type */
  if (UNLIKELY((unsigned int)(type - 1) >= (unsigned int)CSFILE_SND_W)) {
    csoundErrorMsg(csound,
                   Str("internal error: csoundFileOpen(): "
                       "invalid type: %d"),
                   type);
    return NULL;
  }
  /* get full name and open file */
  if (env == NULL) {
#if defined(_WIN32)
    /* To handle Widows errors in file name characters. */
    size_t sz = 2 * MultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
    wchar_t *wfname = malloc(sz);
    wchar_t *wmode = 0;

    MultiByteToWideChar(CP_UTF8, 0, name, -1, wfname, sz);
    sz = 2 * MultiByteToWideChar(CP_UTF8, 0, param, -1, NULL, 0);
    wmode = malloc(sz);
    MultiByteToWideChar(CP_UTF8, 0, param, -1, wmode, sz);
    if (type == CSFILE_STD) {
      tmp_f = _wfopen(wfname, wmode);
      if (UNLIKELY(tmp_f == NULL)) {
        /* csoundErrorMsg(csound, Str("csoundFileOpenWithType(\"%s\") failed:
         * %s."), */
        /*                name, strerror(errno)); */
        free(wfname);
        free(wmode);
        goto err_return;
      }
      fullName = (char *)name;
    }
    free(wfname);
    free(wmode);
    // so we can resume with an else still
    if (type == CSFILE_STD) {
    }
#else
    if (type == CSFILE_STD) {
      fullName = (char *)name;
      tmp_f = fopen(fullName, (char *)param);
      if (UNLIKELY(tmp_f == NULL)) {
        /* csoundErrorMsg(csound, Str("csoundFileOpenWithType(\"%s\") failed:
         * %s."), */
        /*                name, strerror(errno)); */
        goto err_return;
      }
    }
#endif
    else {
      fullName = (char *)name;
      if (type == CSFILE_SND_R || type == CSFILE_FD_R)
        tmp_fd = open(fullName, RD_OPTS);
      else
        tmp_fd = open(fullName, WR_OPTS);
      if (tmp_fd < 0) goto err_return;
    }
  } else {
    if (type == CSFILE_STD) {
      tmp_f = csoundFindFile_Std(csound, &fullName, name, (char *)param, env);
      if (UNLIKELY(tmp_f == NULL)) goto err_return;
    } else {
      if (type == CSFILE_SND_R || type == CSFILE_FD_R)
        tmp_fd = csoundFindFile_Fd(csound, &fullName, name, 0, env);
      else
        tmp_fd = csoundFindFile_Fd(csound, &fullName, name, 1, env);
      if (UNLIKELY(tmp_fd < 0)) goto err_return;
    }
  }
  nbytes += (int)strlen(fullName);
  /* allocate file structure */
  p = (CSFILE *)mmalloc(csound, (size_t)nbytes);
  if (UNLIKELY(p == NULL)) goto err_return;
  p->nxt = (CSFILE *)csound->open_files;
  p->prv = (CSFILE *)NULL;
  p->type = type;
  p->fd = tmp_fd;
  p->f = tmp_f;
  p->sf = (SNDFILE *)NULL;
  strcpy(&(p->fullName[0]), fullName);
  if (env != NULL) {
    mfree(csound, fullName);
    env = NULL;
  }

  /* if sound file, re-open file descriptor with libsndfile */
  switch (type) {
    case CSFILE_STD: /* stdio */
      *((FILE **)fd) = tmp_f;
      break;
    case CSFILE_SND_R: /* sound file read */
      memcpy(&sfinfo, param, sizeof(SFLIB_INFO));
      p->sf = sflib_open_fd(tmp_fd, SFM_READ, &sfinfo, 0);
      if (p->sf == (SNDFILE *)NULL) {
        int extPos;
        /* open failed: */
        extPos = (nbytes - (int)sizeof(CSFILE)) - 4;
        /* check for .sd2 file first */
        if (extPos > 0 && p->fullName[extPos] == (char)'.' &&
            tolower(p->fullName[extPos + 1]) == (char)'s' &&
            tolower(p->fullName[extPos + 2]) == (char)'d' &&
            p->fullName[extPos + 3] == (char)'2') {
          // memset(&sfinfo, 0, sizeof(SFLIB_INFO));
          p->sf = sflib_open(&(p->fullName[0]), SFM_READ, &sfinfo);
          if (p->sf != (SNDFILE *)NULL) {
            /* if successfully opened as .sd2, */
            /* the integer file descriptor is no longer needed */
            close(tmp_fd);
            p->fd = tmp_fd = -1;
            sflib_command(p->sf, SFC_SET_VBR_ENCODING_QUALITY,
                          &csound->oparms->quality, sizeof(double));
            goto doneSFOpen;
          }
        }
#if 0
        /* maybe raw file ? rewind and try again */
        if (lseek(tmp_fd, (off_t) 0, SEEK_SET) == (off_t) 0) {
          SFLIB_INFO *sf = (SFLIB_INFO*)param;
          sf->format = TYPE2SF(TYP_RAW) |  AE_SHORT ;
          sf->samplerate = csound->esr;
          //sf->channels = 1;//csound->inchnls;
          csoundWarning(csound,
                          Str("After open failure(%s)\n"
                              "will try to open %s as raw\n"),
                          sflib_strerror(NULL), fullName);
          p->sf = sflib_open_fd(tmp_fd, SFM_READ, sf, 0);
        }
#endif
        if (UNLIKELY(p->sf == (SNDFILE *)NULL)) {
          /* csoundWarning(csound, Str("Failed to open %s: %s\n"), */
          /*                 fullName, sflib_strerror(NULL)); */
          goto err_return;
        }
      } else {
      doneSFOpen:
        memcpy((SFLIB_INFO *)param, &sfinfo, sizeof(SFLIB_INFO));
      }
      *((SNDFILE **)fd) = p->sf;
      break;
    case CSFILE_SND_W: /* sound file write */
      p->sf = sflib_open_fd(tmp_fd, SFM_WRITE, (SFLIB_INFO *)param, 0);
      if (UNLIKELY(p->sf == (SNDFILE *)NULL)) {
        csoundWarning(csound, Str("Failed to open %s: %s\n"), fullName,
                      sflib_strerror(NULL));
        goto err_return;
      }
      sflib_command(p->sf, SFC_SET_CLIPPING, NULL, SFLIB_TRUE);
      sflib_command(p->sf, SFC_SET_VBR_ENCODING_QUALITY,
                    &csound->oparms->quality, sizeof(double));
      *((SNDFILE **)fd) = p->sf;
      break;
    default: /* low level I/O */
      *((int *)fd) = tmp_fd;
  }
  /* link into chain of open files */
  if (csound->open_files != NULL) ((CSFILE *)csound->open_files)->prv = p;
  csound->open_files = (void *)p;
  /* notify the host if it asked */
  if (csound->FileOpenCallback_ != NULL) {
    int writing = (type == CSFILE_SND_W || type == CSFILE_FD_W ||
                   (type == CSFILE_STD && ((char *)param)[0] == 'w'));
    if (csFileType == CSFTYPE_UNKNOWN_AUDIO && type == CSFILE_SND_R)
      csFileType = sftype2csfiletype(((SFLIB_INFO *)param)->format);
    csound->FileOpenCallback_(csound, p->fullName, csFileType, writing,
                              isTemporary);
  }
  /* return with opaque file handle */
  p->cb = NULL;
  p->async_flag = 0;
  p->buf = NULL;
  p->bufsize = 0;
  return (void *)p;

err_return:
  /* clean up on error */
  if (p != NULL) mfree(csound, p);
  if (fullName != NULL && env != NULL) mfree(csound, fullName);
  if (tmp_fd >= 0)
    close(tmp_fd);
  else if (tmp_f != NULL)
    fclose(tmp_f);
  if (type > CSFILE_STD)
    *((SNDFILE **)fd) = (SNDFILE *)NULL;
  else if (type == CSFILE_STD)
    *((FILE **)fd) = (FILE *)NULL;
  else
    *((int *)fd) = -1;
  return NULL;
}

void *csoundFileOpenWithType_Async(CSOUND *csound, void *fd, int type,
                                   const char *name, void *param,
                                   const char *env, int csFileType,
                                   int buffsize, int isTemporary) {
#ifndef __EMSCRIPTEN__
  CSFILE *p;
  if ((p = (CSFILE *)csoundFileOpenWithType(csound, fd, type, name, param, env,
                                            csFileType, isTemporary)) == NULL)
    return NULL;

  if (csound->file_io_start == 0) {
    csound->file_io_start = 1;
    csound->file_io_threadlock = csoundCreateThreadLock();
    csoundNotifyThreadLock(csound->file_io_threadlock);
    csound->file_io_thread = csoundCreateThread(file_iothread, (void *)csound);
  }
  csoundWaitThreadLockNoTimeout(csound->file_io_threadlock);
  p->async_flag = ASYNC_GLOBAL;

  p->cb = csoundCreateCircularBuffer(csound, buffsize * 4, sizeof(MYFLT));
  p->items = 0;
  p->pos = 0;
  p->bufsize = buffsize;
  p->buf = (MYFLT *)mcalloc(csound, sizeof(MYFLT) * buffsize);
  csoundNotifyThreadLock(csound->file_io_threadlock);

  if (p->cb == NULL || p->buf == NULL) {
    /* close file immediately */
    csoundFileClose(csound, (void *)p);
    return NULL;
  }
  return (void *)p;
#else
  return NULL;
#endif
}

unsigned int csoundReadAsync(CSOUND *csound, void *handle, MYFLT *buf,
                             int items) {
  CSFILE *p = handle;
  if (p != NULL && p->cb != NULL)
    return csoundReadCircularBuffer(csound, p->cb, buf, items);
  else
    return 0;
}

unsigned int csoundWriteAsync(CSOUND *csound, void *handle, MYFLT *buf,
                              int items) {
  CSFILE *p = handle;
  if (p != NULL && p->cb != NULL)
    return csoundWriteCircularBuffer(csound, p->cb, buf, items);
  else
    return 0;
}

int csoundFSeekAsync(CSOUND *csound, void *handle, int pos, int whence) {
  CSFILE *p = handle;
  int ret = 0;
  csoundWaitThreadLockNoTimeout(csound->file_io_threadlock);
  switch (p->type) {
    case CSFILE_FD_R:
      break;
    case CSFILE_FD_W:
      break;
    case CSFILE_STD:
      break;
    case CSFILE_SND_R:
    case CSFILE_SND_W:
      ret = sflib_seek(p->sf, pos, whence);
      // csoundMessage(csound, "seek set %d\n", pos);
      csoundFlushCircularBuffer(csound, p->cb);
      p->items = 0;
      break;
  }
  csoundNotifyThreadLock(csound->file_io_threadlock);
  return ret;
}

/**
 * Get pointer to value of environment variable 'name'.
 * Return value is NULL if the variable is not set.
 */

PUBLIC const char *csoundGetEnv(CSOUND *csound, const char *name) {
  if (csound == NULL) {
      int i;
      if (name == NULL || name[0] == '\0')
      return (const char *)NULL;
      for (i = 0; i < 16; i++) {
      if (strcmp(globalEnvVarName(i), name) == 0)
          return (const char *)globalEnvVarValue(i);
      }
      return (const char *)getenv(name);
  }

  if (csound->envVarDB == NULL)
      return NULL;

  return (const char *)cs_hash_table_get(csound, csound->envVarDB,
                                         (char *)name);
}

/**
 * Set the global value of environment variable 'name' to 'value',
 * or delete variable if 'value' is NULL.
 * It is not safe to call this function while any Csound instances
 * are active.
 * Returns zero on success.
 */

PUBLIC int csoundSetGlobalEnv(const char *name, const char *value) {
  int i;

  if (UNLIKELY(name == NULL || name[0] == '\0' || (int)strlen(name) >= 32))
      return -1; /* invalid name             */
  for (i = 0; i < 16; i++) {
      if ((value != NULL && globalEnvVarName(i)[0] == '\0') ||
          strcmp(name, globalEnvVarName(i)) == 0)
      break;
  }
  if (UNLIKELY(i >= 16)) /* not found / no free slot */
      return -1;
  if (value == NULL) {
      globalEnvVarName(i)[0] = '\0'; /* delete existing variable */
      return 0;
  }
  if (UNLIKELY(strlen(value) >= 480))
      return -1; /* string value is too long */
  strcpy(globalEnvVarName(i), name);
  strcpy(globalEnvVarValue(i), value);
  return 0;
}