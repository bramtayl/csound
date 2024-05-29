#include "musmon_internal.h"
#include "musmon.h"
#include "csound.h"
#include "oload.h"
#include "fgens_public.h"
#include "rdscor.h"
#include "namedins_public.h"
#include "csoundCore_internal.h"
#include "text.h"
#include "insert_public.h"
#include "midirecv.h"
#include "cwindow.h"
#include "ugens6.h"
#include "libsnd_u_internal.h"
#include "memalloc.h"
#include "csound_internal.h"
#include "insert.h"
#include "midirecv.h"
#include "midisend.h"
#include "linevent.h"
#include "soundio.h"
#include "libsnd_internal.h"
#include "corfile.h"
#include "scsort.h"
#include "csound_threads.h"
#include "csdebug.h"
#include "insert.h"
#include "namedins.h"
#include "rdscor_internal.h"
#include "memfiles_internal.h"

void settempo(CSOUND *csound, double tempo)
{
    if (tempo <= 0.0) return;
    if (csound->oparms->Beatmode==1)
      csound->ibeatTime = (int64_t)(csound->esr*60.0 / tempo);
    csound->curBeat_inc = tempo / (60.0 * (double) csound->ekr);
}

/* make list to turn on instrs for indef */
/* perf called from i0 for execution in playevents */

int turnon(CSOUND *csound, TURNON *p)
{
  EVTBLK  evt;
  int insno;
  memset(&evt, 0, sizeof(EVTBLK));
  evt.strarg = NULL; evt.scnt = 0;
  evt.opcod = 'i';
  evt.pcnt = 3;

  if (isstrcod(*p->insno)) {
    char *ss = get_arg_string(csound,*p->insno);
    insno = strarg2insno(csound,ss,1);
    if (insno == NOT_AN_INSTRUMENT)
      return NOTOK;
  } else insno = *p->insno;
  evt.p[1] = (MYFLT) insno;
  evt.p[2] = *p->itime;
  evt.p[3] = FL(-1.0);
  evt.c.extra = NULL;
  return insert_score_event_at_sample(csound, &evt, csound->icurTime);
}

/* make list to turn on instrs for indef */
/* perf called from i0 for execution in playevents */

int turnon_S(CSOUND *csound, TURNON *p)
{
  EVTBLK  evt;
  int     insno;
  memset(&evt, 0, sizeof(EVTBLK));
  evt.strarg = NULL; evt.scnt = 0;
  evt.opcod = 'i';
  evt.pcnt = 3;
  insno = strarg2insno(csound, ((STRINGDAT *)p->insno)->data, 1);
  if (UNLIKELY(insno == NOT_AN_INSTRUMENT))
    return NOTOK;
  evt.p[1] = (MYFLT) insno;
  evt.p[2] = *p->itime;
  evt.p[3] = FL(-1.0);
  evt.c.extra = NULL;
  return insert_score_event_at_sample(csound, &evt, csound->icurTime);
}

int tempset(CSOUND *csound, TEMPO *p)
{
    double tempo;

    if (UNLIKELY((tempo = (double)*p->istartempo) <= FL(0.0))) {
      return csoundInitError(csound, Str("illegal istartempo value"));
    }
    if (UNLIKELY(csound->oparms->Beatmode==0))
      return csoundInitError(csound, Str("Beat mode not in force"));
    settempo(csound, tempo);
    p->prvtempo = (MYFLT)tempo;
    return OK;
}

int tempo(CSOUND *csound, TEMPO *p)
{
    if (*p->ktempo != p->prvtempo) {
      settempo(csound, (double)*p->ktempo);
      p->prvtempo = *p->ktempo;
    }
    return OK;
}

int gettempo(CSOUND *csound, GTEMPO *p)
{
    if (LIKELY(csound->oparms->Beatmode)) {
      *p->ans = FL(60.0) * csound->esr / (MYFLT)csound->ibeatTime;
    }
    else
      *p->ans = FL(60.0);
    return OK;
}


void print_csound_version(CSOUND* csound)
{
#ifdef USE_DOUBLE
#ifdef BETA
    csoundErrorMsg(csound,
                    Str("--Csound version %s beta (double samples) %s\n"
                        "[commit: %s]\n"),
                    CS_PACKAGE_VERSION, CS_PACKAGE_DATE,
                    STRING_HASH(GIT_HASH_VALUE));
#else
    csoundErrorMsg(csound, Str("--Csound version %s (double samples) %s\n"
                                "[commit: %s]\n"),
                    CS_PACKAGE_VERSION, CS_PACKAGE_DATE
                    , STRING_HASH(GIT_HASH_VALUE));
#endif
#else
#ifdef BETA
    csoundErrorMsg(csound, Str("--Csound version %s beta (float samples) %s\n"
                                "[commit: %s]\n"),
                    CS_PACKAGE_VERSION, CS_PACKAGE_DATE,
                    STRING_HASH(GIT_HASH_VALUE));
#else
    csoundErrorMsg(csound, Str("--Csound version %s (float samples) %s\n"
                                "[commit: %s]\n"),
                    CS_PACKAGE_VERSION, CS_PACKAGE_DATE,
                    STRING_HASH(GIT_HASH_VALUE));
#endif
#endif
}

void print_sndfile_version(CSOUND* csound) {
#ifdef USE_LIBSNDFILE
        char buffer[128];
        sflib_command(NULL, SFC_GET_LIB_VERSION, buffer, 128);
        csoundErrorMsg(csound, "%s\n", buffer);
#else
        csoundErrorMsg(csound, "%s\n", "No soundfile IO");
#endif
}

#define SEGAMPS CS_AMPLMSG
#define SORMSG  CS_RNGEMSG

