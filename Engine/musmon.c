/*
  musmon.c:

  Copyright (C) 1991,2002 Barry Vercoe, John ffitch,
  Istvan Varga, rasmus ekman

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
#include "musmon.h"

#include "csoundCore_internal.h"         /*                         MUSMON.C     */
#include "midiops.h"
#include "soundio.h"
#include "namedins.h"
#include "oload.h"
#include "remote.h"
#include <math.h>
#include "corfile.h"

#include "csdebug.h"
#include "cwindow.h"
#include "rdscor.h"
#include "namedins_public.h"
#include "memalloc.h"
#include "fgens_public.h"
#include "insert_public.h"
#include "server.h"
#include "csound_threads.h"
#include "text.h"
#include "musmon_internal.h"
#include "insert.h"
#include "libsnd_internal.h"
#include "libsnd_u_internal.h"
#include "linevent.h"
#include "scsort.h"
#include "ugens6.h"
#include "rdscor_internal.h"
#include "memfiles_internal.h"
#include "rdscor_internal.h"
#include "cscore.h"
#include "libsnd_u_internal.h"
#include "midirecv.h"
#include "midisend.h"
#include "scsort.h"
#include "libsnd_internal.h"
#include "envvar.h"

  void    MidiOpen(CSOUND *);
  void    m_chn_init_all(CSOUND *);
//  char *  scsortstr(CSOUND *, CORFIL *);
  void    infoff(CSOUND*, MYFLT), orcompact(CSOUND*);
  void    beatexpire(CSOUND *, double), timexpire(CSOUND *, double);
  void    MidiClose(CSOUND *);
  void    RTclose(CSOUND *);
  void    remote_Cleanup(CSOUND *);
void print_csound_version(CSOUND*);

/* IV - Jan 28 2005 */
void print_benchmark_info(CSOUND *csound, const char *s)
{
  double  rt, ct;

  if ((csound->oparms->msglevel & CS_TIMEMSG) == 0 || csound->csRtClock == NULL)
    return;
  rt = csoundGetRealTime(csound->csRtClock);
  ct = csoundGetCPUTime(csound->csRtClock);
  csoundErrorMsg(csound,
                  Str("Elapsed time at %s: real: %.3fs, CPU: %.3fs\n"),
                  (char*) s, rt, ct);
}

void print_engine_parameters(CSOUND *csound);
void print_sndfile_version(CSOUND* csound);

static inline void cs_beep(CSOUND *csound)
{
    csoundErrorMsg(csound, Str("%c\tbeep!\n"), '\a');
}

void print_maxamp(CSOUND *csound, MYFLT x)
{
    int   attr = 0;
    if (!(csound->oparms->msglevel & 0x60)) {   /* 0x00: raw amplitudes */
      if (csound->oparms->msglevel & 0x300) {
        MYFLT y = x / csound->e0dbfs;     /* relative level */
        if (UNLIKELY(y >= FL(1.0)))                    /* >= 0 dB: red */
          attr = CSOUNDMSG_FG_BOLD | CSOUNDMSG_FG_RED;
        else if (csound->oparms->msglevel & 0x200) {
          if (y >= FL(0.5))                            /* -6..0 dB: yellow */
            attr = CSOUNDMSG_FG_BOLD | CSOUNDMSG_FG_CYAN; /* was yellow but... */
          else if (y >= FL(0.125))                      /* -24..-6 dB: green */
            attr = CSOUNDMSG_FG_BOLD | CSOUNDMSG_FG_GREEN;
          else                                          /* -200..-24 dB: blue */
            attr = CSOUNDMSG_FG_BOLD | CSOUNDMSG_FG_BLUE;
        }
      }
      if (csound->e0dbfs > FL(3000.0))
        csoundMessageS(csound, attr, "%9.1f", x);
      else if (csound->e0dbfs < FL(3.0))
        csoundMessageS(csound, attr, "%9.5f", x);
      else if (csound->e0dbfs > FL(300.0))
       csoundMessageS(csound, attr, "%9.2f", x);
      else if (csound->e0dbfs > FL(30.0))
        csoundMessageS(csound, attr, "%9.3f", x);
      else
        csoundMessageS(csound, attr, "%9.4f", x);
    }
    else {                              /* dB values */
      MYFLT y = x / csound->e0dbfs;     /* relative level */
      if (UNLIKELY(y < FL(1.0e-10))) {
        /* less than -200 dB: print zero */
        csoundMessage(csound, "      0  ");
        return;
      }
      y = FL(20.0) * (MYFLT) log10((double) y);
      if (csound->oparms->msglevel & 0x40) {
        if (UNLIKELY(y >= FL(0.0)))                     /* >= 0 dB: red */
          attr = CSOUNDMSG_FG_BOLD | CSOUNDMSG_FG_RED;
        else if (csound->oparms->msglevel & 0x20) {
          if (y >= FL(-6.0))                            /* -6..0 dB: yellow */
            attr = CSOUNDMSG_FG_BOLD | CSOUNDMSG_FG_YELLOW;
          else if (y >= FL(-24.0))                      /* -24..-6 dB: green */
            attr = CSOUNDMSG_FG_BOLD | CSOUNDMSG_FG_GREEN;
          else                                          /* -200..-24 dB: blue */
            attr = CSOUNDMSG_FG_BOLD | CSOUNDMSG_FG_BLUE;
        }
      }
      csoundMessageS(csound, attr, "%+9.2f", y);
    }
}

