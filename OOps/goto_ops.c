/*
  goto_ops.c:

  Copyright (C) 1991, 1997, 1999, 2002, 2005
  Barry Vercoe, Istvan Varga, John ffitch,
  Gabriel Maldonado, matt ingalls

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

#include "csoundCore_internal.h" /*                            GOTO_OPS.C        */
#include "insert.h"     /* for goto's */
#include "aops.h"       /* for cond's */
#include "rdscor.h"
#include "namedins_public.h"
#include "fgens_public.h"
#include "insert_public.h"
#include "text.h"
#include "goto_ops.h"
#include "musmon_internal.h"
#include "interlocks.h"

int32_t igoto(CSOUND *csound, GOTO *p)
{
  csound->ids = p->lblblk->prvi;
  return OK;
}


int32_t kgoto(CSOUND *csound, GOTO *p)
{
  IGN(csound);
  CS_PDS = p->lblblk->prvp;
  /* VL 16.2.23 fix for UDOs where the
     label gets confused */
  if(CS_PDS->insdshead == NULL) 
    CS_PDS->insdshead =  p->h.insdshead;
  return OK;
}

int32_t icgoto(CSOUND *csound, CGOTO *p)
{
  if (*p->cond)
    csound->ids = p->lblblk->prvi;
  return OK;
}

int32_t kcgoto(CSOUND *csound, CGOTO *p)
{
  IGN(csound);
  if (*p->cond)
    CS_PDS = p->lblblk->prvp;

  return OK;
}

/* an 'if-then' variant of 'if-goto' */
int32_t ingoto(CSOUND *csound, CGOTO *p)
{
    /* Make sure we have an i-time conditional */
    if (csoundGetTypeForArg(p->cond) == &CS_VAR_TYPE_b && !*p->cond)
      csound->ids = p->lblblk->prvi;
    return OK;
}

int32_t kngoto(CSOUND *csound, CGOTO *p)
{
  IGN(csound);
  if (!*p->cond)
    CS_PDS = p->lblblk->prvp;
  return OK;
}

int32_t timset(CSOUND *csound, TIMOUT *p)
{
  if (UNLIKELY((p->cnt1 = (int32_t)(*p->idel * CS_EKR + FL(0.5))) < 0L ||
               (p->cnt2 = (int32_t)(*p->idur * CS_EKR + FL(0.5))) < 0L))
    return csoundInitError(csound, Str("negative time period"));
  return OK;
}

int32_t timout(CSOUND *csound, TIMOUT *p)
{
  IGN(csound);
  if (p->cnt1)                            /* once delay has expired, */
    p->cnt1--;
  else if (--p->cnt2 >= 0L)               /*  br during idur countdown */
    CS_PDS = p->lblblk->prvp;
  return OK;
}

int32_t rireturn(CSOUND *csound, LINK *p)
{
  IGN(p);
  IGN(csound);
  return OK;
}

int32_t reinit(CSOUND *csound, GOTO *p)
{
  csound->reinitflag = p->h.insdshead->reinitflag = 1;
  if (csound->oparms->realtime == 0) {
    csound->curip = p->h.insdshead;
    csound->ids = p->lblblk->prvi;        /* now, despite ANSI C warning:  */
    while ((csound->ids = csound->ids->nxti) != NULL &&
           (csound->ids->iopadr != (SUBR) rireturn))
      (*csound->ids->iopadr)(csound, csound->ids);
    csound->reinitflag = p->h.insdshead->reinitflag = 0;
  }
  else {
    uint64_t wp = csound->alloc_queue_wp;
    ATOMIC_SET(p->h.insdshead->init_done, 0);
    ATOMIC_SET8(p->h.insdshead->actflg, 0);
    csound->alloc_queue[wp].ip = p->h.insdshead;
    csound->alloc_queue[wp].ids = p->lblblk->prvi;
    csound->alloc_queue[wp].type = 3;
    csound->alloc_queue_wp = wp + 1 < MAX_ALLOC_QUEUE ? wp + 1 : 0;
    ATOMIC_INCR(csound->alloc_queue_items);
    return NOTOK;
  }
  return OK;
}

