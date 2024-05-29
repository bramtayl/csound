/*
    csmodule.c:

    Copyright (C) 2005 Istvan Varga
    based on dl_opcodes.c, Copyright (C) 2002 John ffitch

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

/******************************************************************************
 * NEW PLUGIN INTERFACE                                                       *
 * ====================                                                       *
 *                                                                            *
 * Plugin libraries are loaded from the directory defined by the environment  *
 * variable OPCODE7DIR (or the current directory if OPCODE7DIR is unset) by   *
 * csoundPreCompile() while initialising a Csound instance, and are unloaded  *
 * at the end of performance by csoundReset().                                *
 * A library may export any of the following five interface functions,        *
 * however, the presence of csoundModuleCreate() is required for identifying  *
 * the file as a Csound plugin module.                                        *
 *                                                                            *
 * int csoundModuleCreate(CSOUND *csound)       (required)                    *
 * --------------------------------------                                     *
 *                                                                            *
 * Pre-initialisation function, called by csoundPreCompile().                 *
 *                                                                            *
 * int csoundModuleInit(CSOUND *csound)         (optional)                    *
 * ------------------------------------                                       *
 *                                                                            *
 * Called by Csound instances before orchestra translation. One possible use  *
 * of csoundModuleInit() is adding new opcodes with csoundAppendOpcode().     *
 *                                                                            *
 * int csoundModuleDestroy(CSOUND *csound)      (optional)                    *
 * ---------------------------------------                                    *
 *                                                                            *
 * Destructor function for Csound instance 'csound', called at the end of     *
 * performance, after closing audio output.                                   *
 *                                                                            *
 * const char *csoundModuleErrorCodeToString(int errcode)   (optional)        *
 * ------------------------------------------------------                     *
 *                                                                            *
 * Converts error codes returned by any of the initialisation or destructor   *
 * functions to a string message.                                             *
 *                                                                            *
 * int csoundModuleInfo(void)                   (optional)                    *
 * --------------------------                                                 *
 *                                                                            *
 * Returns information that can be used to determine if the plugin was built  *
 * for a compatible version of libcsound. The return value may be the sum of  *
 * any of the following two values:                                           *
 *                                                                            *
 *   ((CS_APIVERSION << 16) + (CS_APISUBVER << 8))      API version           *
 *   (int) sizeof(MYFLT)                                MYFLT type            *
 *                                                                            *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !(defined (__wasi__))
#include <setjmp.h>
#include <errno.h>
#endif

#include "csoundCore_internal.h"
#include "csmodule.h"
#include "csmodule_public.h"
#include "memalloc.h"
#include "csound_orc_semantics_public.h"
#include "envvar_public.h"
#include "text.h"
#include "sfont.h"
#include "stdopcod.h"
#include "pvs_ops.h"
#include "gab/newgabopc.h"
#include "fgens.h"

#include "csound_type_system_internal.h"

#if defined(__MACH__)
#include <TargetConditionals.h>
#if (TARGET_OS_IPHONE == 0) && (TARGET_IPHONE_SIMULATOR == 0)
#if defined(MAC_OS_X_VERSION_10_6) && \
    (MAC_OS_X_VERSION_MIN_REQUIRED>=MAC_OS_X_VERSION_10_6)
#define NEW_MACH_CODE
#endif
#endif
#endif

#if !(defined (NACL)) && !(defined (__wasi__))
#if defined(__gnu_linux__) || defined(NEW_MACH_CODE) || defined(__HAIKU__)
#include <dlfcn.h>
#elif defined(_WIN32)
#include <windows.h>
#endif
#endif


#if defined(HAVE_DIRENT_H)
#  include <dirent.h>
#  if 0 && defined(__MACH__)
typedef void*   DIR;
DIR             opendir(const char *);
struct dirent   *readdir(DIR*);
int             closedir(DIR*);
#  endif
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <io.h>
#  include <direct.h>
#endif

#if defined(INIT_STATIC_MODULES)
#include "init_static_modules.h"
#endif

/* module interface function names */

static  const   char    *opcode_init_Name =   "csound_opcode_init";
static  const   char    *fgen_init_Name =     "csound_fgen_init";

static  const   char    *PreInitFunc_Name =   "csoundModuleCreate";
static  const   char    *InitFunc_Name =      "csoundModuleInit";
static  const   char    *DestFunc_Name =      "csoundModuleDestroy";
static  const   char    *ErrCodeToStr_Name =  "csoundModuleErrorCodeToString";

static  const   char    *InfoFunc_Name =      "csoundModuleInfo";

/* environment variable storing path to plugin libraries */
static  const   char    *plugindir_envvar =   "OPCODE7DIR";
static  const   char    *plugindir64_envvar = "OPCODE7DIR64";

/* default directory to load plugins from if environment variable is not set */
#if !(defined (NACL)) && !(defined (__wasi__))
#ifdef __HAIKU__
# ifndef USE_DOUBLE
   static char haikudirs[] = "/boot/system/lib/csound7/plugins:"
        "/boot/home/config/lib/csound7/plugins:"
        "/boot/system/non-packaged/lib/csound7/plugins:"
        "/boot/home/config/non-packaged/lib/csound7/plugins";
# else
   static char haikudirs[] = "/boot/system/lib/csound7/plugins64:"
        "/boot/home/config/lib/csound7/plugins64:"
        "/boot/system/non-packaged/lib/csound7/plugins64:"
        "/boot/home/config/non-packaged/lib/csound7/plugins64";
# endif
# define CS_DEFAULT_PLUGINDIR  haikudirs
#elif !(defined(_CSOUND_RELEASE_) && (defined(__gnu_linux__) || defined(__MACH__)))
#  define ENABLE_OPCODEDIR_WARNINGS 1
#  ifdef CS_DEFAULT_PLUGINDIR
#    undef CS_DEFAULT_PLUGINDIR
#  endif
#  define CS_DEFAULT_PLUGINDIR      "."
#else
#  define ENABLE_OPCODEDIR_WARNINGS 0
#  ifndef CS_DEFAULT_PLUGINDIR
#    ifndef USE_DOUBLE
#      define CS_DEFAULT_PLUGINDIR  "/usr/local/lib/csound/plugins"
#    else
#      define CS_DEFAULT_PLUGINDIR  "/usr/local/lib/csound/plugins64"
#    endif
#  endif
#endif
#endif

#if (TARGET_OS_IPHONE != 0) && (TARGET_IPHONE_SIMULATOR != 0)
#  define ENABLE_OPCODEDIR_WARNINGS 0
#endif