/**
  Open and Initialises the input/output
  returns the HW sampling rate if it has been
  set, -1.0 otherwise.
*/
MYFLT csoundInitialiseIO(CSOUND *csound) {
    OPARMS *O = csound->oparms;
    if (csound->enableHostImplementedAudioIO &&
        csound->hostRequestedBufferSize) {
      int bufsize    = (int) csound->hostRequestedBufferSize;
      int ksmps      = (int) csound->ksmps;
      bufsize        = (bufsize + (ksmps >> 1)) / ksmps;
      bufsize        = (bufsize ? bufsize * ksmps : ksmps);
      O->outbufsamps = O->inbufsamps = bufsize;
    }
    else {
      if (!O->oMaxLag)
        O->oMaxLag = IODACSAMPS;
      if (!O->outbufsamps)
        O->outbufsamps = IOBUFSAMPS;
      else if (UNLIKELY(O->outbufsamps < 0)) { /* if k-aligned iobufs requested  */
        /* set from absolute value */
        O->outbufsamps *= -((int64_t)csound->ksmps);
        csoundErrorMsg(csound, Str("k-period aligned audio buffering\n"));
        if (O->oMaxLag <= O->outbufsamps)
          O->oMaxLag = O->outbufsamps << 1;
      }
      /* else keep the user values */
      /* IV - Feb 04 2005: make sure that buffer sizes for real time audio */
      /* are usable */
      if (check_rtaudio_name(O->infilename, NULL, 0) >= 0 ||
          check_rtaudio_name(O->outfilename, NULL, 1) >= 0) {
        O->oMaxLag = ((O->oMaxLag + O->outbufsamps - 1) / O->outbufsamps)
          * O->outbufsamps;
        if (O->oMaxLag <= O->outbufsamps && O->outbufsamps > 1)
          O->outbufsamps >>= 1;
      }
      O->inbufsamps = O->outbufsamps;
    }
     csoundErrorMsg(csound, Str("audio buffered in %d sample-frame blocks\n"),
                    (int) O->outbufsamps);
    O->inbufsamps  *= csound->inchnls;    /* now adjusted for n channels  */
    O->outbufsamps *= csound->nchnls;
    iotranset(csound);          /* point recv & tran to audio formatter */
    /* open audio file or device for input first, and then for output */
    if (!csound->enableHostImplementedAudioIO) {
      if (O->sfread)
        sfopenin(csound);
      if (O->sfwrite && !csound->initonly)
        sfopenout(csound);
      else
       sfnopenout(csound);
    }
    csound->io_initialised = 1;
    return csoundSystemSr(csound, 0);
 }