int32_t rigoto(CSOUND *csound, GOTO *p)
{
  if (p->h.insdshead->reinitflag)
    csound->ids = p->lblblk->prvi;
  return OK;
}

int32_t tigoto(CSOUND *csound, GOTO *p)     /* I-time only, NOP at reinit */
{
  if (p->h.insdshead->tieflag && !p->h.insdshead->reinitflag)
    csound->ids = p->lblblk->prvi;
  return OK;
}

int32_t tival(CSOUND *csound, EVAL *p)      /* I-time only, NOP at reinit */
{
  IGN(csound);
  if (!p->h.insdshead->reinitflag)
    *p->r = p->h.insdshead->tieflag;
  /* *p->r = (csound->tieflag ? FL(1.0) : FL(0.0)); */
  return OK;
}

int32_t ihold(CSOUND *csound, LINK *p)  /* make this note indefinit duration */
{                                       /* called by ihold statmnt at Itime  */
  IGN(csound);
  if (!p->h.insdshead->reinitflag) {  /* no-op at reinit                   */
    csound->curip->offbet = -1.0;
    csound->curip->offtim = -1.0;
  }
  return OK;
}

int32_t turnoff(CSOUND *csound, LINK *p)/* terminate the current instrument  */
{                                       /* called by turnoff statmt at Ptime */
  IGN(csound);
  INSDS *current = p->h.insdshead;
  INSDS *lcurip = p->h.insdshead;
  if (lcurip->actflg) {
    /* IV - Oct 16 2002: check for subinstr and user opcode */
    /* find top level instrument instance */
    while (lcurip->opcod_iobufs)
      lcurip = ((OPCOD_IOBUFS*) lcurip->opcod_iobufs)->parent_ip;
    xturnoff(csound, lcurip);
    if (current->xtratim <= 0) {
      OPDS* curOp = current->pds;
      while (curOp->nxtp != NULL)
        curOp = curOp->nxtp;                /* loop to last opds */
      current->pds = curOp;
    }
  }
  return OK;
}

/* turnoff2 opcode */
int32_t turnoff2(CSOUND *csound, TURNOFF2 *p, int32_t isStringArg)
{
  MYFLT p1;                     /* Shoud e a float */
  INSDS *ip, *ip2, *nip;
  int32_t   mode, insno, allow_release;

  if (isStringArg) {
    p1 = (MYFLT) strarg2insno(csound, ((STRINGDAT *)p->kInsNo)->data, 1);
  }
  else if (isstrcod(*p->kInsNo)) {
    p1 = (MYFLT) strarg2insno(csound, get_arg_string(csound, *p->kInsNo), 1);
  }
  else p1 = *(p->kInsNo);

  if (p1 <= FL(0.0))
    return OK;    /* not triggered */

  insno = (int32_t) p1;
  if (UNLIKELY(insno < 1 || insno > (int32_t) csound->engineState.maxinsno ||
               csound->engineState.instrtxtp[insno] == NULL)) {
    if(p->h.iopadr == NULL)
      return csoundPerfError(csound, &(p->h),
                             Str("turnoff2: invalid instrument number"));
    else return csoundInitError(csound,
                                Str("turnoff2: invalid instrument number"));

  }
  mode = (int32_t) (*(p->kFlags) + FL(0.5));
  allow_release = (*(p->kRelease) == FL(0.0) ? 0 : 1);
  if (UNLIKELY(mode < 0 || mode > 15 || (mode & 3) == 3)) {
    if(p->h.iopadr == NULL)
      return csoundPerfError(csound, &(p->h),
                             Str("turnoff2: invalid mode parameter"));
    else csoundInitError(csound,
                         Str("turnoff2: invalid mode parameter"));
  }
  ip = &(csound->actanchor);
  ip2 = NULL;
  /*     if ((mode & 4) && !ip->p1){ */
  /*       return csoundPerfError(csound, &(p->h), */
  /*                              Str("turnoff2: invalid instrument number")); */
  /*     }   */
  while ((ip = ip->nxtact) != NULL && (int32_t) ip->insno != insno);
  if (ip == NULL)
    return OK;
  do {                        /* This loop does not terminate in mode=0 */
    nip = ip->nxtact;
    if (((mode & 8) && ip->offtim >= 0.0) ||
        ((mode & 4) && ip->p1.value != p1) ||
        (allow_release && ip->relesing)) {
      ip = nip;
      continue;
    }
    if (!(mode & 3)) {
      if (allow_release) {
        xturnoff(csound, ip);
      }
      else {
        nip = ip->nxtact;
        xturnoff_now(csound, ip);
      }
    }
    else {
      ip2 = ip;
      if ((mode & 3) == 1)
        break;
    }
    ip = nip;
  } while (ip != NULL && (int32_t) ip->insno == insno);
  if (ip2 != NULL) {
    if (allow_release) {
      xturnoff(csound, ip2);
    }
    else {
      xturnoff_now(csound, ip2);
    }



    if (!p->h.insdshead->actflg) {  /* if current note was deactivated: */
      while (CS_PDS->nxtp != NULL)
        CS_PDS = CS_PDS->nxtp;            /* loop to last opds */
    }
  }
  return OK;
}

