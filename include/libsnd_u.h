#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

/* Returns a string name for the file type */
PUBLIC char *type2string(int type);

PUBLIC void *SAsndgetset(CSOUND *, char *, void *, MYFLT *, MYFLT *, MYFLT *,
                         int);

PUBLIC void *sndgetset(CSOUND *, void *);

PUBLIC int getsndin(CSOUND *, void *, MYFLT *, int, void *);

PUBLIC void rewriteheader(void *ofd);

PUBLIC int sftype2csfiletype(int type);

PUBLIC char *getstrformat(int format);

PUBLIC int sfsampsize(int sf_format);

PUBLIC int type2csfiletype(int type, int encoding);

#ifdef __cplusplus
}
#endif /* __cplusplus */