PUBLIC int csoundCleanup(CSOUND *csound)
{
    void    *p;
    MYFLT   *maxp;
    int32   *rngp;
    uint32_t n;

    csoundLockMutex(csound->API_lock);
    if (csoundQueryGlobalVariable(csound,"::UDPCOM")
        != NULL) csoundUDPServerClose(csound);



    while (csound->evtFuncChain != NULL) {
      p = (void*) csound->evtFuncChain;
      csound->evtFuncChain = ((EVT_CB_FUNC*) p)->nxt;
      mfree(csound,p);
    }

    /* check if we have already cleaned up */
    if (!(csound->engineStatus & CS_STATE_CLN)){
      csoundUnlockMutex(csound->API_lock);
      return 0;
    }
    /* will not clean up more than once */
    csound->engineStatus &= ~(CS_STATE_CLN);

    deactivate_all_notes(csound);

    if (csound->engineState.instrtxtp &&
        csound->engineState.instrtxtp[0] &&
        csound->engineState.instrtxtp[0]->instance &&
        csound->engineState.instrtxtp[0]->instance->actflg)
      xturnoff_now(csound, csound->engineState.instrtxtp[0]->instance);
    delete_pending_rt_events(csound);

#ifndef __EMSCRIPTEN__
    if (csound->event_insert_loop == 1) {
      csound->event_insert_loop = 0;
      csoundJoinThread(csound->event_insert_thread);
      csoundDestroyMutex(csound->init_pass_threadlock);
      csound->event_insert_thread = 0;
    }
#endif

    while (csound->freeEvtNodes != NULL) {
      p = (void*) csound->freeEvtNodes;
      csound->freeEvtNodes = ((EVTNODE*) p)->nxt;
      mfree(csound,p);
    }

    orcompact(csound);

    corfile_rm(csound, &csound->scstr);

    /* print stats only if musmon was actually run */
    /* NOT SURE HOW   ************************** */
    // if(csound->oparms->msglevel)
    {
      csoundErrorMsg(csound, Str("end of score.\t\t   overall amps:"));
      corfile_rm(csound, &csound->expanded_sco);
      for (n = 0; n < csound->nchnls; n++) {
        if (csound->smaxamp[n] > csound->omaxamp[n])
          csound->omaxamp[n] = csound->smaxamp[n];
        if (csound->maxamp[n] > csound->omaxamp[n])
          csound->omaxamp[n] = csound->maxamp[n];
        STA_MUSMON(orngcnt)[n] += (STA_MUSMON(srngcnt)[n] + csound->rngcnt[n]);
      }
      for (maxp = csound->omaxamp, n = csound->nchnls; n--; )
        print_maxamp(csound, *maxp++);
      if (csound->oparms->outformat != AE_FLOAT) {
        csoundErrorMsg(csound, Str("\n\t   overall samples out of range:"));
        for (rngp = STA_MUSMON(orngcnt), n = csound->nchnls; n--; )
          csoundErrorMsg(csound, "%9d", *rngp++);
      }
      csoundErrorMsg(csound, Str("\n%d errors in performance\n"),
                      csound->perferrcnt);
      print_benchmark_info(csound, Str("end of performance"));
      if (csound->print_version) print_csound_version(csound);
    }
    /* close line input (-L) */
    RTclose(csound);
    /* close MIDI input */
    MidiClose(csound);

    /* IV - Feb 03 2005: do not need to call rtclose from here, */
    /* as sfclosein/sfcloseout will do that. */
    if (!csound->enableHostImplementedAudioIO) {
      sfclosein(csound);
      sfcloseout(csound);
      if (UNLIKELY(!csound->oparms->sfwrite)) {
        if(csound->oparms->msglevel ||csound->oparms->odebug)
         csoundErrorMsg(csound, Str("no sound written to disk\n"));
      }
    }
    /* close any remote.c sockets */
    if (csound->remoteGlobals) remote_Cleanup(csound);
    if (UNLIKELY(csound->oparms->ringbell))
      cs_beep(csound);

    csoundUnlockMutex(csound->API_lock);
    return dispexit(csound);    /* hold or terminate the display output     */
}