typedef struct opcodeLibFunc_s {
    int64_t  (*opcode_init)(CSOUND *, OENTRY **);  /* list of opcode entries  */
    NGFENS  *(*fgen_init)(CSOUND *);        /* list of named GEN routines    */
    void    (*dummy)(void);                 /* unused                        */
} opcodeLibFunc_t;

typedef struct pluginLibFunc_s {
    int         (*InitFunc)(CSOUND *);      /* initialisation routine        */
    int         (*DestFunc)(CSOUND *);      /* destructor routine            */
    const char  *(*ErrCodeToStr)(int);      /* convert error code to string  */
} pluginLibFunc_t;

typedef struct csoundModule_s {
    struct csoundModule_s *nxt;             /* pointer to next link in chain */
    void        *h;                         /* library handle                */
    int         (*PreInitFunc)(CSOUND *);   /* pre-initialisation routine    */
                                            /*   (always NULL if opcode lib) */
    union {
      pluginLibFunc_t   p;                  /* generic plugin interface      */
      opcodeLibFunc_t   o;                  /* opcode library interface      */
    } fn;
    char        name[1];                    /* name of the module            */
} csoundModule_t;

static CS_NOINLINE void print_module_error(CSOUND *csound,
                                           const char *fmt, const char *fname,
                                           const csoundModule_t *m, int err)
{
    csoundMessageS(csound, CSOUNDMSG_ERROR, Str(fmt), fname);
    if (m != NULL && m->fn.p.ErrCodeToStr != NULL)
      csoundMessageS(csound, CSOUNDMSG_ERROR,
                       ": %s\n", Str(m->fn.p.ErrCodeToStr(err)));
    else
      csoundMessageS(csound, CSOUNDMSG_ERROR, "\n");
}

static int check_plugin_compatibility(CSOUND *csound, const char *fname, int n)
{
    int     myfltSize, minorVersion, majorVersion;

    myfltSize = n & 0xFF;
    if (UNLIKELY(myfltSize != 0 && myfltSize != (int) sizeof(MYFLT))) {
      csoundWarning(csound, Str("not loading '%s' (uses incompatible "
                                  "floating point type)"), fname);
      return -1;
    }
    if (UNLIKELY(n & (~0xFF))) {
      minorVersion = (n & 0xFF00) >> 8;
      majorVersion = (n & (~0xFFFF)) >> 16;
      if (majorVersion != (int) CS_APIVERSION ||
          (minorVersion > (int) CS_APISUBVER)) { /* NOTE **** REFACTOR *** */
        csoundWarning(csound, Str("not loading '%s' (incompatible "
                                    "with this version of Csound (%d.%d/%d.%d)"),
                        fname, majorVersion,minorVersion,
                        CS_APIVERSION,CS_APISUBVER);
        return -1;
      }
    }
    return 0;
}


/**
 * Initialise a single module.
 * Return value is CSOUND_SUCCESS if there was no error.
 */
static CS_NOINLINE int csoundInitModule(CSOUND *csound, csoundModule_t *m)
{
    int     i;

    if (m->PreInitFunc != NULL) {
      if (m->fn.p.InitFunc != NULL) {
        i = m->fn.p.InitFunc(csound);
        if (UNLIKELY(i != 0)) {
          print_module_error(csound, Str("Error starting module '%s'"),
                                     &(m->name[0]), m, i);
          return CSOUND_ERROR;
        }
      }
    }
    else {
      /* deal with fgens if there are any */
      if (m->fn.o.fgen_init != NULL) {
        NGFENS  *names = m->fn.o.fgen_init(csound);
        for (i = 0; names[i].name != NULL; i++)
          allocgen(csound, names[i].name, names[i].fn);
      }
      if (m->fn.o.opcode_init != NULL) {
        OENTRY  *opcodlst_n;
        int64_t    length;
        /* load opcodes */
        if (UNLIKELY((length = m->fn.o.opcode_init(csound, &opcodlst_n)) < 0L))
          return CSOUND_ERROR;
        else {
          length /= (long) sizeof(OENTRY);
          if (length) {
            if (UNLIKELY(csoundAppendOpcodes(csound, opcodlst_n,
                                               (int) length) != 0))
              return CSOUND_ERROR;
          }
        }
      }
    }
    return CSOUND_SUCCESS;
}


#ifdef __wasi__

__attribute__((used))
void csoundWasiLoadPlugin(CSOUND *csound, void *preInitFunc, void *initFunc, void *destFunc, void *errCodeToStr) {
    csoundModule_t *module = mmalloc(csound, sizeof(csoundModule_t) + 1);
    module->h = (void*) NULL;

    // The javascript host must assert that this is provided
    module->PreInitFunc = (int (*)(CSOUND *)) preInitFunc;
    if (initFunc) {
        module->fn.p.InitFunc = (int (*)(CSOUND *)) initFunc;
    }
    if (destFunc) {
        module->fn.p.DestFunc = (int (*)(CSOUND *)) destFunc;
    }
    if (errCodeToStr) {
        module->fn.p.ErrCodeToStr = (const char *(*)(int)) errCodeToStr;
    }

    module->nxt = (csoundModule_t*) csound->csmodule_db;
    csound->csmodule_db = module;

    module->PreInitFunc(csound);
}

__attribute__((used))
void csoundWasiLoadOpcodeLibrary(CSOUND *csound, void *fgenInitFunc, void *opcodeInitFunc) {
    csoundModule_t *module = mmalloc(csound, sizeof(csoundModule_t) + 1);
    module->h = (void*) NULL;

    if (fgenInitFunc) {
        module->fn.o.fgen_init = (NGFENS *(*)(CSOUND *)) fgenInitFunc;
    }

    if (opcodeInitFunc) {
        module->fn.o.opcode_init = (int64_t (*)(CSOUND *, OENTRY **)) opcodeInitFunc;
    }

    module->nxt = (csoundModule_t*) csound->csmodule_db;
    csound->csmodule_db = module;
}

__attribute__((used))
int csoundDestroyModules(CSOUND *csound) {
    csoundModule_t  *m;
    int i;
    int retval = CSOUND_SUCCESS;

    while (csound->csmodule_db != NULL) {
      m = (csoundModule_t*) csound->csmodule_db;
      /* call destructor functions */
      if (m->PreInitFunc != NULL && m->fn.p.DestFunc != NULL) {
        i = m->fn.p.DestFunc(csound);
        if (UNLIKELY(i != 0)) {
          print_module_error(csound, Str("Error de-initialising module '%s'"),
                                     &(m->name[0]), m, i);
          retval = CSOUND_ERROR;
        }
      }
      csound->csmodule_db = (void*) m->nxt;
      /* free memory used by database */
      mfree(csound, (void*) m);
    }
    /* return with error code */
    return retval;
}

