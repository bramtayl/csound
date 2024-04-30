#include "auxfd_internal.h"

#include <string.h>               // for NULL, memset

#include "csoundCore_internal.h"  // for CSOUND_
#include "envvar_public.h"        // for csoundFileClose
#include "memalloc.h"             // for mfree
#include "text.h"                 // for Str

/* release all xds in instr auxp chain */
/*   called by insert at orcompact     */

void auxchfree(CSOUND *csound, INSDS *ip) {
  if (UNLIKELY(csound->oparms->odebug))
    auxchprint(csound, ip);
  while (LIKELY(ip->auxchp != NULL)) { /* for all auxp's in chain: */
    void *auxp = (void *)ip->auxchp->auxp;
    AUXCH *nxt = ip->auxchp->nxtchp;
    memset((void *)ip->auxchp, 0, sizeof(AUXCH)); /*  delete the pntr     */
    mfree(csound, auxp);                          /*  & free the space    */
    ip->auxchp = nxt;
  }
  if (UNLIKELY(csound->oparms->odebug))
    auxchprint(csound, ip);
}

/* close all files in instr fd chain        */
/* called by insert on deact & expire       */
/* (also musmon on s-code, & fgens for gen01) */

void fdchclose(CSOUND *csound, INSDS *ip) {
  if (UNLIKELY(csound->oparms->odebug))
    fdchprint(csound, ip);
  /* for all fd's in chain: */
  for (; ip->fdchp != NULL; ip->fdchp = ip->fdchp->nxtchp) {
    void *fd = ip->fdchp->fd;
    if (LIKELY(fd)) {
      ip->fdchp->fd = NULL;        /*    delete the fd     */
      csoundFileClose(csound, fd); /*    & close the file  */
    }
  }
  if (UNLIKELY(csound->oparms->odebug))
    fdchprint(csound, ip);
}

/* print the xp chain for this insds blk */

CS_NOINLINE void auxchprint(CSOUND *csound, INSDS *ip) {
  AUXCH *curchp;
  char *name = csound->engineState.instrtxtp[ip->insno]->insname;

  if (name)
    csoundMessage(csound, Str("auxlist for instr %s [%d] (%p):\n"), name,
                  ip->insno, ip);
  else
    csoundMessage(csound, Str("auxlist for instr %d (%p):\n"), ip->insno, ip);
  /* chain through auxlocs */
  for (curchp = ip->auxchp; curchp != NULL; curchp = curchp->nxtchp)
    csoundMessage(csound, Str("\tauxch at %p: size %zu, auxp %p, endp %p\n"),
                  curchp, curchp->size, curchp->auxp, curchp->endp);
}

/* print the fd chain for this insds blk */

CS_NOINLINE void fdchprint(CSOUND *csound, INSDS *ip) {
  FDCH *curchp;
  char *name = csound->engineState.instrtxtp[ip->insno]->insname;

  if (name)
    csoundMessage(csound, Str("fdlist for instr %s [%d] (%p):"), name,
                  ip->insno, ip);
  else
    csoundMessage(csound, Str("fdlist for instr %d (%p):"), ip->insno, ip);
  /* chain through fdlocs */
  for (curchp = ip->fdchp; curchp != NULL; curchp = curchp->nxtchp)
    csoundMessage(csound, Str("  fd %p in %p"), curchp->fd, curchp);
  csoundMessage(csound, "\n");
}