int32_t turnoff2S(CSOUND *csound, TURNOFF2 *p){
  return turnoff2(csound, p, 1);
}

int32_t turnoff2k(CSOUND *csound, TURNOFF2 *p){
  return turnoff2(csound, p, 0);
}

int32_t turnoff3(CSOUND *csound, TURNOFF2 *p, int32_t isStringArg)
{
  MYFLT p1;
  int32_t   insno;

  if (isStringArg) {
    p1 = (MYFLT) strarg2insno(csound, ((STRINGDAT *)p->kInsNo)->data, 1);
  }
  else if (isstrcod(*p->kInsNo)) {
    p1 = (MYFLT) strarg2insno(csound, get_arg_string(csound, *p->kInsNo), 1);
  }
  else p1 = *(p->kInsNo);

  if (p1 <= FL(0.0))
    return OK;    /* not triggered */

  insno = (int32_t) p1;
  //printf("*** turnoff3: insno = %d, p1 = %f\n", insno, p1);
  if (UNLIKELY(insno < 1 || insno > (int32_t) csound->engineState.maxinsno ||
               csound->engineState.instrtxtp[insno] == NULL)) {
    return csoundPerfError(csound, &(p->h),
                           Str("turnoff3: invalid instrument number"));
  }
  delete_selected_rt_events(csound, p1);
  return OK;
}

int32_t turnoff3S(CSOUND *csound, TURNOFF2 *p){
  return turnoff3(csound, p, 1);
}

int32_t turnoff3k(CSOUND *csound, TURNOFF2 *p){
  return turnoff3(csound, p, 0);
}

int32_t loop_l_i(CSOUND *csound, LOOP_OPS *p)
{
  /* if ((indxvar += iincr) < ilimit) igoto l */
  *(p->ndxvar) += *(p->incr);
  if (*(p->ndxvar) < *(p->limit))
    csound->ids = p->l->prvi;
  return OK;
}

int32_t loop_le_i(CSOUND *csound, LOOP_OPS *p)
{
  /* if ((indxvar += iincr) <= ilimit) igoto l */
  *(p->ndxvar) += *(p->incr);
  if (*(p->ndxvar) <= *(p->limit))
    csound->ids = p->l->prvi;
  return OK;
}