__attribute__((used))
int csoundInitModules(CSOUND *csound) {
    csoundModule_t  *m;
    int i, retval = CSOUND_SUCCESS;
    for (m = (csoundModule_t*) csound->csmodule_db; m != NULL; m = m->nxt) {
      i = csoundInitModule(csound, m);
      if (UNLIKELY(i != CSOUND_SUCCESS && i < retval))
        retval = i;
    }
    /* return with error code */
    return retval;
}

// In browser-wasi, this function is replaced
// by the js-host.

__attribute__((used))
extern int csoundLoadModules(CSOUND *csound);

int csoundLoadAndInitModules(CSOUND *csound, const char *opdir) {
    return 0;
}

#else /* __wasi__ */


/* load a single plugin library, and run csoundModuleCreate() if present */
/* returns zero on success */

static CS_NOINLINE int csoundLoadExternal(CSOUND *csound,
                                          const char *libraryPath)
{
    csoundModule_t  m;
    volatile jmp_buf tmpExitJmp;
    csoundModule_t  *mp;
    char            *fname;
    void            *h, *p;
    int             (*infoFunc)(void);
    int             err;

    /* check for a valid name */
    if (UNLIKELY(libraryPath == NULL || libraryPath[0] == '\0'))
      return CSOUND_ERROR;
    /* remove leading directory components from name */
    fname = (char*) libraryPath + (int) strlen(libraryPath);
    for ( ; fname[0] != DIRSEP && fname != (char*) libraryPath; fname--)
      ;
    if (fname[0] == DIRSEP)
      fname++;
    if (UNLIKELY(fname[0] == '\0'))
      return CSOUND_ERROR;
    /* load library */
/*  #if defined(__gnu_linux__) */
    //printf("About to open library '%s'\n", libraryPath);
/* #endif */
    err = csoundOpenLibrary(&h, libraryPath);
    if (UNLIKELY(err)) {
      char ERRSTR[256];
 #if !(defined(NACL)) && (defined(__gnu_linux__) || defined(__HAIKU__))
      snprintf(ERRSTR, 256, Str("could not open library '%s' (%s)"),
               libraryPath, dlerror());
 #else
      snprintf(ERRSTR, 256, Str("could not open library '%s' (%d)"),
               libraryPath, err);
 #endif
      if (csound->delayederrormessages == NULL) {
        csound->delayederrormessages = mmalloc(csound, strlen(ERRSTR)+1);
        strcpy(csound->delayederrormessages, ERRSTR);
      }
      else {
        char *new =
          mrealloc(csound, csound->delayederrormessages,
                          strlen(csound->delayederrormessages)+strlen(ERRSTR)+11);
        if (UNLIKELY(new==NULL)) {
          mfree(csound, csound->delayederrormessages);
          return CSOUND_ERROR;
        }
        csound->delayederrormessages = new;
        strcat(csound->delayederrormessages, "\nWARNING: ");
        strcat(csound->delayederrormessages, ERRSTR);
      }
      return CSOUND_ERROR;
    }
    /* check if the library is compatible with this version of Csound */
    infoFunc = (int (*)(void)) csoundGetLibrarySymbol(h, InfoFunc_Name);
    if (infoFunc != NULL) {
      if (UNLIKELY(check_plugin_compatibility(csound, fname, infoFunc()) != 0)) {
        csoundCloseLibrary(h);
        return CSOUND_ERROR;
      }
    }
    /* was this plugin already loaded ? */
    for (mp = (csoundModule_t*) csound->csmodule_db; mp != NULL; mp = mp->nxt) {
      if (UNLIKELY(mp->h == h)) {
        csoundCloseLibrary(h);
        return CSOUND_SUCCESS;
      }
    }
    /* find out if it is a Csound plugin */
    memset(&m, 0, sizeof(csoundModule_t));
    m.h = h;
    m.PreInitFunc =
        (int (*)(CSOUND *)) csoundGetLibrarySymbol(h, PreInitFunc_Name);
    if (m.PreInitFunc != NULL) {
      /* generic plugin library */
      m.fn.p.InitFunc =
          (int (*)(CSOUND *)) csoundGetLibrarySymbol(h, InitFunc_Name);
      m.fn.p.DestFunc =
          (int (*)(CSOUND *)) csoundGetLibrarySymbol(h, DestFunc_Name);
      m.fn.p.ErrCodeToStr =
          (const char *(*)(int)) csoundGetLibrarySymbol(h, ErrCodeToStr_Name);
    }
    else {
      /* opcode library */
      m.fn.o.opcode_init =
          (int64_t (*)(CSOUND *, OENTRY **))
              csoundGetLibrarySymbol(h, opcode_init_Name);
      m.fn.o.fgen_init =
          (NGFENS *(*)(CSOUND *)) csoundGetLibrarySymbol(h, fgen_init_Name);
      if (UNLIKELY(m.fn.o.opcode_init == NULL && m.fn.o.fgen_init == NULL)) {
        /* must have csound_opcode_init() or csound_fgen_init() */
        csoundCloseLibrary(h);
        if (UNLIKELY(csound->oparms->msglevel & 0x400))
          csoundWarning(csound, Str("'%s' is not a Csound plugin library"),
                          libraryPath);
        return CSOUND_ERROR;
      }
    }
    /* set up module info structure */
    /* (note: space for NUL character is already included in size of struct) */
    p = (void*) mmalloc(csound,
                               sizeof(csoundModule_t) + (size_t) strlen(fname));
    if (UNLIKELY(p == NULL)) {
      csoundCloseLibrary(h);
      csoundErrorMsg(csound,
                       Str("csoundLoadExternal(): memory allocation failure"));
      return CSOUND_MEMORY;
    }
    mp = (csoundModule_t*) p;
    memcpy(mp, &m, sizeof(csoundModule_t));
    strcpy(&(mp->name[0]), fname);
    /* link into database */
    mp->nxt = (csoundModule_t*) csound->csmodule_db;
    csound->csmodule_db = (void*) mp;
    /* call csoundModuleCreate() if available */
    if (m.PreInitFunc != NULL) {
      memcpy((void*) &tmpExitJmp, (void*) &csound->exitjmp, sizeof(jmp_buf));
      if ((err = setjmp(csound->exitjmp)) != 0) {
        memcpy((void*) &csound->exitjmp, (void*) &tmpExitJmp, sizeof(jmp_buf));
        print_module_error(csound, Str("Error in pre-initialisation function "
                                       "of module '%s'"), fname, NULL, 0);
        return (err == (CSOUND_EXITJMP_SUCCESS + CSOUND_MEMORY) ?
                CSOUND_MEMORY : CSOUND_INITIALIZATION);
      }
      err = m.PreInitFunc(csound);
      memcpy((void*) &csound->exitjmp, (void*) &tmpExitJmp, sizeof(jmp_buf));
      if (UNLIKELY(err != 0)) {
        print_module_error(csound, Str("Error in pre-initialisation function "
                                       "of module '%s'"), fname, &m, err);
        return CSOUND_INITIALIZATION;
      }
    }
    /* plugin was loaded successfully */
    return CSOUND_SUCCESS;
}

