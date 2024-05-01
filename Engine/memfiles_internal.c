#include "memfiles_internal.h"
#include "memfiles.h"
#include "csoundCore_internal.h"
#include "memalloc.h"

/* clear the memfile array, & free all allocated space */

void rlsmemfiles(CSOUND *csound)
{
    MEMFIL  *mfp = csound->memfiles, *nxt;

    while (mfp != NULL) {
      nxt = mfp->next;
      mfree(csound, mfp->beginp);       /*   free the space */
      mfree(csound, mfp);
      mfp = nxt;
    }
    csound->memfiles = NULL;
}


int delete_memfile(CSOUND *csound, const char *filnam)
{
    MEMFIL  *mfp, *prv;

    prv = NULL;
    mfp = csound->memfiles;
    while (mfp != NULL) {
      if (strcmp(mfp->filename, filnam) == 0)
        break;
      prv = mfp;
      mfp = mfp->next;
    }
    if (mfp == NULL)
      return -1;
    if (prv == NULL)
      csound->memfiles = mfp->next;
    else
      prv->next = mfp->next;
    mfree(csound, mfp->beginp);
    mfree(csound, mfp);
    return 0;
}