int32_t loop_g_i(CSOUND *csound, LOOP_OPS *p)
{
  /* if ((indxvar -= idecr) > ilimit) igoto l */
  *(p->ndxvar) -= *(p->incr);
  if (*(p->ndxvar) > *(p->limit))
    csound->ids = p->l->prvi;
  return OK;
}

int32_t loop_ge_i(CSOUND *csound, LOOP_OPS *p)
{
  /* if ((indxvar -= idecr) >= ilimit) igoto l */
  *(p->ndxvar) -= *(p->incr);
  if (*(p->ndxvar) >= *(p->limit))
    csound->ids = p->l->prvi;
  return OK;
}

int32_t loop_l_p(CSOUND *csound, LOOP_OPS *p)
{
  /* if ((kndxvar += kincr) < klimit) kgoto l */
  IGN(csound);
  *(p->ndxvar) += *(p->incr);
  if (*(p->ndxvar) < *(p->limit))
    CS_PDS = p->l->prvp;
  return OK;
}

int32_t loop_le_p(CSOUND *csound, LOOP_OPS *p)
{
  /* if ((kndxvar += kincr) <= klimit) kgoto l */
  IGN(csound);
  *(p->ndxvar) += *(p->incr);
  if (*(p->ndxvar) <= *(p->limit))
    CS_PDS = p->l->prvp;
  return OK;
}

int32_t loop_g_p(CSOUND *csound, LOOP_OPS *p)
{
  /* if ((kndxvar -= kdecr) > klimit) kgoto l */
  IGN(csound);
  *(p->ndxvar) -= *(p->incr);
  if (*(p->ndxvar) > *(p->limit))
    CS_PDS = p->l->prvp;
  return OK;
}

int32_t loop_ge_p(CSOUND *csound, LOOP_OPS *p)
{
  /* if ((kndxvar -= kdecr) >= klimit) kgoto l */
  IGN(csound);
  *(p->ndxvar) -= *(p->incr);
  if (*(p->ndxvar) >= *(p->limit))
    CS_PDS = p->l->prvp;
  return OK;
}