static int csoundCheckOpcodeDeny(CSOUND * csound, const char *fname)
{
    /* Check to see if the fname is on the do-not-load list */
    char buff[256];
    char *th;
    char *p, *deny;
    char *list = getenv("CS_OMIT_LIBS");
    /* printf("DEBUG %s(%d): check fname=%s\n", __FILE__, __LINE__, fname); */
    /* printf("DEBUG %s(%d): list %s\n", __FILE__, __LINE__, list); */
    if (list==NULL) return 0;
    strNcpy(buff, fname, 255); //buff[255]='\0';
    strrchr(buff, '.')[0] = '\0'; /* Remove .so etc */
    p = cs_strdup(csound, list);
    deny = cs_strtok_r(p, ",", &th);
    /* printf("DEBUG %s(%d): check buff=%s\n", __FILE__, __LINE__, deny); */
    while (deny) {
      /* printf("DEBUG %s(%d): deny=%s\n", __FILE__, __LINE__, deny); */
      if (strcmp(deny, buff)==0) {
        mfree(csound, p);
        /* printf("DEBUG %s(%d): found\n", __FILE__, __LINE__); */
        return 1;
      }
      deny = cs_strtok_r(NULL, ",", &th);
    }
    mfree(csound, p);
    /* printf("DEBUG %s(%d): not found\n", __FILE__, __LINE__); */
    return 0;
}


static int _dir_exists(char *path) {
    // returns 1 if path is a directory and it exists
    struct stat s;
    int err = stat(path, &s);
    return (err == 0 && S_ISDIR(s.st_mode)) ? 1 : 0;
}


/**
 * Load plugin libraries for Csound instance 'csound', and call
 * pre-initialisation functions.
 * Return value is CSOUND_SUCCESS if there was no error, CSOUND_ERROR if
 * some modules could not be loaded or initialised, and CSOUND_MEMORY
 * if a memory allocation failure has occured.
 */
int csoundLoadModules(CSOUND *csound)
{
#if (defined(HAVE_DIRENT_H) && (TARGET_OS_IPHONE == 0))
    DIR *dir;
    struct dirent *f;
    const char *dname, *fname;
    enum { searchpath_buflen = 2048, buflen = 1024 };
    char buf[buflen];
    int i, n, len, err = CSOUND_SUCCESS;
    char *dname1, *end;
    int read_directory = 1;
    char searchpath_buf[searchpath_buflen];
    char sep =
#ifdef _WIN32
    ';';
#else
    ':';
#endif
#ifdef __HAIKU__
        int dfltdir = 0;
#endif

    if (UNLIKELY(csound->csmodule_db != NULL))
      return CSOUND_ERROR;

    /* open plugin directory */
    dname = csoundGetEnv(csound, (sizeof(MYFLT) == sizeof(float) ?
                                  plugindir_envvar : plugindir64_envvar));
    if (dname == NULL) {
#if ENABLE_OPCODEDIR_WARNINGS
      csound->opcodedirWasOK = 0;
#  ifdef USE_DOUBLE
      dname = csoundGetEnv(csound, plugindir_envvar);
      if (dname == NULL)
#  endif
#endif
#ifdef  CS_DEFAULT_PLUGINDIR
        dname = CS_DEFAULT_PLUGINDIR;
  #ifdef __HAIKU__
        dfltdir = 1;
  #endif
#else
      dname = "";
#endif
    }

    /* opcodedir GLOBAL override **experimental** */
    if (csound->opcodedir != NULL) {
      dname = csound->opcodedir;
      csoundMessage(csound, "OPCODEDIR overridden to %s \n", dname);
    }
    size_t pos = strlen(dname);
    char *userplugindir = getenv("CS_USER_PLUGINDIR");
    // The user set a search path for plugins via an env variable. Paths here
    // should be absolute and should not need variable expansion
    if(userplugindir != NULL) {
      snprintf(searchpath_buf, searchpath_buflen, "%s%c%s", dname, sep, userplugindir);
      dname = searchpath_buf;
    } else {
#ifdef CS_DEFAULT_USER_PLUGINDIR
      // Use default user path
      // In this case, userplugindir is a relative path to a prefix wich needs
      // to be expanded
      userplugindir = CS_DEFAULT_USER_PLUGINDIR;

#if defined(__gnu_linux__) || defined(__MACH__)
      char *prefix = getenv("HOME");
#elif defined(_WIN32)
      char *prefix = getenv("LOCALAPPDATA");
#endif
      // VL: need to check so we don't get a segfault with NULL strings
      size_t prefixlen = prefix ? strlen(prefix) : 0;
      size_t userplugindirlen = userplugindir ? strlen(userplugindir) : 0;

      if(pos + prefixlen + 2 > searchpath_buflen - 1) {
        csoundErrorMsg(csound, Str("Plugins search path too long\n"));
      } else if(userplugindirlen + prefixlen + 1 >= buflen) {
        csoundErrorMsg(csound, Str("User plugin dir too long\n"));
      } else {
      
        snprintf(buf, buflen, "%s/%s", prefix, userplugindir);
        if(_dir_exists(buf)) {
          snprintf(searchpath_buf, searchpath_buflen, "%s%c%s", dname, sep, buf);
          dname = searchpath_buf;
        }
      }
#endif
    }
 
    if(UNLIKELY(csound->oparms->odebug))
      csoundMessage(csound, Str("Plugins search path: %s\n"), dname);

    /* We now loop through the directory list */
    while(read_directory) {
      /* find separator */
    if((end = strchr(dname, sep)) != NULL) {
      *end = '\0';
      /* copy directory name */
      dname1 = cs_strdup(csound, (char *) dname);

      *end = sep;  /* restore for re-execution */
      /* move to next directory name */
      dname = end + 1;

    } else {
      /* copy last directory name) */
      dname1 = cs_strdup(csound, (char *) dname);
      read_directory = 0;
    }

    /* protect for the case where there is an
       extra separator at the end */
    if(*dname1 == '\0') {
      mfree(csound, dname1);
      break;
    }
    dir = opendir(dname1);
    if (UNLIKELY(dir == (DIR*) NULL)) {
 #if defined(__HAIKU__)
        if(!dfltdir)
 #endif
      csoundWarning(csound, Str("Error opening plugin directory '%s': %s"),
                               dname1, strerror(errno));
      mfree(csound, dname1);
      continue;
    }
    if(UNLIKELY(csound->oparms->odebug))
      csoundMessage(csound, "Opening plugin directory: %s\n", dname1);
    /* load database for deferred plugin loading */
/*     n = csoundLoadOpcodeDB(csound, dname); */
/*     if (n != 0) */
/*       return n; */
    /* scan all files in directory */
    while ((f = readdir(dir)) != NULL) {
      fname = &(f->d_name[0]);
      if (UNLIKELY(fname[0]=='_')) continue;
      n = len = (int) strlen(fname);
      if (UNLIKELY(fname[0]=='_')) continue;
#if defined(_WIN32)
      strcpy(buf, "dll");
      n -= 4;
#elif defined(__MACH__)
      strcpy(buf, "dylib");
      n -= 6;
#else
      strcpy(buf, "so");
      n -= 3;
#endif
      if (n <= 0 || fname[n] != '.')
        continue;
      i = 0;
      do {
        if (UNLIKELY((fname[++n] | (char) 0x20) != buf[i]))
          break;
      } while (buf[++i] != '\0');
      if (buf[i] != '\0')
        continue;
      /* found a dynamic library, attempt to open it */
      if (UNLIKELY(((int) strlen(dname) + len + 2) > 1024)) {
        csoundWarning(csound, Str("path name too long, skipping '%s'"),
                                fname);
        continue;
      }
      /* printf("DEBUG %s(%d): possibly deny %s\n", __FILE__, __LINE__,fname); */
      if (UNLIKELY(csoundCheckOpcodeDeny(csound, fname))) {
        csoundWarning(csound, Str("Library %s omitted\n"), fname);
        continue;
      }

      snprintf(buf, 1024, "%s%c%s", dname1, DIRSEP, fname);

      if (UNLIKELY(csound->oparms->odebug)) {
        csoundMessage(csound, Str("Loading '%s'\n"), buf);
       }
      n = csoundLoadExternal(csound, buf);
      if (UNLIKELY(UNLIKELY(n == CSOUND_ERROR)))
        continue;               /* ignore non-plugin files */
      if (UNLIKELY(n < err))
        err = n;                /* record serious errors */
    }
    closedir(dir);
    mfree(csound, dname1);
    }
    return (err == CSOUND_INITIALIZATION ? CSOUND_ERROR : err);
#else
    return CSOUND_SUCCESS;
#endif  /* HAVE_DIRENT_H */
}