int musmon(CSOUND *csound)
{
    OPARMS  *O = csound->oparms;
    /* VL - 08-07-21 messages moved here so we can switch them off */
    print_csound_version(csound);
    print_sndfile_version(csound);

    /* initialise search path cache */
    csoundGetSearchPathFromEnv(csound, "SNAPDIR");
    csoundGetSearchPathFromEnv(csound, "SFDIR;SSDIR;INCDIR");
    csoundGetSearchPathFromEnv(csound, "SFDIR");
    csoundGetSearchPathFromEnv(csound, "SADIR");
    csoundGetSearchPathFromEnv(csound, "SFDIR;SSDIR");

    m_chn_init_all(csound);     /* allocate MIDI channels */
    dispinit(csound);           /* initialise graphics or character display */

    reverbinit(csound);
    dbfs_init(csound, csound->e0dbfs);
    csound->nspout = csound->ksmps * csound->nchnls;  /* alloc spin & spout */
    csound->nspin = csound->ksmps * csound->inchnls; /* JPff: in preparation */
    csound->spin  = (MYFLT *) mcalloc(csound, csound->nspin*sizeof(MYFLT));
    csound->spout_tmp = (MYFLT *)
      mcalloc(csound,(csound->oparms->numThreads+1)*csound->nspout*sizeof(MYFLT));
    csound->spout = (MYFLT *) mcalloc(csound, csound->nspout*sizeof(MYFLT));
    csound->auxspin = (MYFLT *) mcalloc(csound, csound->nspin*sizeof(MYFLT));
    /* memset(csound->maxamp, '\0', sizeof(MYFLT)*MAXCHNLS); */
    /* memset(csound->smaxamp, '\0', sizeof(MYFLT)*MAXCHNLS); */
    /* memset(csound->omaxamp, '\0', sizeof(MYFLT)*MAXCHNLS); */

    /* initialise sensevents state */
    csound->prvbt = csound->curbt = csound->nxtbt = 0.0;
    csound->curp2 = csound->nxtim = csound->timeOffs = csound->beatOffs = 0.0;
    csound->icurTime = 0L;
    if (O->Beatmode && O->cmdTempo > 0.0) {
      /* if performing from beats, set the initial tempo */
      csound->curBeat_inc = O->cmdTempo / (60.0 * (double) csound->ekr);
      csound->ibeatTime = (int64_t)(csound->esr*60.0 / O->cmdTempo);
    }
    else {
      csound->curBeat_inc = 1.0 / (double) csound->ekr;
      csound->ibeatTime = 1;
    }
    csound->cyclesRemaining = 0;
    memset(&(csound->evt), 0, sizeof(EVTBLK));

    print_engine_parameters(csound);

    /* run instr 0 inits */
    if (UNLIKELY(init0(csound) != 0))
      csoundDie(csound, Str("header init errors"));

    /* kperf() will not call csoundYield() more than 250 times per second */
    csound->evt_poll_cnt    = 0;
    csound->evt_poll_maxcnt =
      (int)(250.0 /(double) csound->ekr); /* VL this was wrong: kr/250 originally */
    /* Enable musmon to handle external MIDI input, if it has been enabled. */
    if (O->Midiin || O->FMidiin || O->RMidiin) {
      O->RTevents = 1;
      MidiOpen(csound);                 /*   alloc bufs & open files    */
    }
    /* open MIDI output (moved here from argdecode) */
    if (O->Midioutname != NULL && O->Midioutname[0] == (char) '\0')
      O->Midioutname = NULL;
    if (O->FMidioutname != NULL && O->FMidioutname[0] == (char) '\0')
      O->FMidioutname = NULL;
    if (O->Midioutname != NULL || O->FMidioutname != NULL)
      openMIDIout(csound);
    if(O->msglevel) {
      csoundErrorMsg(csound, Str("orch now loaded\n"));
    }

    csound->multichan = (csound->nchnls > 1 ? 1 : 0);
    STA_MUSMON(segamps) = O->msglevel & SEGAMPS;
    STA_MUSMON(sormsg)  = O->msglevel & SORMSG;

    if (O->Linein)
      RTLineset(csound);                /* if realtime input expected   */

    // VL 01-05-2019
    // if --use-system-sr, this gets called earlier to override
    // the sampling rate. Otherwise it gets called here.
    if(!csound->io_initialised)
         csoundInitialiseIO(csound);

    if (O->playscore!=NULL) corfile_flush(csound, O->playscore);
    //csound->scfp
    if (UNLIKELY(O->usingcscore)) {
      if (STA_MUSMON(lsect) == NULL) {
        STA_MUSMON(lsect) = (EVENT*) mmalloc(csound, sizeof(EVENT));
        STA_MUSMON(lsect)->op = 'l';
      }
      csoundErrorMsg(csound, Str("using Cscore processing\n"));
      /* override stdout in */
      if (UNLIKELY(!(csound->oscfp = fopen("cscore.out", "w"))))
        csoundDie(csound, Str("cannot create cscore.out"));
      csoundNotifyFileOpened(csound, "cscore.out", CSFTYPE_SCORE_OUT, 1, 0);
      /* rdscor for cscorefns */
      csoundInitializeCscore(csound, csound->scfp, csound->oscfp);
      /* call cscore, optionally re-enter via lplay() */
      csound->cscoreCallback_(csound);
      fclose(csound->oscfp); csound->oscfp = NULL;
      if (csound->scfp != NULL) {
        fclose(csound->scfp);
        csound->scfp = NULL;
      }
      if (STA_MUSMON(lplayed))
        return 0;

      /*  read from cscore.out */
      if (UNLIKELY(!(csound->scfp = fopen("cscore.out", "r")))) {
        csoundDie(csound, Str("cannot reopen cscore.out"));
      }
      else {
        CORFIL *inf = corfile_create_w(csound);
        int c;
        while ((c=getc(csound->scfp))!=EOF) corfile_putc(csound, c, inf);
        corfile_rewind(inf);
        csound->scorestr = inf;
        corfile_rm(csound, &csound->scstr);
      }
      csoundNotifyFileOpened(csound, "cscore.out", CSFTYPE_SCORE_OUT, 0, 0);
      /* write to cscore.srt */
      if (UNLIKELY(!(csound->oscfp = fopen("cscore.srt", "w"))))
        csoundDie(csound, Str("cannot reopen cscore.srt"));
      csoundNotifyFileOpened(csound, "cscore.srt", CSFTYPE_SCORE_OUT, 1, 0);
      csoundErrorMsg(csound, Str("sorting cscore.out ..\n"));
      /* csound->scorestr = copy_to_corefile(csound, "cscore.srt", NULL, 1); */
      scsortstr(csound, csound->scorestr);  /* call the sorter again */
      fclose(csound->scfp); csound->scfp = NULL;
      fputs(corfile_body(csound->scstr), csound->oscfp);
      fclose(csound->oscfp); csound->oscfp = NULL;
      csoundErrorMsg(csound, Str("\t... done\n"));
      csoundErrorMsg(csound, Str("playing from cscore.srt\n"));
      O->usingcscore = 0;
    }

     csoundErrorMsg(csound, Str("SECTION %d:\n"), ++STA_MUSMON(sectno));
    /* apply score offset if non-zero */
    if (csound->csoundScoreOffsetSeconds_ > FL(0.0))
      csoundSetScoreOffsetSeconds(csound, csound->csoundScoreOffsetSeconds_);

#ifndef __EMSCRIPTEN__
    if (csound->oparms->realtime && csound->event_insert_loop == 0){
      csound->init_pass_threadlock = csoundCreateMutex(0);
      csoundErrorMsg(csound, "Initialising spinlock...\n");
      csoundSpinLockInit(&csound->alloc_spinlock);
      csound->event_insert_loop = 1;
      csound->alloc_queue = (ALLOC_DATA *)
        mcalloc(csound, sizeof(ALLOC_DATA)*MAX_ALLOC_QUEUE);
      csound->event_insert_thread =
        csoundCreateThread(event_insert_thread,
                             (void*)csound);
      csoundErrorMsg(csound, "Starting realtime mode queue: %p thread: %p\n",
                      csound->alloc_queue, csound->event_insert_thread );
    }
#endif

    /* since we are running in components, we exit here to playevents later */
    return 0;
}

void delete_selected_rt_events(CSOUND *csound, MYFLT instr)
{
  EVTNODE *ep = csound->OrcTrigEvts;
  EVTNODE *last = NULL;
  while (ep != NULL) {
    EVTNODE *nxt = ep->nxt;
    //printf("*** delete_selected_rt_events: instr = %f, p[1] = %f\n",
    //instr, ep->evt.p[1]);
    if (ep->evt.opcod=='i' &&
        (((int)(ep->evt.p[1]) == instr) || (ep->evt.p[1] == instr))) {
      //printf(" ** found\n");
      // Found an event to cancel
      if (ep->evt.strarg != NULL) {
        // clearstring if necessary
        mfree(csound,ep->evt.strarg);
        ep->evt.strarg = NULL;
      }
      if (last) last->nxt = nxt; else csound->OrcTrigEvts = nxt;
      /* push to stack of free event nodes */
      ep->nxt = csound->freeEvtNodes;
      csound->freeEvtNodes = ep;
    }
    else last = ep;
    ep = nxt;
  }
  //csound->OrcTrigEvts = NULL;
}