/* Print current amplitude values, and update section amps. */

void print_amp_values(CSOUND *csound, int score_evt)
{
  CSOUND        *p = csound;
  MYFLT         *maxp, *smaxp;
  uint32        *maxps, *smaxps;
  int32         *rngp, *srngp;
  int           n;

  if (UNLIKELY(STA_MUSMON(segamps) || (p->rngflg && STA_MUSMON(sormsg)))) {
    if (score_evt > 0)
      csoundMessage(p, "B%7.3f ..%7.3f T%7.3f TT%7.3f M:",
                 p->prvbt - p->beatOffs,  p->curbt - p->beatOffs,
                 p->curp2 - p->timeOffs,  p->curp2);
    else
      csoundMessage(p, "  rtevent:\t   T%7.3f TT%7.3f M:",
                 p->curp2 - p->timeOffs,  p->curp2);

    for (n = p->nchnls, maxp = p->maxamp; n--; )
      print_maxamp(p, *maxp++);               /* IV - Jul 9 2002 */
    csoundMessage(p, "\n");
    if (UNLIKELY(p->rngflg)) {
      csoundMessage(p, Str("\t number of samples out of range:"));
      for (n = p->nchnls, rngp = p->rngcnt; n--; )
        csoundMessage(p, "%9d", *rngp++);
      csoundMessage(p, "\n");
    }
  }
  if (p->rngflg) {
    p->rngflg = 0;
    STA_MUSMON(srngflg)++;
  }
  for (n = p->nchnls,
         maxp = p->maxamp - 1, smaxp = p->smaxamp - 1,
         maxps = p->maxpos - 1, smaxps = p->smaxpos - 1,
         rngp = p->rngcnt, srngp = STA_MUSMON(srngcnt); n--; ) {
    ++maxps; ++smaxps;
    if (*++maxp > *++smaxp) {
      *smaxp = *maxp;
      *smaxps = *maxps;
    }
    *maxp = FL(0.0);
    *maxps = 0;
    *srngp++ += *rngp;
    *rngp++ = 0;
  }
}

static inline uint64_t time2kcnt(CSOUND *csound, double tval)
{
  if (tval > 0.0) {
    tval *= (double) csound->ekr;
#ifdef HAVE_C99
    return (uint64_t) llrint(tval);
#else
    return (uint64_t) (tval + 0.5);
#endif
  }
  return 0UL;
}