static int cmp_func(const void *p1, const void *p2)
{
    return (strcmp(*((const char**) p1), *((const char**) p2)));
}

int csoundLoadExternals(CSOUND *csound)
{
    char    *s, **lst;
    int     i, cnt, err;

    s = csound->dl_opcodes_oplibs;
    if (UNLIKELY(s == NULL || s[0] == '\0'))
      return 0;
    /* IV - Feb 19 2005 */
    csound->dl_opcodes_oplibs = NULL;
    csoundMessage(csound, Str("Loading command-line libraries:\n"));
    cnt = 1;
    i = 0;
    do {
      if (s[i] == ',')
        cnt++;
    } while (s[++i] != '\0');
    lst = (char**) mmalloc(csound, sizeof(char*) * cnt);
    i = cnt = 0;
    lst[cnt++] = s;
    do {
      if (s[i] == ',') {
        lst[cnt++] = &(s[i + 1]);
        s[i] = '\0';
      }
    } while (s[++i] != '\0');
    qsort((void*) lst, (size_t) cnt, sizeof(char*), cmp_func);
    i = 0;
    do {
      char  *fname = lst[i];
      if (fname[0] != '\0' && !(i && strcmp(fname, lst[i - 1]) == 0)) {
        err = csoundLoadExternal(csound, fname);
        if (UNLIKELY(err == CSOUND_INITIALIZATION || err == CSOUND_MEMORY))
          csoundDie(csound, Str(" *** error loading '%s'"), fname);
        else if (!err)
          csoundMessage(csound, "  %s\n", fname);
      }
    } while (++i < cnt);
    /* file list is no longer needed */
    mfree(csound, lst);
    mfree(csound, s);
    return 0;
}

/**
 * Call initialisation functions of all loaded modules that have a
 * csoundModuleInit symbol, for Csound instance 'csound'.
 * Return value is CSOUND_SUCCESS if there was no error, and CSOUND_ERROR if
 * some modules could not be initialised.
 */

int csoundInitModules(CSOUND *csound)
{
    csoundModule_t  *m;
    int             i, retval = CSOUND_SUCCESS;
    /* For regular Csound, init_static_modules is not compiled or called.
     * For some builds of Csound, e.g. for PNaCl, init_static_modules is
     * compiled and called to initialize statically linked opcodes and other
     * plugins that are dynamically loaded on other platforms.
     */
#if defined(INIT_STATIC_MODULES)
    retval = init_static_modules(csound);
#endif
    /* call init functions */
    for (m = (csoundModule_t*) csound->csmodule_db; m != NULL; m = m->nxt) {
      i = csoundInitModule(csound, m);
      if (UNLIKELY(i != CSOUND_SUCCESS && i < retval))
        retval = i;
    }
    /* return with error code */
    return retval;
}

/* load a plugin library and also initialise it */
/* called on deferred loading of opcode plugins */