int lplay(CSOUND *csound, EVLIST *a)    /* cscore re-entry into musmon */
{
  /* if (csound->musmonGlobals == NULL) */
  /*  csound->musmonGlobals = mcalloc(csound, sizeof(MUSMON_GLOBALS)); */
  STA_MUSMON(lplayed) = 1;
  if (!STA_MUSMON(sectno)) {
    if(csound->oparms->msglevel ||csound->oparms->odebug)
    csoundErrorMsg(csound, Str("SECTION %d:\n"), ++STA_MUSMON(sectno));
    }
  STA_MUSMON(ep) = &a->e[1];                  /* from 1st evlist member */
  STA_MUSMON(epend) = STA_MUSMON(ep) + a->nevents;    /*   to last              */
  while (csoundPerform(csound) == 0)  /* play list members      */
    ;                                 /* NB: empoty loop */
  return OK;
}

void deactivate_all_notes(CSOUND *csound)
{
    INSDS *ip = csound->actanchor.nxtact;

    while (ip != NULL) {
      INSDS *nxt = ip->nxtact;
#ifdef BETA
      csoundMessage(csound, "deativate: ip, nxt = %p , %p\n", ip, nxt);
#endif
      xturnoff_now(csound, ip);
      // should not be needed -- if (ip == nxt) break;
      ip = nxt;
    }
}

#ifdef HAVE_PTHREAD_SPIN_LOCK
#define RT_SPIN_TRYLOCK { int trylock = CSOUND_SUCCESS; \
  if(csound->oparms->realtime)             \
    trylock = csoundSpinTryLock(&csound->alloc_spinlock);      \
  if(trylock == CSOUND_SUCCESS) {
#else
#define RT_SPIN_TRYLOCK csoundSpinLock(&csound->alloc_spinlock);
#endif

#ifdef HAVE_PTHREAD_SPIN_LOCK
#define RT_SPIN_UNLOCK \
  if(csound->oparms->realtime) \
    csoundSpinUnLock(&csound->alloc_spinlock); \
  trylock = CSOUND_SUCCESS; } }
#else
#define RT_SPIN_UNLOCK csoundSpinUnLock(&csound->alloc_spinlock);
#endif

/* RM: this now broken out for access from process_rt_event & sensevents -- bv  */
static void process_midi_event(CSOUND *csound, MEVENT *mep, MCHNBLK *chn)
{
  int n, insno = chn->insno;
  if (mep->type == NOTEON_TYPE && mep->dat2) {      /* midi note ON: */
    if (UNLIKELY((n = MIDIinsert(csound, insno, chn, mep)))) {
      /* alloc,init,activ */
      csoundErrorMsg(csound,
                      Str("\t\t   T%7.3f - note deleted. "), csound->curp2);
      {
        char *name = csound->engineState.instrtxtp[insno]->insname;
        if (name)
          csoundErrorMsg(csound, Str("instr %s had %d init errors\n"),
                          name, n);
        else
          csoundErrorMsg(csound, Str("instr %d had %d init errors\n"),
                          insno, n);
      }
      csound->perferrcnt++;
    }
  }
  else {                                          /* else midi note OFF:    */

    INSDS *ip = chn->kinsptr[mep->dat1];
    if (ip == NULL)                               /*  if already off, done  */
      csound->Mxtroffs++;
    else if (chn->sustaining) {                   /*  if sustain pedal on   */
      while (ip != NULL && ip->m_sust)
        ip = ip->nxtolap;
      if (ip != NULL) {
        ip->m_sust = 1;                           /*    let the note ring   */
        chn->ksuscnt++;
      } else csound->Mxtroffs++;
    }
    else xturnoff(csound, ip);                    /*  else some kind of off */
  }
}

/* unlink expired notes from activ chain */
/*      and mark them inactive           */
/*    close any files in each fdchain    */

/* IV - Feb 05 2005: changed to double */

void beatexpire(CSOUND *csound, double beat)
{
  INSDS  *ip;
 strt:
  if ((ip = csound->frstoff) != NULL && ip->offbet <= beat) {
    do {
      if (!ip->relesing && ip->xtratim) {
        /* IV - Nov 30 2002: */
        /*   allow extra time for finite length (p3 > 0) score notes */
        set_xtratim(csound, ip);      /* enter release stage */
        csound->frstoff = ip->nxtoff; /* update turnoff list */
#ifdef BETA
        if (UNLIKELY(csound->oparms->odebug))
          csoundMessage(csound, "Calling schedofftim line %d\n", __LINE__);
#endif
        schedofftim(csound, ip);
        goto strt;                    /* and start again */
      }
      else
        deact(csound, ip);    /* IV - Sep 5 2002: use deact() as it also */
    }                         /* deactivates subinstrument instances */
    while ((ip = ip->nxtoff) != NULL && ip->offbet <= beat);
    csound->frstoff = ip;
    if (UNLIKELY(csound->oparms->odebug)) {
      csoundMessage(csound, "deactivated all notes to beat %7.3f\n", beat);
      csoundMessage(csound, "frstoff = %p\n", (void*) csound->frstoff);
    }
  }
}

static CS_NOINLINE void printScoreError(CSOUND *p, int rtEvt,
                                        const char *fmt, ...)
{
  va_list args;

  if (rtEvt)
    csoundMessage(p, "\t\t   T%7.3f", p->curp2 - p->timeOffs);
  else
    csoundMessage(p, "\t  B%7.3f", p->curbt - p->beatOffs);
  va_start(args, fmt);
  csoundErrMsgV(p, NULL, fmt, args);
  va_end(args);
  p->perferrcnt++;
}

