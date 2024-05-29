#include "fgens_public.h"
#include "csoundCore_internal.h"
#include "memalloc.h"
#include "text.h"
#include "fgens.h"
#include "insert_public.h"
#include "rdscor_internal.h"


int isstrcod(MYFLT xx)
{
    int sel = (byte_order()+1)&1;
#ifdef USE_DOUBLE
    union {
      double d;
      int32_t i[2];
    } z;
    z.d = xx;
    return ((z.i[sel]&0x7ff00000)==0x7ff00000);
#else
    union {
      float f;
      int32_t j;
    } z;
    z.f = xx;
    return ((z.j&0x7f800000) == 0x7f800000);
#endif
}


/**
 * Create ftable using evtblk data, and store pointer to new table in *ftpp.
 * If mode is zero, a zero table number is ignored, otherwise a new table
 * number is automatically assigned.
 * Returns zero on success.
 */

int hfgens(CSOUND *csound, FUNC **ftpp, const EVTBLK *evtblkp, int mode)
{
    int32    genum, ltest;
    int     lobits, msg_enabled, i;
    FUNC    *ftp;
    FGDATA  ff;
    int nonpowof2_flag=0; /* gab: fixed for non-powoftwo function tables*/

    *ftpp = NULL;
    if (UNLIKELY(csound->gensub == NULL)) {
      csound->gensub = (GEN*) mmalloc(csound, sizeof(GEN) * (GENMAX + 1));
      memcpy(csound->gensub, or_sub, sizeof(GEN) * (GENMAX + 1));
      csound->genmax = GENMAX + 1;
    }
    msg_enabled = csound->oparms->msglevel & 7;
    memset(&ff, '\0', sizeof(ff)); /* for Valgrind */
    ff.csound = csound;
    memcpy((char*) &(ff.e), (char*) evtblkp,
           (size_t) ((char*) &(evtblkp->p[2]) - (char*) evtblkp));
    ff.fno = (int) MYFLT2LRND(ff.e.p[1]);
    if (!ff.fno) {
      if (!mode)
        return 0;                               /*  fno = 0: return,        */
      ff.fno = FTAB_SEARCH_BASE;
      do {                                      /*      or automatic number */
        ++ff.fno;
      } while (ff.fno <= csound->maxfnum && csound->flist[ff.fno] != NULL);
      ff.e.p[1] = (MYFLT) (ff.fno);
    }
    else if (ff.fno < 0) {                      /*  fno < 0: remove         */
      ff.fno = -(ff.fno);
      if (UNLIKELY(ff.fno > csound->maxfnum ||
                   (ftp = csound->flist[ff.fno]) == NULL)) {
        return fterror(&ff, Str("ftable does not exist"));
      }
      csound->flist[ff.fno] = NULL;
      mfree(csound, (void*) ftp);
      if (UNLIKELY(msg_enabled))
        csoundMessage(csound, Str("ftable %d now deleted\n"), ff.fno);
      return 0;
    }
    if (UNLIKELY(ff.fno > csound->maxfnum)) {   /* extend list if necessary */
      FUNC  **nn;
      int   size;
      for (size = csound->maxfnum; size < ff.fno; size += MAXFNUM)
        ;
      nn = (FUNC**) mrealloc(csound,
                                    csound->flist, (size + 1) * sizeof(FUNC*));
      csound->flist = nn;
      for (i = csound->maxfnum + 1; i <= size; i++)
        csound->flist[i] = NULL;                /*  Clear new section       */
      csound->maxfnum = size;
    }
    if (UNLIKELY(ff.e.pcnt <= 4)) {             /*  chk minimum arg count   */
      return fterror(&ff, Str("insufficient gen arguments"));
    }
    if (UNLIKELY(ff.e.pcnt>PMAX)) {
      //#ifdef BETA
      csoundDebugMsg(csound, "T%d/%d(%d): x=%p memcpy from %p to %p length %zu\n",
              (int)evtblkp->p[1], (int)evtblkp->p[4], ff.e.pcnt, evtblkp->c.extra,
              &(ff.e.p[2]), &(evtblkp->p[2]), sizeof(MYFLT) * PMAX);
      //#endif
      memcpy(&(ff.e.p[2]), &(evtblkp->p[2]), sizeof(MYFLT) * (PMAX-2));
      ff.e.c.extra =
        (MYFLT*)mmalloc(csound,sizeof(MYFLT) * (evtblkp->c.extra[0]+1));
      memcpy(ff.e.c.extra, evtblkp->c.extra,
             sizeof(MYFLT) * (evtblkp->c.extra[0]+1));
    }
    else
      memcpy(&(ff.e.p[2]), &(evtblkp->p[2]),
             sizeof(MYFLT) * ((int) ff.e.pcnt - 1));
    if (isstrcod(ff.e.p[4])) {
      /* A named gen given so search the list of extra gens */
      NAMEDGEN *n = (NAMEDGEN*) csound->namedgen;
      while (n) {
        if (strcmp(n->name, ff.e.strarg) == 0) {    /* Look up by name */
          ff.e.p[4] = genum = n->genum;
          break;
        }
        n = n->next;                            /*  and round again         */
      }
      if (UNLIKELY(n == NULL)) {
        return fterror(&ff, Str("Named gen \"%s\" not defined"), ff.e.strarg);
      }
    }
    else {
      genum = (int32) MYFLT2LRND(ff.e.p[4]);
      if (genum < 0)
        genum = -genum;
      if (UNLIKELY(!genum || genum > csound->genmax)) { /*   & legal gen number x*/
        return fterror(&ff, Str("illegal gen number"));
      }
    }
    ff.flen = (int32) MYFLT2LRND(ff.e.p[3]);
    if (!ff.flen) {
      /* defer alloc to gen01|gen23|gen28 */
      ff.guardreq = 1;
      if (UNLIKELY(genum != 1 && genum != 2 && genum != 23 &&
                   genum != 28 && genum != 44 && genum != 49 && genum<=GENMAX)) {
        return fterror(&ff, Str("deferred size for GENs 1, 2, 23, 28 or 49 only"));
      }
      if (UNLIKELY(msg_enabled))
        csoundMessage(csound, Str("ftable %d:\n"), ff.fno);
      i = (*csound->gensub[genum])(&ff, NULL);
      ftp = csound->flist[ff.fno];
      if (i != 0) {
        csound->flist[ff.fno] = NULL;
        mfree(csound, ftp);
        return -1;
      }
      *ftpp = ftp;
      return 0;
    }
    /* if user flen given */
    if (ff.flen < 0L || !isPowerOfTwo(ff.flen&~1)) {
      /* gab for non-pow-of-two-length    */
      ff.guardreq = 1;
      if (ff.flen<0) ff.flen = -(ff.flen);             /* gab: fixed */
      if (!(ff.flen & (ff.flen - 1L)) || ff.flen > MAXLEN)
        goto powOfTwoLen;
      lobits = 0;                       /* Hope this is not needed! */
      nonpowof2_flag = 1; /* gab: fixed for non-powoftwo function tables*/
    }
    else {
      ff.guardreq = ff.flen & 01;       /*  set guard request flg   */
      ff.flen &= -2L;                   /*  flen now w/o guardpt    */
 powOfTwoLen:
      if (UNLIKELY(ff.flen <= 0L || ff.flen > MAXLEN)) {
        return fterror(&ff, Str("illegal table length"));
      }
      for (ltest = ff.flen, lobits = 0;
           (ltest & MAXLEN) == 0L;
           lobits++, ltest <<= 1)
        ;
      if (UNLIKELY(ltest != MAXLEN)) {  /*  flen is not power-of-2 */
        // return fterror(&ff, Str("illegal table length"));
        //csoundWarning(csound, Str("table %d size not power of two"), ff.fno);
        lobits = 0;
        nonpowof2_flag = 1;
        ff.guardreq = 1;
      }
    }
    ftp = ftalloc(&ff);                 /*  alloc ftable space now  */
    ftp->lenmask  = ((ff.flen & (ff.flen - 1L)) ?
                     0L : (ff.flen - 1L));      /*  init hdr w powof2 data  */
    ftp->lobits   = lobits;
    i = (1 << lobits);
    ftp->lomask   = (int32) (i - 1);
    ftp->lodiv    = FL(1.0) / (MYFLT) i;        /*    & other useful vals   */
    ftp->nchanls  = 1;                          /*    presume mono for now  */
    ftp->gen01args.sample_rate = csound->esr;  /* set table SR to esr */
    ftp->flenfrms = ff.flen;
    if (nonpowof2_flag)
      ftp->lenmask = 0xFFFFFFFF; /* gab: fixed for non-powoftwo function tables */

    if (UNLIKELY(msg_enabled))
      csoundMessage(csound, Str("ftable %d:\n"), ff.fno);
    if ((*csound->gensub[genum])(&ff, ftp) != 0) {
      csound->flist[ff.fno] = NULL;
      mfree(csound, ftp);
      return -1;
    }
    /* VL 11.01.05 for deferred GEN01, it's called in gen01raw */
    ftresdisp(&ff, ftp);                        /* rescale and display      */
    *ftpp = ftp;
    /* keep original arguments, from GEN number  */
    ftp->argcnt = ff.e.pcnt - 3;
    {  /* Note this does not handle extended args -- JPff */
      int size=ftp->argcnt;
      if (UNLIKELY(size>PMAX-4)) size=PMAX-4;
      /* printf("size = %d -> %d ftp->args = %p\n", */
      /*        size, sizeof(MYFLT)*size, ftp->args); */
      memcpy(ftp->args, &(ff.e.p[4]), sizeof(MYFLT)*size); /* is this right? */
      /*for (k=0; k < size; k++)
        csoundMessage(csound, "%f\n", ftp->args[k]);*/
    }
    return 0;
}