/* Schedule new score event to be played. 'time_ofs' is the amount of */
/* time in seconds to add to evt->p[2] to get the actual start time   */
/* of the event (measured from the beginning of performance, and not  */
/* section) in seconds.                                               */
/* Required parameters in 'evt':                                      */
/*   char   *strarg   string argument of event (NULL if none)         */
/*   char   opcod     event opcode (a, e, f, i, l, q, s)              */
/*   int16  pcnt      number of p-fields (>=3 for q, i, a; >=4 for f) */
/*   MYFLT  p[]       array of p-fields, p[1]..p[pcnt] should be set  */
/*  p2orig and p3orig are calculated from p[2] and p[3].              */
/* The contents of 'evt', including the string argument, need not be  */
/* preserved after calling this function, as a copy of the event is   */
/* made.                                                              */
/* Return value is zero on success.                                   */

int insert_score_event_at_sample(CSOUND *csound, EVTBLK *evt, int64_t time_ofs)
{
  double        start_time;
  EVTNODE       *e, *prv;
  CSOUND        *st = csound;
  MYFLT         *p;
  uint32        start_kcnt;
  int           i, retval;

  retval = -1;
  /* make a copy of the event... */
  if (csound->freeEvtNodes != NULL) {             /* pop alloc from stack */
    e = csound->freeEvtNodes;                     /*   if available       */
    csound->freeEvtNodes = e->nxt;
  }
  else {
    e = (EVTNODE*) mcalloc(csound, sizeof(EVTNODE)); /* or alloc new one */
    if (UNLIKELY(e == NULL))
      return CSOUND_MEMORY;
  }
  if (evt->strarg != NULL) {  /* copy string argument if present */
    /* NEED TO COPY WHOLE STRING STRUCTURE */
    int n = evt->scnt;
    char *p = evt->strarg;
    while (n--) { p += strlen(p)+1; };
    e->evt.strarg = (char*) mmalloc(csound, (size_t) (p-evt->strarg)+1);
    if (UNLIKELY(e->evt.strarg == NULL)) {
      mfree(csound, e);
      return CSOUND_MEMORY;
    }
    memcpy(e->evt.strarg, evt->strarg, p-evt->strarg+1 );
    e->evt.scnt = evt->scnt;
  }
  e->evt.pinstance = evt->pinstance;
  e->evt.opcod = evt->opcod;
  e->evt.pcnt = evt->pcnt;
  p = &(e->evt.p[0]);
  i = 0;
  while (++i <= evt->pcnt)    /* copy p-field list */
    p[i] = evt->p[i];
  /* ...and use the copy from now on */
  evt = &(e->evt);

  /* check for required p-fields */
  switch (evt->opcod) {
  case 'f':
    if (UNLIKELY((evt->pcnt < 4) && (p[1]>0)))
      goto pfld_err;
    goto cont;
  case 'i':
  case 'q':
  case 'a':
    if (UNLIKELY(evt->pcnt < 3))
      goto pfld_err;
    /* fall through */
  case 'd':
  cont:
    /* calculate actual start time in seconds and k-periods */
    start_time = (double) p[2] + (double)time_ofs/csound->esr;
    start_kcnt = time2kcnt(csound, start_time);
    /* correct p2 value for section offset */
    p[2] = (MYFLT) (start_time - st->timeOffs);
    if (p[2] < FL(0.0))
      p[2] = FL(0.0);
    /* start beat: this is possibly wrong */
    evt->p2orig = (MYFLT) (((start_time - st->icurTime/st->esr) /
                            st->ibeatTime)
                           + (st->curBeat - st->beatOffs));
    if (evt->p2orig < FL(0.0))
      evt->p2orig = FL(0.0);
    evt->p3orig = p[3];
    break;
  default:
    start_kcnt = 0UL;   /* compiler only */
  }

  switch (evt->opcod) {
  case 'i':                         /* note event */
  case 'd':
    /* calculate the length in beats */
    if (evt->p3orig > FL(0.0))
      evt->p3orig = (MYFLT) ((double) evt->p3orig / st->ibeatTime);
    /* fall through */
  case 'q':                         /* mute instrument */
    /* check for a valid instrument number or name */
    if (evt->opcod=='d') {
      if (evt->strarg != NULL && isstrcod(p[1])) {
        i = (int) named_instr_find(csound, evt->strarg);
        //printf("d opcode %s -> %d\n", evt->strarg, i);
        p[1] = -i;
      }
      else {
        i = (int) fabs((double) p[1]);
        p[1] = -i;
      }
    }
    else if (evt->strarg != NULL && isstrcod(p[1])) {
      MYFLT n = named_instr_find(csound, evt->strarg);
      p[1] = n;
      i =(int) n;
      if (n<0) {i= -i;}
    }
    else
      i = (int) fabs((double) p[1]);
    if (UNLIKELY((unsigned int) (i - 1) >=
                 (unsigned int) csound->engineState.maxinsno ||
                 csound->engineState.instrtxtp[i] == NULL)) {
      if (i > INT32_MAX-10)
        csoundErrorMsg(csound, "%s",
                      Str("insert_score_event(): invalid named instrument\n"));
      else
        csoundErrorMsg(csound, Str("insert_score_event(): invalid instrument "
                                  "number or name %d\n" ), i);
      goto err_return;
    }
    break;
  case 'a':                         /* advance score time */
    /* calculate the length in beats */
    evt->p3orig = (MYFLT) ((double) evt->p3orig *csound->esr/ st->ibeatTime);
    /* fall through */
  case 'f':                         /* function table */
    break;
  case 'e':                         /* end of score, */
  case 'l':                         /*   lplay list, */
  case 's':                         /*   section:    */
    start_time = (double)time_ofs/csound->esr;
    if (evt->pcnt >= 2)
      start_time += (double) p[2];
    evt->pcnt = 0;
    start_kcnt = time2kcnt(csound, start_time);
    break;
  default:
    csoundErrorMsg(csound, Str("insert_score_event(): unknown opcode: %c\n"),
                  evt->opcod);
    goto err_return;
  }
  /* queue new event */
  e->start_kcnt = start_kcnt;
  prv = csound->OrcTrigEvts;
  /* if list is empty, or at beginning of list: */
  if (prv == NULL || start_kcnt < prv->start_kcnt) {
    e->nxt = prv;
    csound->OrcTrigEvts = e;
  }
  else {                                      /* otherwise sort by time */
    while (prv->nxt != NULL && start_kcnt >= prv->nxt->start_kcnt)
      prv = prv->nxt;
    e->nxt = prv->nxt;
    prv->nxt = e;
  }
  /* Make sure sensevents() looks for RT events */
  csound->oparms->RTevents = 1;
  return 0;

 pfld_err:
  csoundErrorMsg(csound, Str("insert_score_event(): insufficient p-fields\n"));
 err_return:
  /* clean up */
  if (e->evt.strarg != NULL)
    mfree(csound, e->evt.strarg);
  e->evt.strarg = NULL;
  e->nxt = csound->freeEvtNodes;
  csound->freeEvtNodes = e;
  return retval;
}