static int process_rt_event(CSOUND *csound, int sensType)
{
  EVTBLK  *evt;
  int     retval, insno, rfd;

  retval = 0;
  if (csound->curp2 * csound->esr < (double)csound->icurTime) {
    csound->curp2 = (double)csound->icurTime/csound->esr;
    //if(sensType != 2)
      print_amp_values(csound, 0);
  }
  if (sensType == 4) {                  /* RM: Realtime orc event   */
    EVTNODE *e = csound->OrcTrigEvts;
    /* RM: Events are sorted on insertion, so just check the first */
    evt = &(e->evt);
    insno = MYFLT2LONG(evt->p[1]);
    if ((rfd = getRemoteInsRfd(csound, insno))) {
      if (rfd == GLOBAL_REMOT)
        insGlobevt(csound, evt);       /* RM: do a global send and allow local */
      else
        insSendevt(csound, evt, rfd);  /* RM: or send to single remote Csound */
      return 0;
    }
    /* pop from the list */
    csound->OrcTrigEvts = e->nxt;
    retval = process_score_event(csound, evt, 1);
    if (evt->strarg != NULL) {
      mfree(csound, evt->strarg);
      evt->strarg = NULL;
    }
    /* push back to free alloc stack so it can be reused later */
    e->nxt = csound->freeEvtNodes;
    csound->freeEvtNodes = e;
  }
  else if (sensType == 2) {                      /* Midievent:    */
    MEVENT *mep;
    MCHNBLK *chn;
    /* realtime or Midifile  */
    mep = csound->midiGlobals->Midevtblk;
    chn = csound->m_chnbp[mep->chan];
    if ((rfd = getRemoteChnRfd(csound, mep->chan+1))) { /* RM: USE CHAN + 1 */
      if (rfd == GLOBAL_REMOT)
        MIDIGlobevt(csound, mep);
      else MIDIsendevt(csound, mep, rfd);
      return 0;
    }
    else  /* RM: this part is broken out  -- bv  */
      process_midi_event(csound, mep, chn);
  }
  return retval;
}

int process_score_event(CSOUND *csound, EVTBLK *evt, int rtEvt)
{
  EVTBLK  *saved_currevent;
  int     insno, rfd, n;

  saved_currevent = csound->currevent;
  csound->currevent = evt;
  switch (evt->opcod) {                       /* scorevt or Linevt:     */
  case 'e':           /* quit realtime */
    csound->event_insert_loop = 0;
    /* fall through */
  case 'l':
  case 's':
    while (csound->frstoff != NULL) {
      INSDS *nxt = csound->frstoff->nxtoff;
      xturnoff_now(csound, csound->frstoff);
      csound->frstoff = nxt;
    }
    csound->currevent = saved_currevent;
    return (evt->opcod == 'l' ? 3 : (evt->opcod == 's' ? 1 : 2));
  case 'q':
    if (isstrcod(evt->p[1]) && evt->strarg) {    /* IV - Oct 31 2002 */
      MYFLT n = named_instr_find(csound, evt->strarg);
      if (UNLIKELY((insno = (int) n) == 0)) {
        printScoreError(csound, rtEvt,
                        Str(" - note deleted. instr %s undefined"),
                        evt->strarg);
        break;
      }
      evt->p[1] = n;
      csoundErrorMsg(csound, Str("Setting instrument %s %s\n"),
                      evt->strarg, (evt->p[3] == 0 ? Str("off") : Str("on")));
      csound->engineState.instrtxtp[insno]->muted = (int16) evt->p[3];
    }
    else {                                        /* IV - Oct 31 2002 */
      insno = abs((int) evt->p[1]);
      if (UNLIKELY((unsigned int)(insno-1) >=
                   (unsigned int) csound->engineState.maxinsno ||
                   csound->engineState.instrtxtp[insno] == NULL)) {
        printScoreError(csound, rtEvt,
                        Str(" - note deleted. instr %d(%d) undefined"),
                        insno, csound->engineState.maxinsno);
        break;
      }
      csoundErrorMsg(csound, Str("Setting instrument %d %s\n"),
                      insno, (evt->p[3] == 0 ? Str("off") : (Str("on"))));
      csound->engineState.instrtxtp[insno]->muted = (int16) evt->p[3];
    }
    break;
  case 'i':
  case 'd':
    if (isstrcod(evt->p[1]) && evt->strarg) {    /* IV - Oct 31 2002 */
      MYFLT n = named_instr_find(csound, evt->strarg);
      if (UNLIKELY((insno = (int)n) == 0)) {
        printScoreError(csound, rtEvt,
                        Str(" - note deleted. instr %s undefined"),
                        evt->strarg);
        break;
      }
      evt->p[1] = n;
      if (insno<0) {
        evt->p[1] = insno; insno = -insno;
      }
      else if (evt->opcod=='d') evt->p[1]=-insno;
      if ((rfd = getRemoteInsRfd(csound, insno))) {
        /* RM: if this note labeled as remote */
        if (rfd == GLOBAL_REMOT)
          insGlobevt(csound, evt);  /* RM: do a global send and allow local */
        else {
          insSendevt(csound, evt, rfd);/* RM: or send to single remote Csound */
          break;                       /* RM: and quit */
        }
      }
      evt->p[1] = (MYFLT) insno;
      if (csound->oparms->Beatmode && !rtEvt && evt->p3orig > FL(0.0))
        evt->p[3] = evt->p3orig * (MYFLT) csound->ibeatTime/csound->esr;
      /* else alloc, init, activate */
      if (UNLIKELY((n = insert(csound, insno, evt)))) {
        printScoreError(csound, rtEvt,
                        Str(" - note deleted.  i%d (%s) had %d init errors"),
                        insno, evt->strarg, n);
      }
    }
    else {                                        /* IV - Oct 31 2002 */
      insno = abs((int) evt->p[1]);
      if (UNLIKELY((unsigned int)(insno-1) >=
                   (unsigned int)csound->engineState.maxinsno ||
                   csound->engineState.instrtxtp[insno] == NULL)) {
        printScoreError(csound, rtEvt,
                        Str(" - note deleted. instr %d(%d) undefined"),
                        insno, csound->engineState.maxinsno);
        break;
      }
      if ((rfd = getRemoteInsRfd(csound, insno))) {
        /* RM: if this note labeled as remote  */
        if (rfd == GLOBAL_REMOT)
          insGlobevt(csound, evt);    /* RM: do a global send and allow local */
        else {
          insSendevt(csound, evt, rfd);/* RM: or send to single remote Csound */
          break;                      /* RM: and quit              */
        }
      }
      if (evt->p[1] < FL(0.0))         /* if p1 neg,             */
        infoff(csound, -evt->p[1]);    /*  turnoff any infin cpy */
      else {
        if (csound->oparms->Beatmode && !rtEvt && evt->p3orig > FL(0.0))
          evt->p[3] = evt->p3orig * (MYFLT) csound->ibeatTime/csound->esr;
        if (UNLIKELY((n = insert(csound, insno, evt)))) {
          /* else alloc, init, activate */
          printScoreError(csound, rtEvt,
                          Str(" - note deleted.  i%d had %d init errors"),
                          insno, n);
        }
      }
    }
    break;
  case 'f':                   /* f event: */
    {
      FUNC  *dummyftp;
      hfgens(csound, &dummyftp, evt, 0); /* construct locally */
      if (getRemoteInsRfdCount(csound))
        insGlobevt(csound, evt); /* RM: & optionally send to all remotes      */
    }
    break;
  case 'a':
    {
      int64_t kCnt;
      kCnt = (int64_t) ((double) csound->ekr * (double) evt->p[3] + 0.5);
      if (kCnt > csound->advanceCnt) {
        csound->advanceCnt = kCnt;
        csoundErrorMsg(csound,
                        Str("time advanced %5.3f beats by score request\n"),
                        evt->p3orig);
      }
    }
    break;
  }
  csound->currevent = saved_currevent;
  return 0;
}