/**
 * Allocates space for 'tableNum' with a length (not including the guard
 * point) of 'len' samples. The table data is not cleared to zero.
 * Return value is zero on success.
 */

int csoundFTAlloc(CSOUND *csound, int tableNum, int len)
{
    int   i, size;
    FUNC  **nn, *ftp;

    if (UNLIKELY(tableNum <= 0 || len <= 0 || len > (int) MAXLEN))
      return -1;
    if (UNLIKELY(tableNum > csound->maxfnum)) { /* extend list if necessary     */
      for (size = csound->maxfnum; size < tableNum; size += MAXFNUM)
        ;
      nn = (FUNC**) mrealloc(csound,
                                    csound->flist, (size + 1) * sizeof(FUNC*));
      csound->flist = nn;
      for (i = csound->maxfnum + 1; i <= size; i++)
        csound->flist[i] = NULL;            /* Clear new section            */
      csound->maxfnum = size;
    }
    /* allocate space for table */
    size = (int) (len * (int) sizeof(MYFLT));
    ftp = csound->flist[tableNum];
    if (ftp == NULL) {
      csound->flist[tableNum] = (FUNC*) mmalloc(csound, sizeof(FUNC));
      csound->flist[tableNum]->ftable =
        (MYFLT*)mmalloc(csound, sizeof(MYFLT)*(len+1));
    }
    else if (len != (int) ftp->flen) {
      if (UNLIKELY(csound->actanchor.nxtact != NULL)) { /*   & chk for danger    */
        /* return */  /* VL: changed this into a Warning */
          csoundWarning(csound, Str("ftable %d relocating due to size change"
                                        "\n         currently active instruments "
                                        "may find this disturbing"), tableNum);
      }
      csound->flist[tableNum] = NULL;
      mfree(csound, ftp);
      csound->flist[tableNum] = (FUNC*) mmalloc(csound, (size_t) size);
      csound->flist[tableNum]->ftable =
        (MYFLT*)mmalloc(csound, sizeof(MYFLT)*(len+1));
    }
    /* initialise table header */
    ftp = csound->flist[tableNum];
    //memset((void*) ftp, 0, (size_t) ((char*) &(ftp->ftable) - (char*) ftp));
    ftp->flen = (int32) len;
    if (!(len & (len - 1))) {
      /* for power of two length: */
      ftp->lenmask = (int32) (len - 1);
      for (i = len, ftp->lobits = 0L; i < (int) MAXLEN; ftp->lobits++, i <<= 1)
        ;
      i = (int) MAXLEN / len;
      ftp->lomask = (int32) (i - 1);
      ftp->lodiv = FL(1.0) / (MYFLT) i;
    }
    ftp->flenfrms = (int32) len;
    ftp->nchanls = 1L;
    ftp->fno = (int32) tableNum;
    return 0;
}