int csoundLoadAndInitModule(CSOUND *csound, const char *fname)
{
    volatile jmp_buf  tmpExitJmp;
    volatile int      err;

    err = csoundLoadExternal(csound, fname);
    if (UNLIKELY(err != 0))
      return err;
    memcpy((void*) &tmpExitJmp, (void*) &csound->exitjmp, sizeof(jmp_buf));
    if (UNLIKELY((err = setjmp(csound->exitjmp)) != 0)) {
      memcpy((void*) &csound->exitjmp, (void*) &tmpExitJmp, sizeof(jmp_buf));
      return (err == (CSOUND_EXITJMP_SUCCESS + CSOUND_MEMORY) ?
              CSOUND_MEMORY : CSOUND_INITIALIZATION);
    }
    /* NOTE: this depends on csound->csmodule_db being the most recently */
    /* loaded plugin library */

    err = csoundInitModule(csound, (csoundModule_t*) csound->csmodule_db);
    memcpy((void*) &csound->exitjmp, (void*) &tmpExitJmp, sizeof(jmp_buf));

    return err;
}

int csoundLoadAndInitModules(CSOUND *csound, const char *opdir)
{
#if (defined(HAVE_DIRENT_H) && (TARGET_OS_IPHONE == 0))
    DIR             *dir;
    struct dirent   *f;
    const char      *dname, *fname;
    char            buf[1024];
    int             i, n, len, err = CSOUND_SUCCESS;
    char            *dname1, *end;
    int             read_directory = 1;
    char sep =
#ifdef _WIN32
    ';';
#else
    ':';
#endif
#ifdef __HAIKU__
        int dfltdir = 0;
#endif
        // VL: check is not wanted
        //if (UNLIKELY(csound->csmodule_db != NULL))
        ///return CSOUND_ERROR;

    /* open plugin directory */
    // EM'2021: This seems to be dead code since opdir will never be NULL and
    // the value of dname will be discarded, see "dname = opdir" later
    dname = csoundGetEnv(csound, (sizeof(MYFLT) == sizeof(float) ?
                                  plugindir_envvar : plugindir64_envvar));
    if (dname == NULL) {
#if ENABLE_OPCODEDIR_WARNINGS
      csound->opcodedirWasOK = 0;
#  ifdef USE_DOUBLE
      dname = csoundGetEnv(csound, plugindir_envvar);
      if (dname == NULL)
#  endif
#endif
#ifdef  CS_DEFAULT_PLUGINDIR
        dname = CS_DEFAULT_PLUGINDIR;
 #ifdef __HAIKU__
                dfltdir = 1;
 #endif
#else
      dname = "";
#endif
    }

    if (opdir != NULL) {
      dname = opdir;
    }

    /* We now loop through the directory list */
    while(read_directory) {
      /* find separator */
    if((end = strchr(dname, sep)) != NULL) {
      *end = '\0';
      /* copy directory name */
      dname1 = cs_strdup(csound, (char *) dname);
      *end = sep;  /* restore for re-execution */
      /* move to next directory name */
      dname = end + 1;
    } else {
      /* copy last directory name) */
      dname1 = cs_strdup(csound, (char *) dname);
      read_directory = 0;
    }

    /* protect for the case where there is an
       extra separator at the end */
    if(*dname1 == '\0') {
      mfree(csound, dname1);
      break;
    }

    dir = opendir(dname1);
    if (UNLIKELY(dir == (DIR*) NULL)) {
 #if defined(__HAIKU__)
        if(!dfltdir)
 #endif
      csoundWarning(csound, Str("Error opening plugin directory '%s': %s"),
                               dname1, strerror(errno));
      mfree(csound, dname1);
      continue;
    }

    if(UNLIKELY(csound->oparms->odebug))
      csoundMessage(csound, "Opening plugin directory: %s\n", dname1);
    /* load database for deferred plugin loading */
/*     n = csoundLoadOpcodeDB(csound, dname); */
/*     if (n != 0) */
/*       return n; */
    /* scan all files in directory */
    while ((f = readdir(dir)) != NULL) {
      fname = &(f->d_name[0]);
      if (UNLIKELY(fname[0]=='_')) continue;
      n = len = (int) strlen(fname);
      if (UNLIKELY(fname[0]=='_')) continue;
#if defined(_WIN32)
      strcpy(buf, "dll");
      n -= 4;
#elif defined(__MACH__)
      strcpy(buf, "dylib");
      n -= 6;
#else
      strcpy(buf, "so");
      n -= 3;
#endif
      if (n <= 0 || fname[n] != '.')
        continue;
      i = 0;
      do {
        if (UNLIKELY((fname[++n] | (char) 0x20) != buf[i]))
          break;
      } while (buf[++i] != '\0');
      if (buf[i] != '\0')
        continue;
      /* found a dynamic library, attempt to open it */
      if (UNLIKELY(((int) strlen(dname) + len + 2) > 1024)) {
        csoundWarning(csound, Str("path name too long, skipping '%s'"),
                                fname);
        continue;
      }
      /* printf("DEBUG %s(%d): possibly deny %s\n", __FILE__, __LINE__,fname); */
      if (UNLIKELY(csoundCheckOpcodeDeny(csound, fname))) {
        csoundWarning(csound, Str("Library %s omitted\n"), fname);
        continue;
      }
      snprintf(buf, 1024, "%s%c%s", dname1, DIRSEP, fname);
      if (UNLIKELY(csound->oparms->odebug)) {
        csoundMessage(csound, Str("Loading '%s'\n"), buf);
       }
      n = csoundLoadAndInitModule(csound, buf);
      if (UNLIKELY(UNLIKELY(n == CSOUND_ERROR)))
        continue;               /* ignore non-plugin files */
      if (UNLIKELY(n < err))
        err = n;                /* record serious errors */
    }
    closedir(dir);
    mfree(csound, dname1);
    }
    return (err == CSOUND_INITIALIZATION ? CSOUND_ERROR : err);
#else
    return CSOUND_SUCCESS;
#endif  /* HAVE_DIRENT_H */
}

/**
 * Call destructor functions of all loaded modules that have a
 * csoundModuleDestroy symbol, for Csound instance 'csound'.
 * Return value is CSOUND_SUCCESS if there was no error, and
 * CSOUND_ERROR if some modules could not be de-initialised.
 */

int csoundDestroyModules(CSOUND *csound)
{
    csoundModule_t  *m;
    int             i, retval;

    retval = CSOUND_SUCCESS;
    while (csound->csmodule_db != NULL) {

      m = (csoundModule_t*) csound->csmodule_db;
      /* call destructor functions */
      if (m->PreInitFunc != NULL && m->fn.p.DestFunc != NULL) {
        i = m->fn.p.DestFunc(csound);
        if (UNLIKELY(i != 0)) {
          print_module_error(csound, Str("Error de-initialising module '%s'"),
                                     &(m->name[0]), m, i);
          retval = CSOUND_ERROR;
        }
      }
      /* unload library */
      csoundCloseLibrary(m->h);
      csound->csmodule_db = (void*) m->nxt;
      /* free memory used by database */
      mfree(csound, (void*) m);

    }
    sfont_ModuleDestroy(csound);
    /* return with error code */
    return retval;
}