#define RNDINT64(x) ((int64_t) ((double) (x) + ((double) (x) < 0.0 ? -0.5 : 0.5)))

void delete_pending_rt_events(CSOUND *csound)
{
  EVTNODE *ep = csound->OrcTrigEvts;

  while (ep != NULL) {
    EVTNODE *nxt = ep->nxt;
    if (ep->evt.strarg != NULL) {
      mfree(csound,ep->evt.strarg);
      ep->evt.strarg = NULL;
    }
    /* push to stack of free event nodes */
    ep->nxt = csound->freeEvtNodes;
    csound->freeEvtNodes = ep;
    ep = nxt;
  }
  csound->OrcTrigEvts = NULL;
}

/* Update overall amplitudes from section values, */
/* and optionally print message (1: section end, 2: lplay end). */

void section_amps(CSOUND *csound, int enable_msgs)
{
  CSOUND        *p = csound;
  MYFLT         *maxp, *smaxp;
  uint32        *maxps, *smaxps;
  int32         *rngp, *srngp;
  int           n;

  if (enable_msgs) {
    if (enable_msgs == 1)
      csoundMessage(p, Str("end of section %d\t sect peak amps:"), STA_MUSMON(sectno));
    else if (enable_msgs == 2)
      csoundMessage(p, Str("end of lplay event list\t      peak amps:"));
    for (n = p->nchnls, maxp = p->smaxamp; n--; )
      print_maxamp(p, *maxp++);               /* IV - Jul 9 2002 */
    csoundMessage(p, "\n");
    if (UNLIKELY(STA_MUSMON(srngflg))) {
      csoundMessage(p, Str("\t number of samples out of range:"));
      for (n = p->nchnls, srngp = STA_MUSMON(srngcnt); n--; )
        csoundMessage(p, "%9d", *srngp++);
      csoundMessage(p, "\n");
    }
  }
  STA_MUSMON(srngflg) = 0;
  for (n = p->nchnls,
         smaxp = p->smaxamp - 1, maxp = p->omaxamp - 1,
         smaxps = p->smaxpos - 1, maxps = p->omaxpos - 1,
         srngp = STA_MUSMON(srngcnt), rngp = STA_MUSMON(orngcnt); n--; ) {
    ++maxps; ++smaxps;
    if (UNLIKELY(*++smaxp > *++maxp)) {
      *maxp = *smaxp;                 /* keep ovrl maxamps */
      *maxps = *smaxps;               /* And where */
    }
    *smaxp = FL(0.0);
    *smaxps = 0;
    *rngp++ += *srngp;                /*   and orng counts */
    *srngp++ = 0;
  }
}