/**
 * Deletes a function table.
 * Return value is zero on success.
 */

int csoundFTDelete(CSOUND *csound, int tableNum)
{
    FUNC  *ftp;

    if (UNLIKELY((unsigned int) (tableNum - 1) >= (unsigned int) csound->maxfnum))
      return -1;
    ftp = csound->flist[tableNum];
    if (UNLIKELY(ftp == NULL))
      return -1;
    csound->flist[tableNum] = NULL;
    mfree(csound, ftp);

    return 0;
}


CS_NOINLINE int fterror(const FGDATA *ff, const char *s, ...)
{
    CSOUND  *csound = ff->csound;
    char    buf[64];
    va_list args;

    snprintf(buf, 64, Str("ftable %d: "), ff->fno);
    va_start(args, s);
    csoundErrMsgV(csound, buf, s, args);
    va_end(args);
    csoundMessage(csound, "f%3.0f %8.2f %8.2f ",
                            ff->e.p[1], ff->e.p2orig, ff->e.p3orig);
    if (isstrcod(ff->e.p[4]))
      csoundMessage(csound,"%s ", ff->e.strarg);
    else
      csoundMessage(csound, "%8.2f", ff->e.p[4]);
    if (isstrcod(ff->e.p[5]))
      csoundMessage(csound, "  \"%s\" ...\n", ff->e.strarg);
    else
      csoundMessage(csound, "%8.2f ...\n", ff->e.p[5]);

    return -1;
}

/* find the ptr to an existing ftable structure */
/*   called by oscils, etc at init time         */