int insert_score_event(CSOUND *csound, EVTBLK *evt, double time_ofs)
{
  return insert_score_event_at_sample(csound, evt, time_ofs*csound->esr);
}

/* called by csoundRewindScore() to reset performance to time zero */

/**
 * Register a function to be called once in every control period
 * by sensevents(). Any number of functions may be registered,
 * and will be called in the order of registration.
 * The callback function takes two arguments: the Csound instance
 * pointer, and the userData pointer as passed to this function.
 * Returns zero on success.
 */
PUBLIC int csoundRegisterSenseEventCallback(CSOUND *csound,
                                            void (*func)(CSOUND *, void *),
                                            void *userData)
{
  EVT_CB_FUNC *fp = (EVT_CB_FUNC*) csound->evtFuncChain;

  if (fp == NULL) {
    fp = (EVT_CB_FUNC*) mcalloc(csound, sizeof(EVT_CB_FUNC));
    csound->evtFuncChain = (void*) fp;
  }
  else {
    while (fp->nxt != NULL)
      fp = fp->nxt;
    fp->nxt = (EVT_CB_FUNC*) mcalloc(csound, sizeof(EVT_CB_FUNC));
    fp = fp->nxt;
  }
  if (UNLIKELY(fp == NULL))
    return CSOUND_MEMORY;
  fp->func = func;
  fp->userData = userData;
  fp->nxt = NULL;
  csound->oparms->RTevents = 1;

  return 0;
}