/* sense events for one k-period            */
/* return value is one of the following:    */
/*   0: continue performance                */
/*   1: terminate (e.g. end of MIDI file)   */
/*   2: normal end of score                 */
int sensevents(CSOUND *csound)
{
  EVTBLK  *e;
  OPARMS  *O = csound->oparms;
  int     retval =  0, sensType;
  int     conn, *sinp, end_check=1;

  csdebug_data_t *data = (csdebug_data_t *) csound->csdebug_data;
  if (UNLIKELY(data && data->status == CSDEBUG_STATUS_STOPPED)) {
    return 0; /* don't process events if we're in debug mode and stopped */
  }
  if (UNLIKELY(csound->MTrkend && O->termifend)) {   /* end of MIDI file:  */
    deactivate_all_notes(csound);
    csoundErrorMsg(csound, Str("terminating.\n"));
    return 1;                         /* abort with perf incomplete */
  }
  /* if turnoffs pending, remove any expired instrs */
  RT_SPIN_TRYLOCK
  if (UNLIKELY(csound->frstoff != NULL)) {
    double  tval;
    /* the following comparisons must match those in schedofftim() */
    if (O->Beatmode) {
      tval = csound->curBeat + (0.505 * csound->curBeat_inc);
      if (csound->frstoff->offbet <= tval) beatexpire(csound, tval);
    }
    else {
      tval = ((double)csound->icurTime + csound->ksmps * 0.505)/csound->esr;
      if (csound->frstoff->offtim <= tval)
        timexpire(csound, tval);
    }
  }
  RT_SPIN_UNLOCK

  e = &(csound->evt);
  if (--(csound->cyclesRemaining) <= 0) { /* if done performing score segment: */
    if (!csound->cyclesRemaining) {
      csound->prvbt = csound->curbt;      /* update beats and times */
      csound->curbt = csound->nxtbt;
      csound->curp2 = csound->nxtim;
      print_amp_values(csound, 1);        /* print amplitudes for this segment */
    }
    else                                  /* this should only happen at */
      csound->cyclesRemaining = 0;        /* beginning of performance   */
  }

 retest:
  /* in daemon mode, we will ignore the end of
     the score, but allow for a realtime event
     to stop Csound */
    while (csound->cyclesRemaining <= 0 &&
           (e->opcod != 'e' || !csound->oparms->daemon)){
      /* read each score event:     */
    if (e->opcod != '\0') {
      /* if there is a pending score event, handle it now */
      switch (e->opcod) {
      case 'e':                     /* end of score, */
      case 'l':                     /* lplay list,   */
      case 's':                     /* or section:   */
        if (csound->frstoff != NULL) {    /* if still have notes
                                             with finite length, wait
                                             until all are turned off */
          RT_SPIN_TRYLOCK
          csound->nxtim = csound->frstoff->offtim;
          csound->nxtbt = csound->frstoff->offbet;
          RT_SPIN_UNLOCK
          break;
        }
        /* end of: 1: section, 2: score, 3: lplay list */
        retval = (e->opcod == 'l' ? 3 : (e->opcod == 's' ? 1 : 2));
        if(csound->oparms->realtime && end_check == 1) {
          csoundSleep(5); // wait for 5ms for any first events to go through
          end_check = 0;  // reset first time check
          goto retest;    // loop back
        }
        goto scode;
      default:                            /* q, i, f, a:              */
        process_score_event(csound, e, 0);/*   handle event now       */
        e->opcod = '\0';                  /*   and get next one       */
        continue;
      }
    }
    else {
      /* else read next score event */
      if (UNLIKELY(O->usingcscore)) {       /*    get next lplay event  */
        /* FIXME: this may be non-portable */
        if (STA_MUSMON(ep) < STA_MUSMON(epend))           /* nxt event    */
          memcpy((void*) e, (void*) &((*STA_MUSMON(ep)++)->strarg), sizeof(EVTBLK));
        else                                /* else lcode   */
          memcpy((void*) e, (void*) &(STA_MUSMON(lsect)->strarg), sizeof(EVTBLK));
      } else
        if (!(rdscor(csound, e))){
          /* or rd nxt evt from scstr */
          e->opcod = 'e';
        }
      csound->currevent = e;

      switch (e->opcod) {
      case 'w':
        if (!O->Beatmode)                   /* Not beatmode: read 'w' */
          settempo(csound, (double)e->p2orig); /* to init the tempo   */
        continue;                           /*   for this section     */
      case 'q':
      case 'i':
      case 'd':
      case 'f':
      case 'a':
        csound->nxtim = (double) e->p[2] + csound->timeOffs;
        csound->nxtbt = (double) e->p2orig + csound->beatOffs;
        if (e->opcod=='i'||e->opcod=='d')
          if (UNLIKELY(csound->oparms->odebug))
            csoundErrorMsg(csound, "new event: %16.13lf %16.13lf\n",
                            csound->nxtim, csound->nxtbt);
        break;
      case 'e':
      case 'l':
      case 's':
        continue;
      default:
        csoundErrorMsg(csound,
                        Str("error in score.  illegal opcode %c (ASCII %d)\n"),
                        e->opcod, e->opcod);
        csound->perferrcnt++;
        continue;
      }
    }
    /* calculate the number of k-periods remaining until next event */
    if (!O->sampleAccurate) {
      if (O->Beatmode)
        csound->cyclesRemaining =
          RNDINT64((csound->nxtbt - csound->curBeat) / csound->curBeat_inc);
      else {
        csound->cyclesRemaining =
          RNDINT64((csound->nxtim*csound->esr - csound->icurTime)/csound->ksmps);
        csound->nxtim =
          (csound->cyclesRemaining*csound->ksmps+csound->icurTime)/csound->esr;
      }
    }
    else {
      /* VL 30-11-2012
         new code for sample-accurate timing needs to truncate cyclesRemaining
      */
      if (O->Beatmode)
        csound->cyclesRemaining = (int64_t)
          ((csound->nxtbt - csound->curBeat) / csound->curBeat_inc);
      else {
        csound->cyclesRemaining = (int64_t)
          FLOOR((csound->nxtim*csound->esr -
                 csound->icurTime+csound->onedsr*0.5) / csound->ksmps);
        csound->nxtim =
          (csound->cyclesRemaining*csound->ksmps+csound->icurTime)/csound->esr;
      }
    }
  }

  /* handle any real time events now: */
  /* FIXME: the initialisation pass of real time */
  /*   events is not sorted by instrument number */
  /*   (although it never was sorted anyway...)  */
  if (UNLIKELY(O->RTevents || getRemoteSocksIn(csound))) {
    int nrecvd;
    /* run all registered callback functions */
    if (csound->evtFuncChain != NULL && !csound->advanceCnt) {
      EVT_CB_FUNC *fp = (EVT_CB_FUNC*) csound->evtFuncChain;
      do {
        fp->func(csound, fp->userData);
        fp = fp->nxt;
      } while (fp != NULL);
    }

    /* check for pending real time events */
    while (csound->OrcTrigEvts != NULL &&
           csound->OrcTrigEvts->start_kcnt <=
           (uint32) csound->global_kcounter) {

      if ((retval = process_rt_event(csound, 4)) != 0){
        goto scode;
      }
    }
    /* RM */
    if ((sinp = getRemoteSocksIn(csound))) {
      while ((conn = *sinp++)) {
        while ((nrecvd = SVrecv(csound, conn,
                                (void*)&(csound->SVrecvbuf),
                                sizeof(REMOT_BUF) )) > 0) {
          int lentot = 0;
          do {
            REMOT_BUF *bp = (REMOT_BUF*)((char*)(&(csound->SVrecvbuf))+lentot);

            if (bp->type == SCOR_EVT) {
              EVTBLK *evt = (EVTBLK*)bp->data;
              evt->p[2] = (double)csound->icurTime/csound->esr;
              if ((retval = process_score_event(csound, evt, 1)) != 0) {
                e->opcod = evt->opcod;        /* pass any s, e, or l */

                goto scode;
              }
            }
            else if (bp->type == MIDI_EVT) {
              MEVENT *mep = (MEVENT *)bp->data;
              MCHNBLK *chn = csound->m_chnbp[mep->chan];
              process_midi_event(csound, mep, chn);
            }
            else if (bp->type == MIDI_MSG) {
              MEVENT *mep = (MEVENT *)bp->data;
              if (UNLIKELY(mep->type == 0xFF && mep->dat1 == 0x2F)) {
                csound->MTrkend = 1;                     /* catch a Trkend    */
                csoundErrorMsg(csound, "SERVER%c: ", remoteID(csound));
                csoundErrorMsg(csound, "caught a Trkend\n");
                /*csoundCleanup(csound);
                  exit(0);*/
                return 2;  /* end of performance */
              }
              else m_chanmsg(csound, mep);               /* or a chan msg     */
            }
            lentot+=bp->len;
          } while (lentot < nrecvd);
        }
      }
    }

    /* MIDI note messages */
    if (O->Midiin || O->FMidiin)
      while ((sensType = sensMidi(csound)) != 0)
        if ((retval = process_rt_event(csound, sensType)) != 0) {
          goto scode;
        }
  }
  /* no score event at this time, return to continue performance */

  return 0;
 scode:
  /* end of section (retval == 1), score (retval == 2), */
  /* or lplay list (retval == 3) */
  if (getRemoteInsRfdCount(csound))
    insGlobevt(csound, e);/* RM: send s,e, or l to any remotes */
  e->opcod = '\0';
  if (retval == 3) {
    section_amps(csound, 2);
    return 1;
  }
  /* for s, or e after s */
  if (retval == 1 || (retval == 2 && STA_MUSMON(sectno) > 1)) {
    delete_pending_rt_events(csound);
    if (O->Beatmode)
      csound->curbt = csound->curBeat;
    csound->curp2 = csound->nxtim =
      csound->timeOffs = csound->icurTime/csound->esr;
    csound->prvbt = csound->nxtbt = csound->beatOffs = csound->curbt;
    section_amps(csound, 1);
  }
  else{
    section_amps(csound, 0);
  }
  if (retval == 1) {                        /* if s code,        */
    RT_SPIN_TRYLOCK
    orcompact(csound);                      /*   rtn inactiv spc */
    if (csound->actanchor.nxtact == NULL)   /*   if no indef ins */
      rlsmemfiles(csound);                  /*    purge memfiles */
    csoundErrorMsg(csound, Str("SECTION %d:\n"), ++STA_MUSMON(sectno));
    RT_SPIN_UNLOCK
    goto retest;                            /*   & back for more */
  }

  return retval;                   /* done with entire score */
}

