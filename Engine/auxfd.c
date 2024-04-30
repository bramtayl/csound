/*
  auxfd.c:

  Copyright (C) 1991 Barry Vercoe, John ffitch

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

#include "auxfd.h"

#include "auxfd_internal.h"

#include "csoundCore_internal.h"                         /*      AUXFD.C         */
#include "memalloc.h"
#include "envvar_public.h"
#include "text.h"

/* allocate an auxds, or expand an old one */
/*    call only from init (xxxset) modules */

void csoundAuxAlloc(CSOUND *csound, size_t nbytes, AUXCH *auxchp)
{
    if (auxchp->auxp != NULL) {
      /* if allocd with same size, just clear to zero */
      if (nbytes == (size_t)auxchp->size) {
        memset(auxchp->auxp, 0, nbytes);
        return;
      }
      else {
        void  *tmp = auxchp->auxp;
        /* if size change only, free the old space and re-allocate */
        auxchp->auxp = NULL;
        mfree(csound, tmp);
      }
    }
    else {                                  /* else link in new auxch blk */
      auxchp->nxtchp = csound->curip->auxchp;
      csound->curip->auxchp = auxchp;
    }
    /* now alloc the space and update the internal data */
    auxchp->size = nbytes;
    auxchp->auxp = mcalloc(csound, nbytes);
    auxchp->endp = (char*)auxchp->auxp + nbytes;
    if (UNLIKELY(csound->oparms->odebug))
      auxchprint(csound, csound->curip);
}


static uintptr_t alloc_thread(void *p) {
    AUXASYNC *pp = (AUXASYNC *) p;
    CSOUND *csound = pp->csound;
    AUXCH newm;
    char *ptr;
    if (pp->auxchp->auxp == NULL) {
      /* Allocate new memory */
      newm.size = pp->nbytes;
      newm.auxp = mcalloc(csound, pp->nbytes);
      newm.endp = (char*) newm.auxp + pp->nbytes;
      ptr = (char *) newm.auxp;
      newm  = *(pp->notify(csound, pp->userData, &newm));
      /* check that the returned pointer is not
         NULL and that is not the memory we have
         just allocated in case the old memory was
         never swapped back.
      */
      if (newm.auxp != NULL && newm.auxp != ptr)
        mfree(csound, newm.auxp);
    } else {
      csoundAuxAlloc(csound,pp->nbytes,pp->auxchp);
      pp->notify(csound, pp->userData, pp->auxchp);
    }
    return 0;
}



/* Allocate an auxds asynchronously and
   pass the newly allocated memory via a
   callback, where it can be swapped if necessary.
*/
int csoundAuxAllocAsync(CSOUND *csound, size_t nbytes, AUXCH *auxchp,
                        AUXASYNC *as, aux_cb cb, void *userData) {
    as->csound = csound;
    as->nbytes = nbytes;
    as->auxchp = auxchp;
    as->notify = cb;
    as->userData = userData;
    if (UNLIKELY(csoundCreateThread(alloc_thread, as) == NULL))
      return CSOUND_ERROR;
    else
      return CSOUND_SUCCESS;
}


/* put fdchp into chain of fd's for this instr */
/*      call only from init (xxxset) modules   */

void fdrecord(CSOUND *csound, FDCH *fdchp)
{
    fdchp->nxtchp = csound->curip->fdchp;
    csound->curip->fdchp = fdchp;
    if (UNLIKELY(csound->oparms->odebug))
      fdchprint(csound, csound->curip);
}

/* close a file and remove from fd chain */
/*  call only from inits, after fdrecord */

void csound_fd_close(CSOUND *csound, FDCH *fdchp)
{
    FDCH    *prvchp = NULL, *nxtchp;

    nxtchp = csound->curip->fdchp;              /* from current insds,  */
    while (LIKELY(nxtchp != NULL)) {            /* chain through fdlocs */
      if (UNLIKELY(nxtchp == fdchp)) {          /*   till find this one */
        void  *fd = fdchp->fd;
        if (LIKELY(fd)) {
          fdchp->fd = NULL;                     /* then delete the fd   */
          csoundFileClose(csound, fd);          /*   close the file &   */
        }
        if (prvchp)
          prvchp->nxtchp = fdchp->nxtchp;       /* unlnk from fdchain   */
        else
          csound->curip->fdchp = fdchp->nxtchp;
        if (UNLIKELY(csound->oparms->odebug))
          fdchprint(csound, csound->curip);
        return;
      }
      prvchp = nxtchp;
      nxtchp = nxtchp->nxtchp;
    }
    fdchprint(csound, csound->curip);
    csoundDie(csound, Str("csound_fd_close: no record of fd %p"), fdchp->fd);
}