static OENTRY goto_ops_localops[] = {
    {"cggoto.0", sizeof(CGOTO), 0, 3, "", "Bl", (SUBR)icgoto, (SUBR)kcgoto,
     NULL, NULL},
    {"cigoto", sizeof(CGOTO), 0, 1, "", "Bl", (SUBR)icgoto, NULL, NULL, NULL},
    {"cingoto", sizeof(CGOTO), 0, 1, "", "Bl", (SUBR)ingoto, NULL, NULL, NULL},
    {"ckgoto", sizeof(CGOTO), 0, 2, "", "Bl", NULL, (SUBR)kcgoto, NULL, NULL},
    {"cngoto", sizeof(CGOTO), 0, 3, "", "Bl", (SUBR)ingoto, (SUBR)kngoto, NULL,
     NULL},
    {"cnkgoto", sizeof(CGOTO), 0, 2, "", "Bl", NULL, (SUBR)kngoto, NULL, NULL},
    {"goto", sizeof(GOTO), 0, 3, "", "l", (SUBR)igoto, (SUBR)kgoto, NULL, NULL},
    {"igoto", sizeof(GOTO), 0, 1, "", "l", (SUBR)igoto, NULL, NULL, NULL},
    {"ihold", sizeof(LINK), 0, 1, "", "", (SUBR)ihold, NULL, NULL, NULL},
    {"kgoto", sizeof(GOTO), 0, 2, "", "l", NULL, (SUBR)kgoto, NULL, NULL},
    {"loop_ge.i", sizeof(LOOP_OPS), 0, 1, "", "iiil", (SUBR)loop_ge_i, NULL,
     NULL, NULL},
    {"loop_ge.k", sizeof(LOOP_OPS), 0, 2, "", "kkkl", NULL, (SUBR)loop_ge_p,
     NULL, NULL},
    {"loop_gt.i", sizeof(LOOP_OPS), 0, 1, "", "iiil", (SUBR)loop_g_i, NULL,
     NULL, NULL},
    {"loop_gt.k", sizeof(LOOP_OPS), 0, 2, "", "kkkl", NULL, (SUBR)loop_g_p,
     NULL, NULL},
    {"loop_le.i", sizeof(LOOP_OPS), 0, 1, "", "iiil", (SUBR)loop_le_i, NULL,
     NULL, NULL},
    {"loop_le.k", sizeof(LOOP_OPS), 0, 2, "", "kkkl", NULL, (SUBR)loop_le_p,
     NULL, NULL},
    {"loop_lt.i", sizeof(LOOP_OPS), 0, 1, "", "iiil", (SUBR)loop_l_i, NULL,
     NULL, NULL},
    {"loop_lt.k", sizeof(LOOP_OPS), 0, 2, "", "kkkl", NULL, (SUBR)loop_l_p,
     NULL, NULL},
    {"reinit", sizeof(GOTO), 0, 2, "", "l", NULL, (SUBR)reinit, NULL, NULL},
    {"rigoto", sizeof(GOTO), 0, 1, "", "l", (SUBR)rigoto, NULL, NULL, NULL},
    {"rireturn", sizeof(LINK), 0, 1, "", "", (SUBR)rireturn, NULL, NULL, NULL},
    {"tigoto", sizeof(GOTO), 0, 1, "", "l", (SUBR)tigoto, NULL, NULL, NULL},
    {"timout", sizeof(TIMOUT), 0, 3, "", "iil", (SUBR)timset, (SUBR)timout,
     NULL, NULL},
    {"tival", sizeof(EVAL), 0, 1, "i", "", (SUBR)tival, NULL, NULL, NULL},
    {"turnoff2.c", sizeof(TURNOFF2), _CW, 2, "", "ikk", NULL, (SUBR)turnoff2k,
     NULL, NULL},
    {"turnoff2.i", sizeof(TURNOFF2), _CW, 2, "", "ikk", NULL, (SUBR)turnoff2k,
     NULL, NULL},
    {"turnoff2.k", sizeof(TURNOFF2), _CW, 2, "", "kkk", NULL, (SUBR)turnoff2k,
     NULL, NULL},
    {"turnoff2.r", sizeof(TURNOFF2), _CW, 2, "", "ikk", NULL, (SUBR)turnoff2k,
     NULL, NULL},
    {"turnoff2.S", sizeof(TURNOFF2), _CW, 2, "", "Skk", NULL, (SUBR)turnoff2S,
     NULL, NULL},
    {"turnoff2_i.i", sizeof(TURNOFF2), _CW, 1, "", "ioo", (SUBR)turnoff2k, NULL,
     NULL, NULL},
    {"turnoff2_i.S", sizeof(TURNOFF2), _CW, 1, "", "Soo", (SUBR)turnoff2S, NULL,
     NULL, NULL},
    {"turnoff3.c", sizeof(TURNOFF2), _CW, 2, "", "i", NULL, (SUBR)turnoff3k,
     NULL, NULL},
    {"turnoff3.i", sizeof(TURNOFF2), _CW, 2, "", "i", NULL, (SUBR)turnoff3k,
     NULL, NULL},
    {"turnoff3.k", sizeof(TURNOFF2), _CW, 2, "", "k", NULL, (SUBR)turnoff3k,
     NULL, NULL},
    {"turnoff3.r", sizeof(TURNOFF2), _CW, 2, "", "i", NULL, (SUBR)turnoff3k,
     NULL, NULL},
    {"turnoff3.S", sizeof(TURNOFF2), _CW, 2, "", "S", NULL, (SUBR)turnoff3S,
     NULL, NULL},
    {"turnoff", sizeof(LINK), 0, 2, "", "", NULL, (SUBR)turnoff, NULL, NULL},
};

LINKAGE_BUILTIN(goto_ops_localops)