#endif /* __wasi__ */

 /* ------------------------------------------------------------------------ */

#if ENABLE_OPCODEDIR_WARNINGS
static const char *opcodedirWarnMsg[] = {
    "################################################################",
#ifndef USE_DOUBLE
    "#               WARNING: OPCODE7DIR IS NOT SET !               #",
#else
    "#              WARNING: OPCODE7DIR64 IS NOT SET !              #",
#endif
    "# Csound requires this environment variable to be set to find  #",
    "# its plugin libraries. If it is not set, you may experience   #",
    "# missing opcodes, audio/MIDI drivers, or utilities.           #",
    "################################################################",
    NULL
};
#endif

void print_opcodedir_warning(CSOUND *p)
{
#if ENABLE_OPCODEDIR_WARNINGS
    if (!p->opcodedirWasOK) {
      const char  **sp;
      for (sp = &(opcodedirWarnMsg[0]); *sp != NULL; sp++)
        csoundMessageS(p, CSOUNDMSG_WARNING, "        %s\n", Str(*sp));
    }
#else
    (void) p;
#endif
}

/**
 Builtin linkage for C opcodes - Instructions:
 - use csoundCore.h instead of csdl.h.
 - name the OENTRY array <name>, where <name> is any arbitrary name.
 - add the macro LINKAGE_BUILTIN(<name>) to the end of the file.
 - append the init function prototype below
   EXTERN_INIT_FUNCTION(<name>_init);
 - append the init function name <name>_init to the
   staticmodules[] array initialisation.
 - insert source code to libcsound_SRCS in../CMakeLists.txt
*/

typedef int32_t (*INITFN)(CSOUND *, OENTRY **);

EXTERN_INIT_FUNCTION(babo_localops_init);
EXTERN_INIT_FUNCTION(bilbar_localops_init);
EXTERN_INIT_FUNCTION(compress_localops_init);
EXTERN_INIT_FUNCTION(pvsbuffer_localops_init);
EXTERN_INIT_FUNCTION(vosim_localops_init);
EXTERN_INIT_FUNCTION(eqfil_localops_init);
EXTERN_INIT_FUNCTION(modal4_localops_init);
EXTERN_INIT_FUNCTION(physmod_localops_init);
EXTERN_INIT_FUNCTION(spectra_localops_init);
EXTERN_INIT_FUNCTION(grain4_localops_init);
EXTERN_INIT_FUNCTION(hrtferX_localops_init);
EXTERN_INIT_FUNCTION(loscilx_localops_init);
EXTERN_INIT_FUNCTION(pan2_localops_init);
EXTERN_INIT_FUNCTION(arrayvars_localops_init);
EXTERN_INIT_FUNCTION(phisem_localops_init);
EXTERN_INIT_FUNCTION(pvoc_localops_init);
EXTERN_INIT_FUNCTION(hrtfopcodes_localops_init);
EXTERN_INIT_FUNCTION(hrtfreverb_localops_init);
EXTERN_INIT_FUNCTION(hrtfearly_localops_init);
EXTERN_INIT_FUNCTION(gendy_localops_init);
EXTERN_INIT_FUNCTION(vbap_localops_init);
EXTERN_INIT_FUNCTION(harmon_localops_init);
EXTERN_INIT_FUNCTION(pitchtrack_localops_init);
EXTERN_INIT_FUNCTION(squinewave_localops_init);

EXTERN_INIT_FUNCTION(partikkel_localops_init);
EXTERN_INIT_FUNCTION(shape_localops_init);
EXTERN_INIT_FUNCTION(tabaudio_localops_init);
EXTERN_INIT_FUNCTION(crossfm_localops_init);
EXTERN_INIT_FUNCTION(pvlock_localops_init);
EXTERN_INIT_FUNCTION(scnoise_localops_init);

EXTERN_INIT_FUNCTION(mp3in_localops_init);

EXTERN_INIT_FUNCTION(afilts_localops_init);
EXTERN_INIT_FUNCTION(pinker_localops_init);
EXTERN_INIT_FUNCTION(paulstretch_localops_init);
EXTERN_INIT_FUNCTION(wpfilters_localops_init);
EXTERN_INIT_FUNCTION(zak_localops_init);
EXTERN_INIT_FUNCTION(lufs_localops_init);
EXTERN_INIT_FUNCTION(sterrain_localops_init);
EXTERN_INIT_FUNCTION(date_localops_init);
EXTERN_INIT_FUNCTION(liveconv_localops_init);
EXTERN_INIT_FUNCTION(gamma_localops_init);
EXTERN_INIT_FUNCTION(framebuffer_localops_init);
EXTERN_INIT_FUNCTION(cell_localops_init);
EXTERN_INIT_FUNCTION(exciter_localops_init);
EXTERN_INIT_FUNCTION(buchla_localops_init);
EXTERN_INIT_FUNCTION(select_localops_init);

EXTERN_INIT_FUNCTION(sequencer_localops_init);
EXTERN_INIT_FUNCTION(scugens_localops_init);
EXTERN_INIT_FUNCTION(emugens_localops_init);

#ifdef HAVE_SOCKETS
EXTERN_INIT_FUNCTION(socksend_localops_init);
EXTERN_INIT_FUNCTION(sockrecv_localops_init);
#endif