FUNC *csoundFTFind(CSOUND *csound, MYFLT *argp)
{
    FUNC    *ftp;
    int     fno;

    fno = MYFLT2LONG(*argp);
    if (UNLIKELY(fno == -1)) {
      if (UNLIKELY(csound->sinetable==NULL)) generate_sine_tab(csound);
      return csound->sinetable;
    }
    if (UNLIKELY(fno <= 0                    ||
                 fno > csound->maxfnum       ||
                 (ftp = csound->flist[fno]) == NULL)) {
      csoundInitError(csound, Str("Invalid ftable no. %f"), *argp);
      return NULL;
    }
    else if (UNLIKELY(ftp->lenmask == -1)) {
      csoundInitError(csound, Str("illegal table length"));
      return NULL;
    }
    else if (UNLIKELY(!ftp->lenmask)) {
      csoundInitError(csound,
                      Str("deferred-size ftable %f illegal here"), *argp);
      return NULL;
    }
    return ftp;
}


PUBLIC int csoundGetTable(CSOUND *csound, MYFLT **tablePtr, int tableNum)
{
    FUNC    *ftp;

    if (UNLIKELY((unsigned int) (tableNum - 1) >= (unsigned int) csound->maxfnum))
      goto err_return;
    ftp = csound->flist[tableNum];
    if (UNLIKELY(ftp == NULL))
      goto err_return;
    if (!ftp->flen) {
      ftp = gen01_defer_load(csound, tableNum);
      if (UNLIKELY(ftp == NULL))
        goto err_return;
    }
    *tablePtr = ftp->ftable;
    return (int) ftp->flen;
 err_return:
    *tablePtr = (MYFLT*) NULL;
    return -1;
}



PUBLIC int csoundGetTableArgs(CSOUND *csound, MYFLT **argsPtr, int tableNum)
{
    FUNC    *ftp;
    if (UNLIKELY((unsigned int) (tableNum - 1) >= (unsigned int) csound->maxfnum))
      goto err_return;
    ftp = csound->flist[tableNum];
    if (UNLIKELY(ftp == NULL))
      goto err_return;
    *argsPtr = ftp->args;
    return (int) ftp->argcnt;

 err_return:
    *argsPtr = (MYFLT*) NULL;
    return -1;
}

/**************************************
 * csoundFTFindP()
 *
 * New function to find a function table at performance time.  Based
 * on csoundFTFind() which is intended to run at init time only.
 *
 * This function can be called from other modules - such as ugrw1.c.
 *
 * It returns a pointer to a FUNC data structure which contains all
 * the details of the desired table.  0 is returned if it cannot be
 * found.
 *
 * This does not handle deferred function table loads (gen01).
 *
 * Maybe this could be achieved, but some exploration would be
 * required to see that this is feasible at performance time.
 */
FUNC *csoundFTFindP(CSOUND *csound, MYFLT *argp)
{
    FUNC    *ftp;
    int     fno;

    /* Check limits, and then index  directly into the flist[] which
     * contains pointers to FUNC data structures for each table.
     */
    fno = MYFLT2LONG(*argp);
    if (UNLIKELY(fno == -1)) {
      if (UNLIKELY(csound->sinetable==NULL)) generate_sine_tab(csound);
      return csound->sinetable;
    }
    if (UNLIKELY(fno <= 0                 ||
                 fno > csound->maxfnum    ||
                 (ftp = csound->flist[fno]) == NULL)) {
      csoundErrorMsg(csound, Str("Invalid ftable no. %f"), *argp);
      return NULL;
    }
    else if (UNLIKELY(!ftp->lenmask)) {
      /* Now check that the table has a length > 0.  This should only
       * occur for tables which have not been loaded yet.  */
      csoundErrorMsg(csound, Str("Deferred-size ftable %f load "
                                  "not available at perf time."), *argp);
      return NULL;
    }
    return ftp;
}


FUNC *csoundFTnp2Find(CSOUND *csound, MYFLT *argp)
{
    return csoundFTnp2Findint(csound, argp, 0);
}
FUNC *csoundFTnp2Finde(CSOUND *csound, MYFLT *argp)
{
    return csoundFTnp2Findint(csound, argp, 1);
}

int csoundIsNamedGEN(CSOUND *csound, int num) {
    NAMEDGEN *n = (NAMEDGEN*) csound->namedgen;
    while (n != NULL) {
      if (n->genum == abs(num))
        return strlen(n->name);
      n = n->next;
    }
    return 0;
}

/* ODDITY:  does not stop when num found but continues to end; also not used!
   But has API use
 */
void csoundGetNamedGEN(CSOUND *csound, int num, char *name, int len) {
    NAMEDGEN *n = (NAMEDGEN*) csound->namedgen;
    while (n != NULL) {
      if (n->genum == abs(num)) {
        strNcpy(name,n->name,len+1);
        return;
      }
      n = n->next;
    }
}
