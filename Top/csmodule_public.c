#include "csmodule_public.h"
#include "csound.h"

#if defined(_WIN32)

PUBLIC int csoundOpenLibrary(void **library, const char *libraryPath) {
  *library = (void *)LoadLibrary(libraryPath);
  return (*library != NULL ? 0 : -1);
}

PUBLIC int csoundCloseLibrary(void *library) {
  return (int)(FreeLibrary((HMODULE)library) == FALSE ? -1 : 0);
}

PUBLIC void *csoundGetLibrarySymbol(void *library, const char *procedureName) {
  return (void *)GetProcAddress((HMODULE)library, procedureName);
}

#elif !(defined(NACL)) && !(defined(__wasi__)) &&                              \
    (defined(__gnu_linux__) || defined(NEW_MACH_CODE) || defined(__HAIKU__))

#include <dlfcn.h>

PUBLIC int csoundOpenLibrary(void **library, const char *libraryPath) {
  int flg = RTLD_NOW;
  if (libraryPath != NULL) {
    int len = (int)strlen(libraryPath);
    /* ugly hack to fix importing modules in Python opcodes */
    if (len >= 9 && strcmp(&(libraryPath[len - 9]), "/libpy.so") == 0)
      flg |= RTLD_GLOBAL;
    if (len >= 12 && strcmp(&(libraryPath[len - 12]), "/libpy.dylib") == 0)
      flg |= RTLD_GLOBAL;
  }
  *library = (void *)dlopen(libraryPath, flg);
  return (*library != NULL ? 0 : -1);
}

PUBLIC int csoundCloseLibrary(void *library) { return (int)dlclose(library); }

PUBLIC void *csoundGetLibrarySymbol(void *library, const char *procedureName) {
  return (void *)dlsym(library, procedureName);
}

#else /* case for platforms without shared libraries -- added 062404, akozar   \
       */

int csoundOpenLibrary(void **library, const char *libraryPath) {
  *library = NULL;
  return -1;
}

int csoundCloseLibrary(void *library) { return 0; }

void *csoundGetLibrarySymbol(void *library, const char *procedureName) {
  return NULL;
}

#endif