#ifdef INIT_STATIC_MODULES
  EXTERN_INIT_FUNCTION(ambicode_localops_init);
  EXTERN_INIT_FUNCTION(ambicode1_localops_init);
  EXTERN_INIT_FUNCTION(buchla_localops_init);
  EXTERN_INIT_FUNCTION(cellular_localops_init);
  EXTERN_INIT_FUNCTION(counter_localops_init);
  #ifndef WIN32
    EXTERN_INIT_FUNCTION(cpumeter_localops_init);
  #endif
  EXTERN_INIT_FUNCTION(cross2_localops_init);
  EXTERN_INIT_FUNCTION(date_localops_init);
  EXTERN_INIT_FUNCTION(dcblockr_localops_init);
  EXTERN_INIT_FUNCTION(exciter_localops_init);
  EXTERN_INIT_FUNCTION(fareyseq_localops_init);
  EXTERN_INIT_FUNCTION(filter_localops_init);
  EXTERN_INIT_FUNCTION(framebuffer_localops_init);
  EXTERN_INIT_FUNCTION(freeverb_localops_init);
  EXTERN_INIT_FUNCTION(ftconv_localops_init);
  EXTERN_INIT_FUNCTION(gammatone_localops_init);
  EXTERN_INIT_FUNCTION(liveconv_localops_init);
  EXTERN_INIT_FUNCTION(lufs_localops_init);
  EXTERN_INIT_FUNCTION(metro_localops_init);
  EXTERN_INIT_FUNCTION(minmax_localops_init);
  EXTERN_INIT_FUNCTION(modmatrix_localops_init);
  EXTERN_INIT_FUNCTION(platerev_localops_init);
  EXTERN_INIT_FUNCTION(pvsgendy_localops_init);
  EXTERN_INIT_FUNCTION(scoreline_localops_init);
  EXTERN_INIT_FUNCTION(select_localops_init);
  EXTERN_INIT_FUNCTION(seqtime_localops_init);
  #ifndef NO_SERIAL_OPCODES                                 
    EXTERN_INIT_FUNCTION(serial_localops_init);
  #endif
  EXTERN_INIT_FUNCTION(sterrain_localops_init);
  EXTERN_INIT_FUNCTION(system_call_localops_init);
  EXTERN_INIT_FUNCTION(tabsum_localops_init);
  EXTERN_INIT_FUNCTION(ugakbari_localops_init);
  EXTERN_INIT_FUNCTION(vaops_localops_init);
  EXTERN_INIT_FUNCTION(wterrain2_localops_init);
#endif

const INITFN staticmodules[] = { hrtfopcodes_localops_init, babo_localops_init,
                                 bilbar_localops_init, vosim_localops_init,
                                 compress_localops_init, pvsbuffer_localops_init,
                                 eqfil_localops_init, modal4_localops_init,
                                 physmod_localops_init,
                                 spectra_localops_init,
                                 grain4_localops_init,
                                 hrtferX_localops_init, loscilx_localops_init,
                                 pan2_localops_init, arrayvars_localops_init,
                                 phisem_localops_init, pvoc_localops_init,
                                 vbap_localops_init,
                                 harmon_localops_init,
                                 pitchtrack_localops_init, partikkel_localops_init,
                                 shape_localops_init,
                                 crossfm_localops_init, pvlock_localops_init,
                                 hrtfearly_localops_init,
                                 hrtfreverb_localops_init,
                                 paulstretch_localops_init,
                                 squinewave_localops_init, tabaudio_localops_init,

#if !(defined(NACL)) && !(defined(__wasi__))
#ifdef HAVE_SOCKETS
                                 sockrecv_localops_init,
                                 socksend_localops_init,
#endif
                                 mp3in_localops_init,
#endif
                                 scnoise_localops_init, afilts_localops_init,
                                 pinker_localops_init, gendy_localops_init,
                                 wpfilters_localops_init, zak_localops_init,
                                 scugens_localops_init,
                                 emugens_localops_init, sequencer_localops_init,
                                 csound_type_system_internal_localops_init,
  #ifdef INIT_STATIC_MODULES
    ambicode_localops_init,
    ambicode1_localops_init,
    buchla_localops_init,
    cellular_localops_init,
    counter_localops_init,
    #ifndef WIN32
    cpumeter_localops_init,
    #endif
    cross2_localops_init,
    date_localops_init,
    dcblockr_localops_init,
    exciter_localops_init,
    fareyseq_localops_init,
    filter_localops_init,
    framebuffer_localops_init,
    freeverb_localops_init,
    ftconv_localops_init,
    gammatone_localops_init,
    liveconv_localops_init,
    lufs_localops_init,
    metro_localops_init,
    minmax_localops_init,
    modmatrix_localops_init,
    platerev_localops_init,
    pvsgendy_localops_init,
    scoreline_localops_init,
    select_localops_init,
    seqtime_localops_init,
    #ifndef NO_SERIAL_OPCODES                                 
          serial_localops_init,
    #endif
    sterrain_localops_init,
    system_call_localops_init,
    tabsum_localops_init,
    ugakbari_localops_init,
    system_call_localops_init,
    vaops_localops_init,
    wterrain2_localops_init,
  #endif
                                 NULL };

/**
 Builtin linkage for C fgens - Instructions:
 - use csoundCore.h instead of csdl.h.
 - name the NGFENS array <name>, where <name> is any arbitrary name.
 - add the macro FLINKAGE_BUILTIN(<name>) to the end of the file.
 - append the init function prototype below
   NGFENS* <name>_init(CSOUND *);
 - append the init function name <name>_init to the
   ftgenab[] array initialisation.
 - insert source code to libcsound_SRCS in../CMakeLists.txt
*/
typedef NGFENS* (*FGINITFN)(CSOUND *);

NGFENS *quadbezier_fgens_init(CSOUND *, void *);
NGFENS *ftest_fgens_init(CSOUND *);
NGFENS *farey_fgens_init(CSOUND *);

const FGINITFN fgentab[] = {  ftest_fgens_init, farey_fgens_init, (void*)quadbezier_fgens_init, NULL };

CS_NOINLINE int csoundInitStaticModules(CSOUND *csound)
{
    int     i;
    OENTRY  *opcodlst_n;
    int32_t length;

    for (i=0; staticmodules[i]!=NULL; i++) {
      length = (staticmodules[i])(csound, &opcodlst_n);

      if (UNLIKELY(length <= 0L)) return CSOUND_ERROR;
      length /= (int32_t) sizeof(OENTRY);
      if (length) {
        if (UNLIKELY(csoundAppendOpcodes(csound, opcodlst_n, length) != 0))
          return CSOUND_ERROR;
      }
    }
    /* stdopc module */
    if (UNLIKELY(stdopc_ModuleInit(csound))) return CSOUND_ERROR;

    /* pvs module */
    if (UNLIKELY(pvsopc_ModuleInit(csound))) return CSOUND_ERROR;

    /* sfont module */
    sfont_ModuleCreate(csound);
    if (UNLIKELY(sfont_ModuleInit(csound))) return CSOUND_ERROR;

    /* newgabopc */
    if (UNLIKELY(newgabopc_ModuleInit(csound))) return CSOUND_ERROR;

    /* modules were initialised successfully */
    /* Now fgens */
    for (i = 0; fgentab[i]!=NULL; i++) {
      int j;
      NGFENS  *names = (fgentab[i])(csound);
      for (j = 0; names[j].name != NULL; j++)
        allocgen(csound, names[j].name, names[j].fn);
    }
    return CSOUND_SUCCESS;
}