void musmon_rewind_score(CSOUND *csound)
{
  /* deactivate all currently playing notes */
  deactivate_all_notes(csound);
  /* flush any pending real time events */
  delete_pending_rt_events(csound);

  if (csound->global_kcounter != 0L) {
    /* reset score time */
    csound->global_kcounter = csound->kcounter = 0L;
    csound->nxtbt = csound->curbt = csound->prvbt = 0.0;
    csound->nxtim = csound->curp2 = 0.0;
    csound->beatOffs = csound->timeOffs = 0.0;
    csound->curBeat  = 0.0;
    csound->icurTime = 0L;
    csound->cyclesRemaining = 0;
    csound->evt.strarg = NULL;
    csound->evt.scnt = 0;
    csound->evt.opcod  = '\0';
    /* reset tempo */
    if (csound->oparms->Beatmode)
      settempo(csound, csound->oparms->cmdTempo);
    else
      settempo(csound, 60.0);
    /* update section/overall amplitudes, reset to section 1 */
    section_amps(csound, 1);
    STA_MUSMON(sectno) = 1;
    csoundErrorMsg(csound, Str("SECTION %d:\n"), STA_MUSMON(sectno));
  }

  /* apply score offset if non-zero */
  csound->advanceCnt = 0;
  if (csound->csoundScoreOffsetSeconds_ > FL(0.0))
    csoundSetScoreOffsetSeconds(csound, csound->csoundScoreOffsetSeconds_);
  if (csound->scstr)
    corfile_rewind(csound->scstr);
  else csoundWarning(csound, Str("cannot rewind score: no score in memory\n"));
}