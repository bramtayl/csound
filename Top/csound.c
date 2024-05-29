/*
 * C S O U N D
 *
 * An auto-extensible system for making music on computers
 * by means of software alone.
 *
 * Copyright (C) 2001-2006 Michael Gogins, Matt Ingalls, John D. Ramsdell,
 *                         John P. ffitch, Istvan Varga, Victor Lazzarini,
 *                         Steven Yi
 *
 * L I C E N S E
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

//#ifdef __cplusplus
//extern "C" {
//#endif

#if defined(HAVE_UNISTD_H) || defined (__unix) || defined(__unix__)
#include <unistd.h>
#endif

#include "csoundCore_internal.h"
#include "csmodule.h"
#include "corfile.h"
#include "csGblMtx.h"
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
# include <winsock2.h>
# include <windows.h>
#endif
#include <math.h>
#include "oload.h"
#include "fgens.h"
#include "namedins.h"
#include "pvfileio.h"
#include "pvfileio_internal.h"
#include "fftlib.h"
#include "lpred.h"
#include "cs_par_base.h"
#include "cs_par_orc_semantics.h"
#include "namedins.h"
//#include "cs_par_dispatch.h"
#include "find_opcode.h"
#include "csound_orc_semantics_public.h"
#include "musmon.h"
#include "cfgvar.h"
#include "cfgvar_internal.h"
#include "libsnd_u_internal.h"
#include "scsort.h"
#include "memfiles_internal.h"
#include "memalloc_internal.h"
#include "random_internal.h"
#include "zak.h"

#if defined(linux)||defined(__HAIKU__)|| defined(__EMSCRIPTEN__)||defined(__CYGWIN__)
#define PTHREAD_SPINLOCK_INITIALIZER 0
#endif

#include "csound_standard_types.h"

#include "csdebug.h"
#include "csdebug_internal.h"
#include <time.h>
#include "cwindow.h"
#include "csoundCore_internal.h"
#include "threadsafe.h"
#include "linevent_public.h"
#include "memalloc.h"
#include "fgens_public.h"
#include "csound_threads.h"
#include "namedins_public.h"
#include "text.h"
#include "cscore_internal.h"
#include "one_file.h"
#include "csound_internal.h"
#include "threadsafe.h"
#include "memalloc_internal.h"
#include "entry1.h"
#include "random_internal.h"
#include "musmon_internal.h"
#include "find_opcode.h"
#include "memfiles_internal.h"
#include "midifile.h"
#include "libsnd_u_internal.h"
#include "one_file.h"
#include "scsort.h"
#include "aops.h"
#include "midifile.h"
#include "envvar.h"

void SetInternalYieldCallback(CSOUND *, int (*yieldCallback)(CSOUND *));
static void csoundDefaultMessageCallback(CSOUND *, int, const char *, va_list);
static int  defaultCsoundYield(CSOUND *);
static int  csoundDoCallback_(CSOUND *, void *, unsigned int);
static void reset(CSOUND *);
int  csoundPerformKsmpsInternal(CSOUND *csound);
INSTRTXT **csoundGetInstrumentList(CSOUND *csound);

void (*msgcallback_)(CSOUND *, int, const char *, va_list) = NULL;

static void free_opcode_table(CSOUND* csound) {
    int i;
    CS_HASH_TABLE_ITEM* bucket;
    CONS_CELL* head;

    for (i = 0; i < csound->opcodes->table_size; i++) {
      bucket = csound->opcodes->buckets[i];

      while (bucket != NULL) {
        head = bucket->value;
        cs_cons_free_complete(csound, head);
        bucket = bucket->next;
      }
    }

    cs_hash_table_free(csound, csound->opcodes);
}
static void create_opcode_table(CSOUND *csound)
{

    int err;

    if (csound->opcodes != NULL) {
      free_opcode_table(csound);
    }
    csound->opcodes = cs_hash_table_create(csound);

    /* Basic Entry1 stuff */
    err = csoundAppendOpcodes(csound, &(opcodlst_1[0]), -1);

    if (UNLIKELY(err))
      csoundDie(csound, Str("Error allocating opcode list"));

}

#define MAX_MODULES 64

void module_list_add(CSOUND *csound, char *drv, char *type){
    MODULE_INFO **modules =
      (MODULE_INFO **) csoundQueryGlobalVariable(csound, "_MODULES");
    if (modules != NULL){
     int i = 0;
     while (modules[i] != NULL && i < MAX_MODULES) {
       if (!strcmp(modules[i]->module, drv)) return;
       i++;
     }
     modules[i] = (MODULE_INFO *) mmalloc(csound, sizeof(MODULE_INFO));
     strNcpy(modules[i]->module, drv, 11);
     strNcpy(modules[i]->type, type, 11);
    }
}

int csoundGetRandSeed(CSOUND *csound, int which) {
    if (which > 1) return csound->randSeed1;
    else return csound->randSeed2;
}

char *csoundGetStrsets(CSOUND *csound, long p) {
    if (csound->strsets == NULL) return NULL;
    else return csound->strsets[p];
}

int csoundGetStrsmax(CSOUND *csound){
    return csound->strsmax;
}

void csoundGetOParms(CSOUND *csound, OPARMS *p) {
    memcpy(p, csound->oparms, sizeof(OPARMS));
}

int csoundGetDitherMode(CSOUND *csound) {
    return  csound->dither_output;
}

int csoundGetZakBounds(CSOUND *csound, MYFLT **zkstart){
    ZAK_GLOBALS *zz;
    zz = (ZAK_GLOBALS*) csoundQueryGlobalVariable(csound, "_zak_globals");
    if (zz==NULL) {
      *zkstart = NULL;
      return -1;
    }
    *zkstart = zz->zkstart;
    return zz->zklast;
}

int csoundGetZaBounds(CSOUND *csound, MYFLT **zastart){
    ZAK_GLOBALS *zz;
    zz = (ZAK_GLOBALS*) csoundQueryGlobalVariable(csound, "_zak_globals");
    if (zz==NULL) {
      *zastart = NULL;
      return -1;
    }
    *zastart = zz->zastart;
    return zz->zalast;
}

int csoundGetReinitFlag(CSOUND *csound){
    return csound->reinitflag;
}

int csoundGetTieFlag(CSOUND *csound){
    return csound->tieflag;
}

MYFLT csoundSystemSr(CSOUND *csound, MYFLT val) {
  if (val > 0) csound->_system_sr = val;
  return csound->_system_sr;
}

static const CSOUND cenviron_ = {
    /* ------- private data (not to be used by hosts or externals) ------- */
    /* callback function pointers */
    (SUBR) NULL,    /*  first_callback_     */
    (channelCallback_t) NULL,
    (channelCallback_t) NULL,
    csoundDefaultMessageCallback,
    (int (*)(CSOUND *)) NULL,
    (void (*)(CSOUND *, WINDAT *, const char *)) NULL, /* was: MakeAscii,*/
    (void (*)(CSOUND *, WINDAT *windat)) NULL, /* was: DrawAscii,*/
    (void (*)(CSOUND *, WINDAT *windat)) NULL, /* was: KillAscii,*/
    (int (*)(CSOUND *)) NULL, /* was: defaultCsoundExitGraph, */
    defaultCsoundYield,
    cscore_,        /*  cscoreCallback_     */
    (void(*)(CSOUND*, const char*, int, int, int)) NULL, /* FileOpenCallback_ */
    (SUBR) NULL,    /*  last_callback_      */
    /* these are not saved on RESET */
    playopen_dummy,
    rtplay_dummy,
    recopen_dummy,
    rtrecord_dummy,
    rtclose_dummy,
    audio_dev_list_dummy,
    midi_dev_list_dummy,
    csoundDoCallback_,  /*  doCsoundCallback    */
    defaultCsoundYield, /* csoundInternalYieldCallback_*/
    /* end of callbacks */
    (void (*)(CSOUND *)) NULL,                      /*  spinrecv    */
    (void (*)(CSOUND *)) NULL,                      /*  spoutran    */
    (int (*)(CSOUND *, MYFLT *, int)) NULL,         /*  audrecv     */
    (void (*)(CSOUND *, const MYFLT *, int)) NULL,  /*  audtran     */
    NULL,           /*  hostdata            */
    NULL, NULL,     /*  orchname, scorename */
    NULL, NULL,     /*  orchstr, *scorestr  */
    (OPDS*) NULL,   /*  ids                 */
    { (CS_VAR_POOL*)NULL,
      (CS_HASH_TABLE *) NULL,
      (CS_HASH_TABLE *) NULL,
      -1,
      (INSTRTXT**)NULL,
      { NULL,
        {
          0,0,
          NULL, NULL, NULL, NULL,
          0,0,
          NULL,
          0,0},
        0,0,0,
        //0,
        NULL,
        0,
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        0,
        0,
        0,
        FL(0.0),
        NULL,
        NULL,
        0,
        0,
        0
      },
      NULL,
      MAXINSNO,     /* engineState          */
    },
    (INSTRTXT *) NULL, /* instr0  */
    (INSTRTXT**)NULL,  /* dead_instr_pool */
    0,                /* dead_instr_no */
    (TYPE_POOL*)NULL,
    DFLT_KSMPS,     /*  ksmps               */
    DFLT_NCHNLS,    /*  nchnls              */
    -1,             /*  inchns              */
    0L,             /*  kcounter            */
    0L,             /*  global_kcounter     */
    DFLT_SR,        /*  esr                 */
    DFLT_KR,        /*  ekr                 */
    0l,             /*  curTime             */
    0l,             /*  curTime_inc         */
    0.0,            /*  timeOffs            */
    0.0,            /*  beatOffs            */
    0.0,            /*  curBeat             */
    0.0,            /*  curBeat_inc         */
    0L,             /*  beatTime            */
    (EVTBLK*) NULL, /*  currevent           */
    (INSDS*) NULL,  /*  curip               */
    FL(0.0),        /*  cpu_power_busy      */
    (char*) NULL,   /*  xfilename           */
    1,              /*  peakchunks          */
    0,              /*  keep_tmp            */
    (CS_HASH_TABLE*)NULL, /* Opcode hash table */
    0,              /*  nrecs               */
    NULL,           /*  Linepipe            */
    0,              /*  Linefd              */
    NULL,           /*  csoundCallbacks_    */
    (FILE*)NULL,    /*  scfp                */
    (CORFIL*)NULL,  /*  scstr               */
    NULL,           /*  oscfp               */
    { FL(0.0) },    /*  maxamp              */
    { FL(0.0) },    /*  smaxamp             */
    { FL(0.0) },    /*  omaxamp             */
    {0}, {0}, {0},  /*  maxpos, smaxpos, omaxpos */
    NULL, NULL,     /*  scorein, scoreout   */
    NULL,           /*  argoffspace         */
    NULL,           /*  frstoff             */
    NULL,           /*  stdOp_Env           */
    2345678,        /*  holdrand            */
    0,              /*  randSeed1           */
    0,              /*  randSeed2           */
    NULL,           /*  csRandState         */
    NULL,           /*  csRtClock           */
    // 16384,            /*  strVarMaxLen        */
    0,              /*  strsmax             */
    (char**) NULL,  /*  strsets             */
    NULL,           /*  spin                */
    NULL,           /*  spout               */
    NULL,           /*  spout_tmp               */
    0,              /*  nspin               */
    0,              /*  nspout              */
    NULL,           /*  auxspin             */
    (OPARMS*) NULL, /*  oparms              */
    { NULL },       /*  m_chnbp             */
    0,              /*   dither_output      */
    FL(0.0),        /*  onedsr              */
    FL(0.0),        /*  sicvt               */
    FL(-1.0),       /*  tpidsr              */
    FL(-1.0),       /*  pidsr               */
    FL(-1.0),       /*  mpidsr              */
    FL(-1.0),       /*  mtpdsr              */
    FL(0.0),        /*  onedksmps           */
    FL(0.0),        /*  onedkr              */
    FL(0.0),        /*  kicvt               */ 
    0,              /*  reinitflag          */
    0,              /*  tieflag             */
    DFLT_DBFS,      /*  e0dbfs              */
    FL(1.0) / DFLT_DBFS, /* dbfs_to_float ( = 1.0 / e0dbfs) */
    440.0,               /* A4 base frequency */
    NULL,           /*  rtRecord_userdata   */
    NULL,           /*  rtPlay_userdata     */
#if defined(MSVC) ||defined(__POWERPC__) || defined(__APPLE__)
    {0},
#elif defined(__gnu_linux__)
   {{{0}, 0, {0}}},        /*  exitjmp of type jmp_buf */
#else 
   {0},  
#endif 
    NULL,           /*  frstbp              */
    0,              /*  sectcnt             */
    0, 0, 0,        /*  inerrcnt, synterrcnt, perferrcnt */
    /* {NULL}, */   /*  instxtanchor  in engineState */
    {   /*  actanchor           */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    NULL,
    NULL,
    0,
    NULL,
    0,
    0,
    0,
    0,
    0,
    0.0,
    0.0,
    NULL,
    NULL,
    0,
    0,
    FL(0.0),
    FL(0.0), FL(0.0), FL(0.0),
    NULL,
    {FL(0.0), FL(0.0), FL(0.0), FL(0.0)},
   NULL,
   NULL,NULL,
   NULL,
    0,
    0,
    0,
    NULL,
    NULL,
   0,
   0,
   0,
    FL(0.0),
    NULL,
    NULL,
    {NULL, FL(0.0)},
   {NULL, FL(0.0)},
   {NULL, FL(0.0)},
   {NULL, FL(0.0)}
    },
    {0L },          /*  rngcnt              */
    0, 0,           /*  rngflg, multichan   */
    NULL,           /*  evtFuncChain        */
    NULL,           /*  OrcTrigEvts         */
    NULL,           /*  freeEvtNodes        */
    1,              /*  csoundIsScorePending_ */
    0,              /*  advanceCnt          */
    0,              /*  initonly            */
    0,              /*  evt_poll_cnt        */
    0,              /*  evt_poll_maxcnt     */
    0, 0, 0,        /*  Mforcdecs, Mxtroffs, MTrkend */
    NULL,           /*  opcodeInfo  */
    NULL,           /*  flist               */
    0,              /*  maxfnum             */
    NULL,           /*  gensub              */
    GENMAX+1,       /*  genmax              */
    NULL,           /*  namedGlobals        */
    NULL,           /*  cfgVariableDB       */
    FL(0.0), FL(0.0), FL(0.0),  /*  prvbt, curbt, nxtbt */
    FL(0.0), FL(0.0),       /*  curp2, nxtim        */
    0,              /*  cyclesRemaining     */
    { 0, NULL, NULL, '\0', 0, FL(0.0),
      FL(0.0), { FL(0.0) }, {NULL}},   /*  evt */
    NULL,           /*  memalloc_db         */
    (MGLOBAL*) NULL, /* midiGlobals         */
    NULL,           /*  envVarDB            */
    (MEMFIL*) NULL, /*  memfiles            */
    NULL,           /*  pvx_memfiles        */
    0,              /*  FFT_max_size        */
    NULL,           /*  FFT_table_1         */
    NULL,           /*  FFT_table_2         */
    NULL, NULL, NULL, /* tseg, tpsave, unused */
    (MYFLT*) NULL,  /*  gbloffbas           */
    NULL,           /* file_io_thread    */
    0,              /* file_io_start   */
    NULL,           /* file_io_threadlock */
    0,              /* realtime_audio_flag */
    NULL,           /* init pass thread */
    0,              /* init pass loop  */
    NULL,           /* init pass threadlock */
    NULL,           /* API_lock */
    SPINLOCK_INIT, SPINLOCK_INIT, /* spinlocks */
    SPINLOCK_INIT, SPINLOCK_INIT, /* spinlocks */
    NULL, NULL,             /* Delayed messages */
    {
      NULL, NULL, NULL, NULL, /* bp, prvibp, sp, nx */
      0, 0, 0, 0,   /*  op warpin linpos lincnt */
      -FL(1.0), FL(0.0), FL(1.0), /* prvp2 clock_base warp_factor */
      NULL,         /*  curmem              */
      NULL,         /*  memend              */
      NULL,         /*  macros              */
      -1,           /*  next_name           */
      NULL, NULL,   /*  inputs, str         */
      0,0,0,        /*  input_size, input_cnt, pop */
      1,            /*  ingappop            */
      -1,           /*  linepos             */
      {{NULL, 0, 0}}, /* names        */
      {""},         /*  repeat_name_n[RPTDEPTH][NAMELEN] */
      {0},          /*  repeat_cnt_n[RPTDEPTH] */
      {0},          /*  repeat_point_n[RPTDEPTH] */
      1, {NULL}, 0, /*  repeat_inc_n,repeat_mm_n repeat_index */
      "",          /*  repeat_name[NAMELEN] */
      0,0,1,        /*  repeat_cnt, repeat_point, repeat_inc */
      NULL,         /*  repeat_mm */
      0
    },
    {
      NULL,
      NULL, NULL, NULL, /* orcname, sconame, midname */
      0, 0           /* midiSet, csdlinecount */
    },
    {
      NULL, NULL,   /* Linep, Linebufend    */
      0,            /* stdmode              */
      {
        0, NULL, NULL, 0, 0, FL(0.0), FL(0.0), { FL(0.0) },
        {NULL},
      },            /* EVTBLK  prve         */
      NULL,        /* Linebuf              */
      0,            /* linebufsiz */
      NULL, NULL,
      0
    },
    {
      {0,0}, {0,0},  /* srngcnt, orngcnt    */
      0, 0, 0, 0, 0, /* srngflg, sectno, lplayed, segamps, sormsg */
      NULL, NULL,    /* ep, epend           */
      NULL           /* lsect               */
    },
    //NULL,           /*  musmonGlobals       */
    {
      NULL,         /*  outfile             */
      NULL,         /*  infile              */
      NULL,         /*  sfoutname;          */
      NULL,         /*  inbuf               */
      NULL,         /*  outbuf              */
      NULL,         /*  outbufp             */
      0,            /*  inbufrem            */
      0,            /*  outbufrem           */
      0,0,          /*  inbufsiz,  outbufsiz */
      0,            /*  isfopen             */
      0,            /*  osfopen             */
      0,0,          /*  pipdevin, pipdevout */
      1U,           /*  nframes             */
      NULL, NULL,   /*  pin, pout           */
      0,            /*dither                */
    },
    0,              /*  warped              */
    0,              /*  sstrlen             */
    (char*) NULL,   /*  sstrbuf             */
    1,              /*  enableMsgAttr       */
    0,              /*  sampsNeeded         */
    FL(0.0),        /*  csoundScoreOffsetSeconds_   */
    -1,             /*  inChar_             */
    0,              /*  isGraphable_        */
    0,              /*  delayr_stack_depth  */
    NULL,           /*  first_delayr        */
    NULL,           /*  last_delayr         */
    { 0L, 0L, 0L, 0L, 0L, 0L },     /*  revlpsiz    */
    0L,             /*  revlpsum            */
    0.5,            /*  rndfrac             */
    NULL,           /*  logbase2            */
    NULL, NULL,     /*  omacros, smacros    */
    NULL,           /*  namedgen            */
    NULL,           /*  open_files          */
    NULL,           /*  searchPathCache     */
    NULL,           /*  sndmemfiles         */
    NULL,           /*  reset_list          */
    NULL,           /*  pvFileTable         */
    0,              /*  pvNumFiles          */
    0,              /*  pvErrorCode         */
    //    NULL,           /*  pluginOpcodeFiles   */
    0,              /*  enableHostImplementedAudioIO  */
    0,              /* MIDI IO */
    0,              /*  hostRequestedBufferSize       */
    0,              /*  engineStatus         */
    0,              /*  stdin_assign_flg    */
    0,              /*  stdout_assign_flg   */
    0,              /*  orcname_mode        */
    0,              /*  use_only_orchfile   */
    NULL,           /*  csmodule_db         */
    (char*) NULL,   /*  dl_opcodes_oplibs   */
    (char*) NULL,   /*  SF_csd_licence      */
    (char*) NULL,   /*  SF_id_title         */
    (char*) NULL,   /*  SF_id_copyright     */
    -1,             /*  SF_id_scopyright    */
    (char*) NULL,   /*  SF_id_software      */
    (char*) NULL,   /*  SF_id_artist        */
    (char*) NULL,   /*  SF_id_comment       */
    (char*) NULL,   /*  SF_id_date          */
    NULL,           /*  utility_db          */
    (int16*) NULL,  /*  isintab             */
    NULL,           /*  lprdaddr            */
    0,              /*  currentLPCSlot      */
    0,              /*  max_lpc_slot        */
    NULL,           /*  chn_db              */
    1,              /*  opcodedirWasOK      */
    0,              /*  disable_csd_options */
    { 0, { 0U } },  /*  randState_          */
    0,              /*  performState        */
    1000,           /*  ugens4_rand_16      */
    1000,           /*  ugens4_rand_15      */
    NULL,           /*  schedule_kicked     */
    (MYFLT*) NULL,  /*  disprep_fftcoefs    */
    NULL,           /*  winEPS_globals      */
    {               /*  oparms_             */
      0,            /*    odebug            */
      0, 1, 1, 0,   /*    sfread, ...       */
      0, 0, 0, 0,   /*    inbufsamps, ...   */
      0,            /*    sfsampsize        */
      1,            /*    displays          */
      1, 0, 135,    /*    graphsoff ...     */
      0, 0,         /*    Beatmode, ...     */
      0, 0,         /*    usingcscore, ...  */
      0, 0, 0, 0,   /*    RTevents, ...     */
      0, 0,         /*    ringbell, ...     */
      0, 0, 0,      /*    rewrt_hdr, ...    */
      0.0,          /*    cmdTempo          */
      0.0f, 0.0f,   /*    sr_override ...   */
      0, 0,     /*    nchnls_override ...   */
      (char*) NULL, (char*) NULL, NULL,
      (char*) NULL, (char*) NULL, (char*) NULL,
      (char*) NULL, (char*) NULL,
      0,            /*    midiKey           */
      0,            /*    midiKeyCps        */
      0,            /*    midiKeyOct        */
      0,            /*    midiKeyPch        */
      0,            /*    midiVelocity      */
      0,            /*    midiVelocityAmp   */
      0,            /*    noDefaultPaths    */
      1,            /*    numThreads        */
      0,            /*    syntaxCheckOnly   */
      1,            /*    useCsdLineCounts  */
      0,            /*    samp acc   */
      0,            /*    realtime  */
      0.0,          /*    0dbfs override */
      0,            /*    no exit on compile error */
      0.4,          /*    vbr quality  */
      0,            /*    ksmps_override */
      0,             /*    fft_lib */
      0,             /* echo */
      0.0,           /* limiter */
      DFLT_SR, DFLT_KR,  /* defaults */
      0  /* mp3 mode */
    },
    {0, 0, {0}}, /* REMOT_BUF */
    NULL,           /* remoteGlobals        */
    0, 0,           /* nchanof, nchanif     */
    NULL, NULL,     /* chanif, chanof       */
    0,              /* multiThreadedComplete */
    NULL,           /* multiThreadedThreadInfo */
    NULL,           /* multiThreadedDag */
    NULL,           /* barrier1 */
    NULL,           /* barrier2 */
    NULL,           /* pointer1 was global_var_lock_root */
    NULL,           /* pointer2 was global_var_lock_cache */
    0,              /* int1 was global_var_lock_count */
    /* statics from cs_par_orc_semantic_analysis */
    NULL,           /* instCurr */
    NULL,           /* instRoot */
    0,              /* inInstr */
    /* new dag model statics */
    1,              /* dag_changed */
    0,              /* dag_num_active */
    NULL,           /* dag_task_map */
    NULL,           /* dag_task_status */
    NULL,           /* dag_task_watch */
    NULL,           /* dag_wlmm */
    NULL,           /* dag_task_dep */
    100,            /* dag_task_max_size */
    0,              /* tempStatus */
    1,              /* orcLineOffset */
    0,              /* scoLineOffset */
    NULL,           /* csdname */
    0,              /*  parserNamedInstrFlag */
    0,              /*  tran_nchnlsi */
    0,              /* Count of score strings */
    0,              /* length of current strings space */
    NULL,           /* sinetable */
    16384,          /* sinesize */
    NULL,           /* unused *** pow2 table */
    NULL,           /* cps conv table */
    NULL,           /* output of preprocessor */
    NULL,           /* output of preprocessor */
    {NULL},         /* filedir */
    NULL,           /* message buffer struct */
    0,              /* jumpset */
    0,              /* info_message_request */
    0,              /* modules loaded */
    -1,             /* audio system sr */
    0,              /* csdebug_data */
    kperf_nodebug,  /* current kperf function - nodebug by default */
    0,              /* which score parser */
    0,              /* print_version */
    1,              /* inZero */
    NULL,           /* msg_queue */
    0,              /* msg_queue_wget */
    0,              /* msg_queue_wput */
    0,              /* msg_queue_rstart */
    0,              /* msg_queue_items */
    127,            /* aftouch */
    NULL,           /* directory for corfiles */
    NULL,           /* alloc_queue */
    0,              /* alloc_queue_items */
    0,              /* alloc_queue_wp */
    SPINLOCK_INIT,  /* alloc_spinlock */
    NULL,           /* init_event */
    NULL,           /* message string callback */
    NULL,           /* message_string */
    0,              /* message_string_queue_items */
    0,              /* message_string_queue_wp */
    NULL,           /* message_string_queue */
    0,              /* io_initialised */
    NULL,           /* op */
    0,              /* mode */
    NULL,           /* opcodedir */
    NULL           /* score_srt */
};

typedef struct csInstance_s {
    CSOUND              *csound;
    struct csInstance_s *nxt;
} csInstance_t;

/* initialisation state: */
/* 0: not done yet, 1: complete, 2: in progress, -1: failed */
static  volatile  int init_done = 0;
/* chain of allocated Csound instances */
static  volatile  csInstance_t  *instance_list = NULL;
/* non-zero if performance should be terminated now */
static  volatile  int exitNow_ = 0;


#if !defined(_WIN32)
static void destroy_all_instances(void)
{
    volatile csInstance_t *p;

    csoundLock();
    init_done = -1;     /* prevent the creation of any new instances */
    if (instance_list == NULL) {
      csoundUnLock();
      return;
    }
    csoundUnLock();
    csoundSleep(250);
    while (1) {
      csoundLock();
      p = instance_list;
      csoundUnLock();
      if (p == NULL) {
        break;
      }
      csoundDestroy(p->csound);
    }
}
#endif

#if defined(ANDROID) || (!defined(__gnu_linux__) && !defined(SGI) && \
                         !defined(__HAIKU__) && !defined(__BEOS__) && \
                         !defined(__MACH__) && !defined(__EMSCRIPTEN__))

static char *signal_to_string(int sig)
{
    switch(sig) {
#ifdef SIGHUP
    case SIGHUP:
      return "Hangup";
#endif
#ifdef SIGINT
    case SIGINT:
      return "Interrupt";
#endif
#ifdef SIGQUIT
    case SIGQUIT:
      return "Quit";
#endif
#ifdef SIGILL
    case SIGILL:
      return "Illegal instruction";
#endif
#ifdef SIGTRAP
    case SIGTRAP:
      return "Trace trap";
#endif
#ifdef SIGABRT
    case SIGABRT:
      return "Abort";
#endif
#ifdef SIGBUS
    case SIGBUS:
      return "BUS error";
#endif
#ifdef SIGFPE
    case SIGFPE:
      return "Floating-point exception";
#endif
#ifdef SIGUSR1
    case SIGUSR1:
      return "User-defined signal 1";
#endif
#ifdef SIGSEGV
    case SIGSEGV:
      return "Segmentation violation";
#endif
#ifdef SIGUSR2
    case SIGUSR2:
      return "User-defined signal 2";
#endif
#ifdef SIGPIPE
    case SIGPIPE:
      return "Broken pipe";
#endif
#ifdef SIGALRM
    case SIGALRM:
      return "Alarm clock";
#endif
#ifdef SIGTERM
    case SIGTERM:
      return "Termination";
#endif
#ifdef SIGSTKFLT
    case SIGSTKFLT:
      return "???";
#endif
#ifdef SIGCHLD
    case SIGCHLD:
      return "Child status has changed";
#endif
#ifdef SIGCONT
    case SIGCONT:
      return "Continue";
#endif
#ifdef SIGSTOP
    case SIGSTOP:
      return "Stop, unblockable";
#endif
#ifdef SIGTSTP
    case SIGTSTP:
      return "Keyboard stop";
#endif
#ifdef SIGTTIN
    case SIGTTIN:
      return "Background read from tty";
#endif
#ifdef SIGTTOU
    case SIGTTOU:
      return "Background write to tty";
#endif
#ifdef SIGURG
    case SIGURG:
      return "Urgent condition on socket ";
#endif
#ifdef SIGXCPU
    case SIGXCPU:
      return "CPU limit exceeded";
#endif
#ifdef SIGXFSZ
    case SIGXFSZ:
      return "File size limit exceeded ";
#endif
#ifdef SIGVTALRM
    case SIGVTALRM:
      return "Virtual alarm clock ";
#endif
#ifdef SIGPROF
    case SIGPROF:
      return "Profiling alarm clock";
#endif
#ifdef SIGWINCH
    case SIGWINCH:
      return "Window size change ";
#endif
#ifdef SIGIO
    case SIGIO:
      return "I/O now possible";
#endif
#ifdef SIGPWR
    case SIGPWR:
      return "Power failure restart";
#endif
    default:
      return "???";
    }
}

#ifdef ANDROID
static void psignal_(int sig, char *str)
{
    fprintf(stderr, "%s: %s\n", str, signal_to_string(sig));
}
#else
# if !defined(__CYGWIN__)
void psignal(int sig, const char *str)
{
    fprintf(stderr, "%s: %s\n", str, signal_to_string(sig));
}
# endif
#endif
#elif defined(__BEOS__)
static void psignal_(int sig, char *str)
{
    fprintf(stderr, "%s: %s\n", str, strsignal(sig));
}
#endif

static void signal_handler(int sig)
{
#if defined(HAVE_EXECINFO) && !defined(ANDROID) && !defined(NACL)
    #include <execinfo.h>

    {
      int j, nptrs;
#define SIZE 100
      void *buffer[SIZE];
      char **strings;

      nptrs = backtrace(buffer, SIZE);
      printf("backtrace() returned %d addresses\n", nptrs);

      /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
         would produce similar output to the following: */

      strings = backtrace_symbols(buffer, nptrs);
      if (UNLIKELY(strings == NULL)) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
      }

      for (j = 0; j < nptrs; j++)
        printf("%s\n", strings[j]);

      free(strings);
    }
#endif

#if defined(SIGPIPE)
    if (sig == (int) SIGPIPE) {
#ifdef ANDROID
      psignal_(sig, "Csound ignoring SIGPIPE");
#else
      psignal(sig, "Csound ignoring SIGPIPE");
#endif
      return;
    }
#endif
#ifdef ANDROID
    psignal_(sig, "Csound tidy up");
#else
    psignal(sig, "Csound tidy up");
#endif
    if ((sig == (int) SIGINT || sig == (int) SIGTERM) && !exitNow_) {
      exitNow_ = -1;
      return;
    }
    exit(1);
}

static const int sigs[] = {
#if defined(__gnu_linux__) || defined(SGI) || defined(sol) || defined(__MACH__)
  SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGIOT, SIGBUS,
  SIGFPE, SIGSEGV, SIGPIPE, SIGTERM, SIGXCPU, SIGXFSZ,
#elif defined(_WIN32)
  SIGINT, SIGILL, SIGABRT, SIGFPE, SIGSEGV, SIGTERM,
#elif defined(__EMX__)
  SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGBUS, SIGFPE,
  SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE, SIGTERM, SIGCHLD,
#endif
  -1
};

static void install_signal_handler(void)
{
    unsigned int i;
    for (i = 0; sigs[i] >= 0; i++) {
      signal(sigs[i], signal_handler);
    }
}

static int getTimeResolution(void);

PUBLIC int csoundInitialize(int flags)
{
    int     n;

    do {
      csoundLock();
      n = init_done;
      switch (n) {
      case 2:
        csoundUnLock();
        csoundSleep(1);
      case 0:
        break;
      default:
        csoundUnLock();
        return n;
      }
    } while (n);
    init_done = 2;
    csoundUnLock();
    if (getTimeResolution() != 0) {
      csoundLock();
      init_done = -1;
      csoundUnLock();
      return -1;
    }
    if (!(flags & CSOUNDINIT_NO_SIGNAL_HANDLER)) {
      install_signal_handler();
    }
#if !defined(_WIN32)
    if (!(flags & CSOUNDINIT_NO_ATEXIT))
      atexit(destroy_all_instances);
#endif
    csoundLock();
    init_done = 1;
    csoundUnLock();
    return 0;
  }

static char *opcodedir = NULL;

PUBLIC void csoundSetOpcodedir(const char *s) {
  if(opcodedir != NULL) free(opcodedir);
  opcodedir = strdup(s);
}


PUBLIC CSOUND *csoundCreate(void *hostdata)
{
    CSOUND        *csound;
    csInstance_t  *p;
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    if (init_done != 1) {
      if (csoundInitialize(0) < 0) return NULL;
    }
    csound = (CSOUND*) malloc(sizeof(CSOUND));
    if (UNLIKELY(csound == NULL)) return NULL;
    memcpy(csound, &cenviron_, sizeof(CSOUND));
    init_getstring(csound);
    csound->oparms = &(csound->oparms_);
    csound->hostdata = hostdata;
    p = (csInstance_t*) malloc(sizeof(csInstance_t));
    if (UNLIKELY(p == NULL)) {
      free(csound);
      return NULL;
    }
    csoundLock();
    p->csound = csound;
    p->nxt = (csInstance_t*) instance_list;
    instance_list = p;
    csoundUnLock();
    csoundReset(csound);
    csound->API_lock = csoundCreateMutex(1);
    allocate_message_queue(csound);
    /* NB: as suggested by F Pinot, keep the
       address of the pointer to CSOUND inside
       the struct, so it can be cleared later */
    //csound->self = &csound;
    return csound;
}

/*
PUBLIC int csoundQueryInterface(const char *name, void **iface, int *version)
{
    if (strcmp(name, "CSOUND") != 0)
      return 1;
    *iface = csoundCreate(NULL);
    *version = csoundGetAPIVersion();
    return 0;
}
*/

typedef struct CsoundCallbackEntry_s CsoundCallbackEntry_t;

struct CsoundCallbackEntry_s {
    unsigned int  typeMask;
    CsoundCallbackEntry_t *nxt;
    void    *userData;
    int     (*func)(void *, void *, unsigned int);
};

PUBLIC void csoundDestroy(CSOUND *csound)
{
    csInstance_t  *p, *prv = NULL;

    csoundLock();
    p = (csInstance_t*) instance_list;
    while (p != NULL && p->csound != csound) {
      prv = p;
      p = p->nxt;
    }
    if (p == NULL) {
      csoundUnLock();
      return;
    }
    if (prv == NULL)
      instance_list = p->nxt;
    else
      prv->nxt = p->nxt;
    csoundUnLock();
    free(p);

    reset(csound);

    if (csound->csoundCallbacks_ != NULL) {
      CsoundCallbackEntry_t *pp, *nxt;
      pp = (CsoundCallbackEntry_t*) csound->csoundCallbacks_;
      do {
        nxt = pp->nxt;
        free((void*) pp);
        pp = nxt;
      } while (pp != (CsoundCallbackEntry_t*) NULL);
    }
    if (csound->API_lock != NULL) {
      //csoundLockMutex(csound->API_lock);
      csoundDestroyMutex(csound->API_lock);
    }
    /* clear the pointer */
    // *(csound->self) = NULL;
    free((void*) csound);
}

PUBLIC int csoundGetVersion(void)
{
    return (int) (CS_VERSION * 1000 + CS_SUBVER * 10 + CS_PATCHLEVEL);
}

PUBLIC int csoundGetAPIVersion(void)
{
    return CS_APIVERSION * 100 + CS_APISUBVER;
}

PUBLIC void *csoundGetHostData(CSOUND *csound)
{
    return csound->hostdata;
}

PUBLIC void csoundSetHostData(CSOUND *csound, void *hostData)
{
    csound->hostdata = hostData;
}

/*
 * PERFORMANCE
 */

#ifdef PARCS
/**
 * perform currently active instrs for one kperiod
 *      & send audio result to output buffer
 * returns non-zero if this kperiod was skipped
 */
static int getThreadIndex(CSOUND *csound, void *threadId)
{
    int index = 0;
    THREADINFO *current = csound->multiThreadedThreadInfo;

    if (current == NULL) {
      return -1;
    }

    while (current != NULL) {
#ifdef HAVE_PTHREAD
      if (pthread_equal(*(pthread_t *)threadId, *(pthread_t *)current->threadId))
#elif defined(_WIN32)
      DWORD* d = (DWORD*)threadId;
      if (*d == GetThreadId((HANDLE)current->threadId))
#else
      // FIXME - need to verify this works...
      if (threadId == current->threadId)
#endif
        return index;

      index++;
      current = current->next;
    }
    return -1;
}
#endif

#if 0
static int getNumActive(INSDS *start, INSDS *end)
{
    INSDS *current = start;
    int counter = 1;
    while (((current = current->nxtact) != NULL) && current != end) {
      counter++;
    }
    return counter;
}
#endif

inline void advanceINSDSPointer(INSDS ***start, int num)
{
    int i;
    INSDS *s = **start;

    if (s == NULL) return;
    for (i = 0; i < num; i++) {
      s = s->nxtact;

      if (s == NULL) {
        **start = NULL;
        return;
      }
    }
    **start = s;
}

#ifdef PARCS
unsigned long kperfThread(void * cs)
{
    //INSDS *start;
    CSOUND *csound = (CSOUND *)cs;
    void *threadId;
    int index;
    int numThreads;
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    csoundWaitBarrier(csound->barrier2);

    threadId = csoundGetCurrentThreadId();
    index = getThreadIndex(csound, threadId);
    numThreads = csound->oparms->numThreads;
    //start = NULL;
    csoundMessage(csound,
                    Str("Multithread performance:thread %d of "
                        "%d starting.\n"),
                    /* start ? start->insno : */
                    index+1,
                    numThreads);
    if (UNLIKELY(index < 0)) {
      csoundDie(csound, Str("Bad ThreadId"));
      return ULONG_MAX;
    }
    index++;

    while (1) {

      csoundWaitBarrier(csound->barrier1);

      // FIXME:PTHREAD_WORK - need to check if this is necessary and, if so,
      // use some other kind of locking mechanism as it isn't clear why a
      //global mutex would be necessary versus a per-CSOUND instance mutex
      /*csound_global_mutex_lock();*/
      if (csound->multiThreadedComplete == 1) {
        /*csound_global_mutex_unlock();*/
        free(threadId);
        return 0UL;
      }
      /*csound_global_mutex_unlock();*/

      nodePerf(csound, index, numThreads);

      csoundWaitBarrier(csound->barrier2);
    }
}
#endif

PUBLIC int csoundPerformKsmps(CSOUND *csound)
{
    int done;
    /* VL: 1.1.13 if not compiled (csoundStart() not called)  */
    if (UNLIKELY(!(csound->engineStatus & CS_STATE_COMP))) {
      csoundWarning(csound,
                      Str("Csound not ready for performance: csoundStart() "
                          "has not been called\n"));
      return CSOUND_ERROR;
    }
    if (csound->jumpset == 0) {
      int returnValue;
      csound->jumpset = 1;
      /* setup jmp for return after an exit() */
      if (UNLIKELY((returnValue = setjmp(csound->exitjmp))))
        return ((returnValue - CSOUND_EXITJMP_SUCCESS) | CSOUND_EXITJMP_SUCCESS);
    }
    if(!csound->oparms->realtime) // no API lock in realtime mode
      csoundLockMutex(csound->API_lock);
    do {
      done = sensevents(csound);
      if (UNLIKELY(done)) {
        if(!csound->oparms->realtime) // no API lock in realtime mode
          csoundUnlockMutex(csound->API_lock);
        csoundMessage(csound,
                      Str("Score finished in csoundPerformKsmps() with %d.\n"),
                      done);
        return done;
      }
    } while (csound->kperf(csound));
    if(!csound->oparms->realtime) // no API lock in realtime mode
       csoundUnlockMutex(csound->API_lock);
    return 0;
}

int csoundPerformKsmpsInternal(CSOUND *csound)
{
    int done;
    int returnValue;

    /* VL: 1.1.13 if not compiled (csoundStart() not called)  */
    if (UNLIKELY(!(csound->engineStatus & CS_STATE_COMP))) {
      csoundWarning(csound,
                      Str("Csound not ready for performance: csoundStart() "
                          "has not been called\n"));
      return CSOUND_ERROR;
    }
    /* setup jmp for return after an exit() */
        if (UNLIKELY((returnValue = setjmp(csound->exitjmp)))) {
#ifndef __APPLE__
      csoundMessage(csound, Str("Early return from csoundPerformKsmps().\n"));
#endif
      return ((returnValue - CSOUND_EXITJMP_SUCCESS) | CSOUND_EXITJMP_SUCCESS);
    }
   do {
     if (UNLIKELY((done = sensevents(csound)))) {
       csoundMessage(csound,
                     Str("Score finished in csoundPerformKsmpsInternal().\n"));
        return done;
      }
    } while (csound->kperf(csound));
    return 0;
}

/* external host's outbuffer passed in csoundPerformBuffer() */
PUBLIC int csoundPerformBuffer(CSOUND *csound)
{
    int returnValue;
    int done;
    /* VL: 1.1.13 if not compiled (csoundStart() not called)  */
    if (UNLIKELY(!(csound->engineStatus & CS_STATE_COMP))) {
      csoundWarning(csound,
                      Str("Csound not ready for performance: csoundStart() "
                          "has not been called\n"));
      return CSOUND_ERROR;
    }
    /* Setup jmp for return after an exit(). */
    if (UNLIKELY((returnValue = setjmp(csound->exitjmp)))) {
#ifndef __APPLE__
      csoundMessage(csound, Str("Early return from csoundPerformBuffer().\n"));
#endif
      return ((returnValue - CSOUND_EXITJMP_SUCCESS) | CSOUND_EXITJMP_SUCCESS);
    }
    csound->sampsNeeded += csound->oparms_.outbufsamps;
    while (csound->sampsNeeded > 0) {
     if(!csound->oparms->realtime) {// no API lock in realtime mode
      csoundLockMutex(csound->API_lock);
     }
      do {
        if (UNLIKELY((done = sensevents(csound)))){
          if(!csound->oparms->realtime) // no API lock in realtime mode
            csoundUnlockMutex(csound->API_lock);
          return done;
        }
      } while (csound->kperf(csound));
      if(!csound->oparms->realtime) { // no API lock in realtime mode
       csoundUnlockMutex(csound->API_lock);
      }
      csound->sampsNeeded -= csound->nspout;
    }
    return 0;
}

/* perform an entire score */

PUBLIC int csoundPerform(CSOUND *csound)
{
    int done;
    int returnValue;

   /* VL: 1.1.13 if not compiled (csoundStart() not called)  */
    if (UNLIKELY(!(csound->engineStatus & CS_STATE_COMP))) {
      csoundWarning(csound,
                      Str("Csound not ready for performance: csoundStart() "
                          "has not been called\n"));
      return CSOUND_ERROR;
    }

    csound->performState = 0;
    /* setup jmp for return after an exit() */
    if (UNLIKELY((returnValue = setjmp(csound->exitjmp)))) {
#ifndef __APPLE__
      csoundMessage(csound, Str("Early return from csoundPerform().\n"));
#endif
      return ((returnValue - CSOUND_EXITJMP_SUCCESS) | CSOUND_EXITJMP_SUCCESS);
    }
    do {
        if(!csound->oparms->realtime)
           csoundLockMutex(csound->API_lock);
      do {
        if (UNLIKELY((done = sensevents(csound)))) {
           csoundMessage(csound, Str("Score finished in csoundPerform().\n"));
          if(!csound->oparms->realtime)
            csoundUnlockMutex(csound->API_lock);
          if (csound->oparms->numThreads > 1) {
            csound->multiThreadedComplete = 1;
            csoundWaitBarrier(csound->barrier1);
          }
          return done;
        }
      } while (csound->kperf(csound));
      if(!csound->oparms->realtime)
      csoundUnlockMutex(csound->API_lock);
    } while ((unsigned char) csound->performState == (unsigned char) '\0');
    csoundMessage(csound, Str("csoundPerform(): stopped.\n"));
    csound->performState = 0;
    return 0;
}

/* stop a csoundPerform() running in another thread */

PUBLIC void *csoundGetNamedGens(CSOUND *csound)
{
    return csound->namedgen;
}

PUBLIC void csoundStop(CSOUND *csound)
{
    csound->performState = -1;
}

/*
 * ATTRIBUTES
 */

PUBLIC int64_t csoundGetCurrentTimeSamples(CSOUND *csound){
  return csound->icurTime;
}

PUBLIC MYFLT csoundGetSr(CSOUND *csound)
{
    return csound->esr;
}

PUBLIC MYFLT csoundGetKr(CSOUND *csound)
{
    return csound->ekr;
}

PUBLIC uint32_t csoundGetKsmps(CSOUND *csound)
{
    return csound->ksmps;
}

PUBLIC uint32_t csoundGetNchnls(CSOUND *csound)
{
    return csound->nchnls;
}

PUBLIC uint32_t csoundGetNchnlsInput(CSOUND *csound)
{
  if (csound->inchnls >= 0)
    return (uint32_t) csound->inchnls;
  else return csound->nchnls;
}

PUBLIC MYFLT csoundGet0dBFS(CSOUND *csound)
{
    return csound->e0dbfs;
}

PUBLIC long csoundGetInputBufferSize(CSOUND *csound)
{
    return csound->oparms_.inbufsamps;
}

PUBLIC long csoundGetOutputBufferSize(CSOUND *csound)
{
    return csound->oparms_.outbufsamps;
}

PUBLIC MYFLT *csoundGetSpin(CSOUND *csound)
{
    return csound->spin;
}

PUBLIC void csoundSetSpinSample(CSOUND *csound, int frame,
                                int channel, MYFLT sample)
{
    int index = (frame * csound->inchnls) + channel;
    csound->spin[index] = sample;
}

PUBLIC void csoundClearSpin(CSOUND *csound) {

  memset(csound->spin, 0, sizeof(MYFLT)*csound->ksmps*csound->nchnls);
}

PUBLIC void csoundAddSpinSample(CSOUND *csound, int frame,
                                int channel, MYFLT sample)
{

    int index = (frame * csound->inchnls) + channel;
    csound->spin[index] += sample;
}

PUBLIC MYFLT *csoundGetSpout(CSOUND *csound)
{
    return csound->spout;
}

PUBLIC MYFLT csoundGetSpoutSample(CSOUND *csound, int frame, int channel)
{
    int index = (frame * csound->nchnls) + channel;
    return csound->spout[index];
}

PUBLIC const char *csoundGetOutputName(CSOUND *csound)
{
    return (const char*) csound->oparms_.outfilename;
}

PUBLIC const char *csoundGetInputName(CSOUND *csound)
{
    return (const char*) csound->oparms_.infilename;
}

/**
 * Calling this function with a non-zero will disable all default
 * handling of sound I/O by the Csound library, allowing the host
 * application to use the spin/<spout/input/output buffers directly.
 * If 'bufSize' is greater than zero, the buffer size (-b) will be
 * set to the integer multiple of ksmps that is nearest to the value
 * specified.
 */

PUBLIC void csoundSetHostImplementedAudioIO(CSOUND *csound,
                                            int state, int bufSize)
{
    csound->enableHostImplementedAudioIO = state;
    csound->hostRequestedBufferSize = (bufSize > 0 ? bufSize : 0);
}

PUBLIC void csoundSetHostImplementedMIDIIO(CSOUND *csound,
                                            int state)
{
    csound->enableHostImplementedMIDIIO = state;
}

PUBLIC double csoundGetScoreTime(CSOUND *csound)
{
    double curtime = csound->icurTime;
    double esr = csound->esr;
    return curtime/esr;
}

/*
 * SCORE HANDLING
 */

PUBLIC int csoundIsScorePending(CSOUND *csound)
{
    return csound->csoundIsScorePending_;
}

PUBLIC void csoundSetScorePending(CSOUND *csound, int pending)
{
    csound->csoundIsScorePending_ = pending;
}

PUBLIC void csoundSetScoreOffsetSeconds(CSOUND *csound, MYFLT offset)
{
    double  aTime;
    MYFLT   prv = (MYFLT) csound->csoundScoreOffsetSeconds_;

    csound->csoundScoreOffsetSeconds_ = offset;
    if (offset < FL(0.0))
      return;
    /* if csoundCompile() was not called yet, just store the offset */
    if (!(csound->engineStatus & CS_STATE_COMP))
      return;
    /* otherwise seek to the requested time now */
    aTime = (double) offset - (csound->icurTime/csound->esr);
    if (aTime < 0.0 || offset < prv) {
      csoundRewindScore(csound);    /* will call csoundSetScoreOffsetSeconds */
      return;
    }
    if (aTime > 0.0) {
      EVTBLK  evt;
      memset(&evt, 0, sizeof(EVTBLK));
      evt.strarg = NULL; evt.scnt = 0;
      evt.opcod = 'a';
      evt.pcnt = 3;
      evt.p[2] = evt.p[1] = FL(0.0);
      evt.p[3] = (MYFLT) aTime;
      insert_score_event_at_sample(csound, &evt, csound->icurTime);
    }
}

PUBLIC MYFLT csoundGetScoreOffsetSeconds(CSOUND *csound)
{
    return csound->csoundScoreOffsetSeconds_;
}

PUBLIC void csoundRewindScore(CSOUND *csound)
{
    musmon_rewind_score(csound);
    if (csound->oparms->FMidiname != NULL) midifile_rewind_score(csound);
}

PUBLIC void csoundSetCscoreCallback(CSOUND *p,
                                    void (*cscoreCallback)(CSOUND *))
{
    p->cscoreCallback_ = (cscoreCallback != NULL ? cscoreCallback : cscore_);
}

static void csoundDefaultMessageCallback(CSOUND *csound, int attr,
                                         const char *format, va_list args)
{
#if defined(_WIN32)
    switch (attr & CSOUNDMSG_TYPE_MASK) {
    case CSOUNDMSG_ERROR:
    case CSOUNDMSG_WARNING:
    case CSOUNDMSG_REALTIME:
      vfprintf(stderr, format, args);
      break;
    default:
      vfprintf(stdout, format, args);
    }
#else
    FILE *fp = stderr;
    if ((attr & CSOUNDMSG_TYPE_MASK) == CSOUNDMSG_STDOUT)
      fp = stdout;
    if (!attr || !csound->enableMsgAttr) {
      vfprintf(fp, format, args);
      return;
    }
    if ((attr & CSOUNDMSG_TYPE_MASK) == CSOUNDMSG_ORCH)
      if (attr & CSOUNDMSG_BG_COLOR_MASK)
        fprintf(fp, "\033[4%cm", ((attr & 0x70) >> 4) + '0');
    if (attr & CSOUNDMSG_FG_ATTR_MASK) {
      if (attr & CSOUNDMSG_FG_BOLD)
        fprintf(fp, "\033[1m");
      if (attr & CSOUNDMSG_FG_UNDERLINE)
        fprintf(fp, "\033[4m");
    }
    if (attr & CSOUNDMSG_FG_COLOR_MASK)
      fprintf(fp, "\033[3%cm", (attr & 7) + '0');
    vfprintf(fp, format, args);
    fprintf(fp, "\033[m");
#endif
}

PUBLIC void csoundSetDefaultMessageCallback(
           void (*csoundMessageCallback)(CSOUND *csound,
                                         int attr,
                                         const char *format,
                                         va_list args))
{
    if (csoundMessageCallback) {
      msgcallback_ = csoundMessageCallback;
    } else {
      msgcallback_ = csoundDefaultMessageCallback;
    }
}



PUBLIC void csoundSetMessageStringCallback(CSOUND *csound,
              void (*csoundMessageStrCallback)(CSOUND *csound,
                                            int attr,
                                            const char *str)) {

  if (csoundMessageStrCallback) {
    if(csound->message_string == NULL)
      csound->message_string = (char *) mcalloc(csound, MAX_MESSAGE_STR);
  csound->csoundMessageStringCallback = csoundMessageStrCallback;
  csound->csoundMessageCallback_ = NULL;
  }

}

PUBLIC void csoundSetMessageCallback(CSOUND *csound,
            void (*csoundMessageCallback)(CSOUND *csound,
                                          int attr,
                                          const char *format,
                                          va_list args))
{
    /* Protect against a null callback. */
    if (csoundMessageCallback) {
      csound->csoundMessageCallback_ = csoundMessageCallback;
    } else {
      csound->csoundMessageCallback_ = csoundDefaultMessageCallback;
    }
}

PUBLIC void csoundMessageV(CSOUND *csound,
                           int attr, const char *format, va_list args)
{
  if(!(csound->oparms->msglevel & CS_NOMSG)) {
    if(csound->csoundMessageCallback_) {
      csound->csoundMessageCallback_(csound, attr, format, args);
    } else {
      vsnprintf(csound->message_string, MAX_MESSAGE_STR, format, args);
      csound->csoundMessageStringCallback(csound, attr, csound->message_string);
    }
  }
}

void csoundMessage(CSOUND *csound, const char *format, ...)
{
  if(!(csound->oparms->msglevel & CS_NOMSG)) {
    va_list args;
    va_start(args, format);
    if(csound->csoundMessageCallback_)
      csound->csoundMessageCallback_(csound, 0, format, args);
    else {
      vsnprintf(csound->message_string, MAX_MESSAGE_STR, format, args);
      csound->csoundMessageStringCallback(csound, 0, csound->message_string);
    }
    va_end(args);
  }
}

PUBLIC void csoundMessageS(CSOUND *csound, int attr, const char *format, ...)
{
  if(!(csound->oparms->msglevel & CS_NOMSG)) {
    va_list args;
    va_start(args, format);
    if(csound->csoundMessageCallback_)
      csound->csoundMessageCallback_(csound, attr, format, args);
    else {
      vsnprintf(csound->message_string, MAX_MESSAGE_STR, format, args);
      csound->csoundMessageStringCallback(csound, attr, csound->message_string);
    }
    va_end(args);
    }
}

void csoundDie(CSOUND *csound, const char *msg, ...){
    va_list args;
    va_start(args, msg);
    csoundErrMsgV(csound, (char*) 0, msg, args);
    va_end(args);
    csound->perferrcnt++;
    csoundLongJmp(csound, 1);
}

void csoundWarning(CSOUND *csound, const char *msg, ...)
{
    va_list args;
    if (!(csound->oparms_.msglevel & CS_WARNMSG))
      return;
    csoundMessageS(csound, CSOUNDMSG_WARNING, Str("WARNING: "));
    va_start(args, msg);
    csoundMessageV(csound, CSOUNDMSG_WARNING, msg, args);
    va_end(args);
    csoundMessageS(csound, CSOUNDMSG_WARNING, "\n");
}

void csoundDebugMsg(CSOUND *csound, const char *msg, ...)
{
    va_list args;
    if (!(csound->oparms_.odebug&(~CS_NOQQ)))
      return;
    va_start(args, msg);
    csoundMessageV(csound, 0, msg, args);
    va_end(args);
    csoundMessage(csound, "\n");
}

void csoundErrorMsg(CSOUND *csound, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    csoundMessageV(csound, CSOUNDMSG_ERROR, msg, args);
    va_end(args);
}

void csoundErrMsgV(CSOUND *csound,
                   const char *hdr, const char *msg, va_list args)
{
    if (hdr != NULL)
      csoundMessageS(csound, CSOUNDMSG_ERROR, "%s", hdr);
    csoundMessageV(csound, CSOUNDMSG_ERROR, msg, args);
    csoundMessageS(csound, CSOUNDMSG_ERROR, "\n");

}

void csoundLongJmp(CSOUND *csound, int retval)
{
    int   n = CSOUND_EXITJMP_SUCCESS;

    n = (retval < 0 ? n + retval : n - retval) & (CSOUND_EXITJMP_SUCCESS - 1);
    //printf("**** n = %d\n", n);
    if (!n)
      n = CSOUND_EXITJMP_SUCCESS;

    csound->curip = NULL;
    csound->ids = NULL;
    csound->reinitflag = 0;
    csound->tieflag = 0;
    csound->perferrcnt += csound->inerrcnt;
    csound->inerrcnt = 0;
    csound->engineStatus |= CS_STATE_JMP;
    //printf("**** longjmp with %d\n", n);
    longjmp(csound->exitjmp, n);
}

PUBLIC void csoundSetMessageLevel(CSOUND *csound, int messageLevel)
{
    csound->oparms_.msglevel = messageLevel;
}

PUBLIC int csoundGetMessageLevel(CSOUND *csound)
{
    return csound->oparms_.msglevel;
}

PUBLIC void csoundKeyPress(CSOUND *csound, char c)
{
    csound->inChar_ = (int) ((unsigned char) c);
}

/*
 * CONTROL AND EVENTS
 */

PUBLIC void
csoundSetInputChannelCallback(CSOUND *csound,
                              channelCallback_t inputChannelCalback)
{
    csound->InputChannelCallback_ = inputChannelCalback;
}

PUBLIC void
csoundSetOutputChannelCallback(CSOUND *csound,
                               channelCallback_t outputChannelCalback)
{
    csound->OutputChannelCallback_ = outputChannelCalback;
}

/*
 *    REAL-TIME AUDIO
 */

PUBLIC void csoundSetPlayopenCallback(CSOUND *csound,
                                      int (*playopen__)(CSOUND *,
                                                        const csRtAudioParams
                                                        *parm))
{
    csound->playopen_callback = playopen__;
}

PUBLIC void csoundSetRtplayCallback(CSOUND *csound,
                                    void (*rtplay__)(CSOUND *,
                                                     const MYFLT *outBuf,
                                                     int nbytes))
{
    csound->rtplay_callback = rtplay__;
}

PUBLIC void csoundSetRecopenCallback(CSOUND *csound,
                                     int (*recopen__)(CSOUND *,
                                                      const csRtAudioParams *parm))
{
    csound->recopen_callback = recopen__;
}

PUBLIC void csoundSetRtrecordCallback(CSOUND *csound,
                                      int (*rtrecord__)(CSOUND *,
                                                        MYFLT *inBuf,
                                                        int nbytes))
{
    csound->rtrecord_callback = rtrecord__;
}

PUBLIC void csoundSetRtcloseCallback(CSOUND *csound,
                                     void (*rtclose__)(CSOUND *))
{
    csound->rtclose_callback = rtclose__;
}

PUBLIC void csoundSetAudioDeviceListCallback(CSOUND *csound,
            int (*audiodevlist__)(CSOUND *, CS_AUDIODEVICE *list, int isOutput))
{
    csound->audio_dev_list_callback = audiodevlist__;
}

PUBLIC void csoundSetMIDIDeviceListCallback(CSOUND *csound,
            int (*mididevlist__)(CSOUND *, CS_MIDIDEVICE *list, int isOutput))
{
    csound->midi_dev_list_callback = mididevlist__;
}

PUBLIC int csoundGetAudioDevList(CSOUND *csound,
                                 CS_AUDIODEVICE *list, int isOutput){
  return csound->audio_dev_list_callback(csound,list,isOutput);
}

PUBLIC int csoundGetMIDIDevList(CSOUND *csound,  CS_MIDIDEVICE *list, int isOutput)
{
  return csound->midi_dev_list_callback(csound,list,isOutput);
}

/* Set real time MIDI function pointers. */

PUBLIC void csoundSetExternalMidiInOpenCallback(CSOUND *csound,
                                                int (*func)(CSOUND *,
                                                            void **,
                                                            const char *))
{
    csound->midiGlobals->MidiInOpenCallback = func;
}

PUBLIC void csoundSetExternalMidiReadCallback(CSOUND *csound,
                                              int (*func)(CSOUND *,
                                                          void *,
                                                          unsigned char *, int))
{
    csound->midiGlobals->MidiReadCallback = func;
}

PUBLIC void csoundSetExternalMidiInCloseCallback(CSOUND *csound,
                                                 int (*func)(CSOUND *, void *))
{
    csound->midiGlobals->MidiInCloseCallback = func;
}

PUBLIC void csoundSetExternalMidiOutOpenCallback(CSOUND *csound,
                                                 int (*func)(CSOUND *,
                                                             void **,
                                                             const char *))
{
    csound->midiGlobals->MidiOutOpenCallback = func;
}

PUBLIC void csoundSetExternalMidiWriteCallback(CSOUND *csound,
                                               int (*func)(CSOUND *,
                                                           void *,
                                                           const unsigned char *,
                                                           int))
{
    csound->midiGlobals->MidiWriteCallback = func;
}

PUBLIC void csoundSetExternalMidiOutCloseCallback(CSOUND *csound,
                                                  int (*func)(CSOUND *, void *))
{
    csound->midiGlobals->MidiOutCloseCallback = func;
}

PUBLIC void csoundSetExternalMidiErrorStringCallback(CSOUND *csound,
                                                     const char *(*func)(int))
{
    csound->midiGlobals->MidiErrorStringCallback = func;
}

/*
 *    FUNCTION TABLE DISPLAY.
 */

PUBLIC int csoundSetIsGraphable(CSOUND *csound, int isGraphable)
{
    int prv = csound->isGraphable_;
    csound->isGraphable_ = isGraphable;
    return prv;
}

PUBLIC void csoundSetMakeGraphCallback(CSOUND *csound,
                                       void (*makeGraphCB)(CSOUND *csound,
                                                           WINDAT *windat,
                                                           const char *name))
{
    csound->csoundMakeGraphCallback_ = makeGraphCB;
}

PUBLIC void csoundSetDrawGraphCallback(CSOUND *csound,
                                       void (*drawGraphCallback)(CSOUND *csound,
                                                                 WINDAT *windat))
{
    csound->csoundDrawGraphCallback_ = drawGraphCallback;
}

PUBLIC void csoundSetKillGraphCallback(CSOUND *csound,
                                       void (*killGraphCallback)(CSOUND *csound,
                                                                 WINDAT *windat))
{
    csound->csoundKillGraphCallback_ = killGraphCallback;
}


PUBLIC void csoundSetExitGraphCallback(CSOUND *csound,
                                       int (*exitGraphCallback)(CSOUND *))
{
    csound->csoundExitGraphCallback_ = exitGraphCallback;
}

/*
 * OPCODES
 */
//void add_to_symbtab(CSOUND *csound, OENTRY *ep);

static CS_NOINLINE int opcode_list_new_oentry(CSOUND *csound,
                                              const OENTRY *ep)
{
    CONS_CELL *head;
    OENTRY *entryCopy;
    char *shortName;

    if (UNLIKELY(ep->opname == NULL || csound->opcodes == NULL))
      return CSOUND_ERROR;

    shortName = get_opcode_short_name(csound, ep->opname);

    head = cs_hash_table_get(csound, csound->opcodes, shortName);
    entryCopy = mmalloc(csound, sizeof(OENTRY));
    //printf("%p\n", entryCopy);
    memcpy(entryCopy, ep, sizeof(OENTRY));
    entryCopy->useropinfo = NULL;

    if (head != NULL) {
        cs_cons_append(head, cs_cons(csound, entryCopy, NULL));
    } else {
        head = cs_cons(csound, entryCopy, NULL);
        cs_hash_table_put(csound, csound->opcodes, shortName, head);
    }

    if (shortName != ep->opname) {
        mfree(csound, shortName);
    }

    return 0;
}

PUBLIC int csoundAppendOpcode(CSOUND *csound,
                              const char *opname, int dsblksiz, int flags,
                              int thread, const char *outypes,
                                          const char *intypes,
                              int (*iopadr)(CSOUND *, void *),
                              int (*kopadr)(CSOUND *, void *),
                              int (*aopadr)(CSOUND *, void *))
{
  OENTRY  tmpEntry;
    int     err;

    tmpEntry.opname     = (char*) opname;
    tmpEntry.dsblksiz   = (uint16) dsblksiz;
    tmpEntry.flags      = (uint16) flags;
    tmpEntry.thread     = (uint8_t) thread;
    tmpEntry.outypes    = (char*) outypes;
    tmpEntry.intypes    = (char*) intypes;
    tmpEntry.iopadr     = iopadr;
    tmpEntry.kopadr     = kopadr;
    tmpEntry.aopadr     = aopadr;
    err = opcode_list_new_oentry(csound, &tmpEntry);
    //add_to_symbtab(csound, &tmpEntry);
    if (UNLIKELY(err))
      csoundErrorMsg(csound, Str("Failed to allocate new opcode entry."));

    return err;
}

/**
 * Appends a list of opcodes implemented by external software to Csound's
 * internal opcode list. The list should either be terminated with an entry
 * that has a NULL opname, or the number of entries (> 0) should be specified
 * in 'n'. Returns zero on success.
 */

int csoundAppendOpcodes(CSOUND *csound, const OENTRY *opcodeList, int n)
{
    OENTRY  *ep = (OENTRY*) opcodeList;
    int     err, retval = 0;

    if (UNLIKELY(opcodeList == NULL))
      return -1;
    if (UNLIKELY(n <= 0))
      n = 0x7FFFFFFF;
    while (n && ep->opname != NULL) {
      if (UNLIKELY((err = opcode_list_new_oentry(csound, ep)) != 0)) {
        csoundErrorMsg(csound, Str("Failed to allocate opcode entry for %s."),
                       ep->opname);
        retval = err;
      }

      n--, ep++;
    }
    return retval;
}

/*
 * MISC FUNCTIONS
 */

int defaultCsoundYield(CSOUND *csound)
{
    (void) csound;
    return 1;
}

PUBLIC void csoundSetYieldCallback(CSOUND *csound,
                                   int (*yieldCallback)(CSOUND *))
{
    csound->csoundYieldCallback_ = yieldCallback;
}

void SetInternalYieldCallback(CSOUND *csound,
                              int (*yieldCallback)(CSOUND *))
{
    csound->csoundInternalYieldCallback_ = yieldCallback;
}

int csoundYield(CSOUND *csound)
{
    if (exitNow_)
      csoundLongJmp(csound, CSOUND_SIGNAL);
    csound->csoundInternalYieldCallback_(csound);
    return csound->csoundYieldCallback_(csound);
}

typedef struct resetCallback_s {
  void    *userData;
  int     (*func)(CSOUND *, void *);
  struct resetCallback_s  *nxt;
} resetCallback_t;


static void reset(CSOUND *csound)
{
    CSOUND    *saved_env;
    void      *p1, *p2;
    uintptr_t length;
    uintptr_t end, start;
    int n = 0;

    csoundCleanup(csound);

    /* call registered reset callbacks */
    while (csound->reset_list != NULL) {
      resetCallback_t *p = (resetCallback_t*) csound->reset_list;
      p->func(csound, p->userData);
      csound->reset_list = (void*) p->nxt;
      free(p);
    }
    /* call local destructor routines of external modules */
    /* should check return value... */
    csoundDestroyModules(csound);

    /* IV - Feb 01 2005: clean up configuration variables and */
    /* named dynamic "global" variables of Csound instance */
    csoundDeleteAllConfigurationVariables(csound);
    csoundDeleteAllGlobalVariables(csound);

#ifdef CSCORE
    cscoreRESET(csound);
#endif
    if (csound->opcodes != NULL) {
      free_opcode_table(csound);
      csound->opcodes = NULL;
    }

    csound->oparms_.odebug = 0;
    /* RWD 9:2000 not terribly vital, but good to do this somewhere... */
    pvsys_release(csound);
    close_all_files(csound);
    /* delete temporary files created by this Csound instance */
    remove_tmpfiles(csound);
    rlsmemfiles(csound);

     while (csound->filedir[n])        /* Clear source directory */
       mfree(csound,csound->filedir[n++]);

     memRESET(csound);

    /**
     * Copy everything EXCEPT the function pointers.
     * We do it by saving them and copying them back again...
     * hope that this does not fail...
     */
    /* VL 07.06.2013 - check if the status is COMP before
       resetting.
    */
    //CSOUND **self = csound->self;
    saved_env = (CSOUND*) malloc(sizeof(CSOUND));
    memcpy(saved_env, csound, sizeof(CSOUND));
    memcpy(csound, &cenviron_, sizeof(CSOUND));
    end = (uintptr_t) &(csound->first_callback_); /* used to be &(csound->ids) */
    start =(uintptr_t)  csound;
    length = end - start;
    memcpy((void*) csound, (void*) saved_env, (size_t) length);
    csound->oparms = &(csound->oparms_);
    csound->hostdata = saved_env->hostdata;
    p1 = (void*) &(csound->first_callback_);
    p2 = (void*) &(csound->last_callback_);
    length = (uintptr_t) p2 - (uintptr_t) p1;
    memcpy(p1, (void*) &(saved_env->first_callback_), (size_t) length);
    csound->csoundCallbacks_ = saved_env->csoundCallbacks_;
    csound->API_lock = saved_env->API_lock;
#ifdef HAVE_PTHREAD_SPIN_LOCK
    csound->memlock = saved_env->memlock;
    csound->spinlock = saved_env->spinlock;
    csound->spoutlock = saved_env->spoutlock;
    csound->spinlock1= saved_env->spinlock1;
#endif
    csound->enableHostImplementedMIDIIO = saved_env->enableHostImplementedMIDIIO;
    memcpy(&(csound->exitjmp), &(saved_env->exitjmp), sizeof(jmp_buf));
    csound->memalloc_db = saved_env->memalloc_db;
    csound->message_buffer = saved_env->message_buffer; /*VL 19.06.21 keep msg buffer */
    //csound->self = self;
    free(saved_env);

}


PUBLIC void csoundSetRTAudioModule(CSOUND *csound, const char *module){
    char *s;
    if ((s = csoundQueryGlobalVariable(csound, "_RTAUDIO")) != NULL)
      strNcpy(s, module, 20);
    if (UNLIKELY(s==NULL)) return;        /* Should not happen */
    if (strcmp(s, "null") == 0 || strcmp(s, "Null") == 0 ||
        strcmp(s, "NULL") == 0) {
      csoundMessage(csound, Str("setting dummy interface\n"));
      csoundSetPlayopenCallback(csound, playopen_dummy);
      csoundSetRecopenCallback(csound, recopen_dummy);
      csoundSetRtplayCallback(csound, rtplay_dummy);
      csoundSetRtrecordCallback(csound, rtrecord_dummy);
      csoundSetRtcloseCallback(csound, rtclose_dummy);
      csoundSetAudioDeviceListCallback(csound, audio_dev_list_dummy);
      return;
    }
    if (csoundInitModules(csound) != 0)
      csoundLongJmp(csound, 1);
}


PUBLIC void csoundSetMIDIModule(CSOUND *csound, const char *module){
    char *s;

    if ((s = csoundQueryGlobalVariable(csound, "_RTMIDI")) != NULL)
      strNcpy(s, module, 20);
    if (UNLIKELY(s==NULL)) return;        /* Should not happen */
    if (strcmp(s, "null") == 0 || strcmp(s, "Null") == 0 ||
       strcmp(s, "NULL") == 0) {
      csoundSetMIDIDeviceListCallback(csound, midi_dev_list_dummy);
      csoundSetExternalMidiInOpenCallback(csound, DummyMidiInOpen);
      csoundSetExternalMidiReadCallback(csound,  DummyMidiRead);
      csoundSetExternalMidiInCloseCallback(csound, NULL);
      csoundSetExternalMidiOutOpenCallback(csound,  DummyMidiOutOpen);
      csoundSetExternalMidiWriteCallback(csound, DummyMidiWrite);
      csoundSetExternalMidiOutCloseCallback(csound, NULL);

      return;
    }
    if (csoundInitModules(csound) != 0)
      csoundLongJmp(csound, 1);
}


PUBLIC int csoundGetModule(CSOUND *csound, int no, char **module, char **type){
    MODULE_INFO **modules =
      (MODULE_INFO **) csoundQueryGlobalVariable(csound, "_MODULES");
    if (UNLIKELY(modules[no] == NULL || no >= MAX_MODULES)) return CSOUND_ERROR;
    *module = modules[no]->module;
    *type = modules[no]->type;
    return CSOUND_SUCCESS;
}

PUBLIC int csoundLoadPlugins(CSOUND *csound, const char *dir){
  if (dir != NULL) {
   csoundMessage(csound, "loading plugins from %s\n", dir);
   int err = csoundLoadAndInitModules(csound, dir);
  if(!err) {
     return CSOUND_SUCCESS;
  }
  else return err;
  }
  else return CSOUND_ERROR;
}

PUBLIC void csoundReset(CSOUND *csound)
{
    char    *s;
    int     i, max_len;
    OPARMS  *O = csound->oparms;


    if (csound->engineStatus & CS_STATE_COMP ||
        csound->engineStatus & CS_STATE_PRE) {
     /* and reset */
      csoundMessage(csound, "resetting Csound instance\n");
      reset(csound);
      /* clear compiled flag */
      csound->engineStatus |= ~(CS_STATE_COMP);
    } else {
      csoundSpinLockInit(&csound->spoutlock);
      csoundSpinLockInit(&csound->spinlock);
      csoundSpinLockInit(&csound->memlock);
      csoundSpinLockInit(&csound->spinlock1);
      if (UNLIKELY(O->odebug))
        csoundMessage(csound,"init spinlocks\n");
    }

    if (msgcallback_ != NULL) {
      csoundSetMessageCallback(csound, msgcallback_);
    } else {
      csoundSetMessageCallback(csound, csoundDefaultMessageCallback);
    }
    csound->printerrormessagesflag = (void*)1234;
    /* copysystem environment variables */
    i = csoundInitEnv(csound);
    if (UNLIKELY(i != CSOUND_SUCCESS)) {
      csound->engineStatus |= CS_STATE_JMP;
      csoundDie(csound, Str("Failed during csoundInitEnv"));
    }
    csound_init_rand(csound);

    csound->engineState.stringPool = cs_hash_table_create(csound);
    csound->engineState.constantsPool = cs_hash_table_create(csound);
    csound->engineStatus |= CS_STATE_PRE;
    csound_aops_init_tables(csound);
    create_opcode_table(csound);
    /* now load and pre-initialise external modules for this instance */
    /* this function returns an error value that may be worth checking */
    {

      int err = csoundInitStaticModules(csound);

      if (csound->delayederrormessages &&
          csound->printerrormessagesflag==NULL) {
        csoundWarning(csound, "%s",csound->delayederrormessages);
        mfree(csound, csound->delayederrormessages);
        csound->delayederrormessages = NULL;
      }
      if (UNLIKELY(err==CSOUND_ERROR))
        csoundDie(csound, Str("Failed during csoundInitStaticModules"));

 #ifndef BARE_METAL
     csoundCreateGlobalVariable(csound, "_MODULES",
                                (size_t) MAX_MODULES*sizeof(MODULE_INFO *));
     char *modules = (char *) csoundQueryGlobalVariable(csound, "_MODULES");
     memset(modules, 0, sizeof(MODULE_INFO *)*MAX_MODULES);

     /* VL now load modules has opcodedir override */
     if(opcodedir)
      csound->opcodedir = cs_strdup(csound, opcodedir);
     else
       csound->opcodedir = NULL;

     err = csoundLoadModules(csound);
      if (csound->delayederrormessages &&
          csound->printerrormessagesflag==NULL) {
        csoundWarning(csound, "%s", csound->delayederrormessages);
        mfree(csound, csound->delayederrormessages);
        csound->delayederrormessages = NULL;
      }
      if (UNLIKELY(err != CSOUND_SUCCESS))
        csoundDie(csound, Str("Failed during csoundLoadModules"));

      if (csoundInitModules(csound) != 0)
            csoundLongJmp(csound, 1);

#endif // BARE_METAL
      init_pvsys(csound);
      /* utilities depend on this as well as orchs; may get changed by an orch */
      dbfs_init(csound, DFLT_DBFS);
      csound->csRtClock = (RTCLOCK*) mcalloc(csound, sizeof(RTCLOCK));
      csoundInitTimerStruct(csound->csRtClock);
      csound->engineStatus |= /*CS_STATE_COMP |*/ CS_STATE_CLN;

      /*
        this was moved to musmon();
        print_csound_version(csound);
        print_sndfile_version(csound);
      */

      /* do not know file type yet */
      O->filetyp = -1;
      O->sfheader = 0;
      csound->peakchunks = 1;
      csound->typePool = mcalloc(csound, sizeof(TYPE_POOL));
      csound->engineState.varPool = csoundCreateVarPool(csound);
      csoundAddStandardTypes(csound, csound->typePool);
      /* csoundLoadExternals(csound); */
    }
#ifndef BARE_METAL
    /* allow selecting real time audio module */
    max_len = 21;
    csoundCreateGlobalVariable(csound, "_RTAUDIO", (size_t) max_len);
    s = csoundQueryGlobalVariable(csound, "_RTAUDIO");
#ifndef __gnu_linux__
 #ifdef __HAIKU__
      strcpy(s, "haiku");
 #else
      strcpy(s, "PortAudio");
 #endif
#else
    strcpy(s, "alsa");
#endif
    csoundCreateConfigurationVariable(csound, "rtaudio", s, CSOUNDCFG_STRING,
                                      0, NULL, &max_len,
                                      Str("Real time audio module name"), NULL);
#endif
    /* initialise real time MIDI */
    csound->midiGlobals = (MGLOBAL*) mcalloc(csound, sizeof(MGLOBAL));
    csound->midiGlobals->bufp = &(csound->midiGlobals->mbuf[0]);
    csound->midiGlobals->endatp = csound->midiGlobals->bufp;
#ifndef BARE_METAL    
    csoundCreateGlobalVariable(csound, "_RTMIDI", (size_t) max_len);
    csoundSetMIDIDeviceListCallback(csound, midi_dev_list_dummy);
    csoundSetExternalMidiInOpenCallback(csound, DummyMidiInOpen);
    csoundSetExternalMidiReadCallback(csound,  DummyMidiRead);
    csoundSetExternalMidiOutOpenCallback(csound,  DummyMidiOutOpen);
    csoundSetExternalMidiWriteCallback(csound, DummyMidiWrite);
  
    s = csoundQueryGlobalVariable(csound, "_RTMIDI");
    strcpy(s, "null");
    if (csound->enableHostImplementedMIDIIO == 0)
#ifndef __gnu_linux__
 #ifdef __HAIKU__
      strcpy(s, "haiku");
 #else
      strcpy(s, "portmidi");
 #endif
#else
    strcpy(s, "alsa");
#endif
    else strcpy(s, "hostbased");
 
    csoundCreateConfigurationVariable(csound, "rtmidi", s, CSOUNDCFG_STRING,
                                      0, NULL, &max_len,
                                      Str("Real time MIDI module name"), NULL);
    max_len = 256;  /* should be the same as in csoundCore.h */
    csoundCreateConfigurationVariable(csound, "mute_tracks",
                                      &(csound->midiGlobals->muteTrackList[0]),
                                      CSOUNDCFG_STRING, 0, NULL, &max_len,
                                      Str("Ignore events (other than tempo "
                                          "changes) in tracks defined by pattern"),
                                      NULL);
    csoundCreateConfigurationVariable(csound, "raw_controller_mode",
                                      &(csound->midiGlobals->rawControllerMode),
                                      CSOUNDCFG_BOOLEAN, 0, NULL, NULL,
                                      Str("Do not handle special MIDI controllers"
                                          " (sustain pedal etc.)"), NULL);
 
    /* sound file tag options */
    max_len = 201;
    i = (max_len + 7) & (~7);
    csound->SF_id_title = (char*) mcalloc(csound, (size_t) i * (size_t) 6);
    csoundCreateConfigurationVariable(csound, "id_title", csound->SF_id_title,
                                      CSOUNDCFG_STRING, 0, NULL, &max_len,
                                      Str("Title tag in output soundfile "
                                          "(no spaces)"), NULL);
    csound->SF_id_copyright = (char*) csound->SF_id_title + (int) i;
    csoundCreateConfigurationVariable(csound, "id_copyright",
                                      csound->SF_id_copyright,
                                      CSOUNDCFG_STRING, 0, NULL, &max_len,
                                      Str("Copyright tag in output soundfile"
                                          " (no spaces)"), NULL);
    csoundCreateConfigurationVariable(csound, "id_scopyright",
                                      &csound->SF_id_scopyright,
                                      CSOUNDCFG_INTEGER, 0, NULL, &max_len,
                                      Str("Short Copyright tag in"
                                          " output soundfile"), NULL);
    csound->SF_id_software = (char*) csound->SF_id_copyright + (int) i;
    csoundCreateConfigurationVariable(csound, "id_software",
                                      csound->SF_id_software,
                                      CSOUNDCFG_STRING, 0, NULL, &max_len,
                                      Str("Software tag in output soundfile"
                                          " (no spaces)"), NULL);
    csound->SF_id_artist = (char*) csound->SF_id_software + (int) i;
    csoundCreateConfigurationVariable(csound, "id_artist", csound->SF_id_artist,
                                      CSOUNDCFG_STRING, 0, NULL, &max_len,
                                      Str("Artist tag in output soundfile "
                                          "(no spaces)"),
                                      NULL);
    csound->SF_id_comment = (char*) csound->SF_id_artist + (int) i;
    csoundCreateConfigurationVariable(csound, "id_comment",
                                      csound->SF_id_comment,
                                      CSOUNDCFG_STRING, 0, NULL, &max_len,
                                      Str("Comment tag in output soundfile"
                                          " (no spaces)"), NULL);
    csound->SF_id_date = (char*) csound->SF_id_comment + (int) i;
    csoundCreateConfigurationVariable(csound, "id_date", csound->SF_id_date,
                                      CSOUNDCFG_STRING, 0, NULL, &max_len,
                                      Str("Date tag in output soundfile "
                                          "(no spaces)"),
                                      NULL);
    {

      MYFLT minValF = FL(0.0);

      csoundCreateConfigurationVariable(csound, "msg_color",
                                        &(csound->enableMsgAttr),
                                        CSOUNDCFG_BOOLEAN, 0, NULL, NULL,
                                        Str("Enable message attributes "
                                            "(colors etc.)"),
                                        NULL);
      csoundCreateConfigurationVariable(csound, "skip_seconds",
                                        &(csound->csoundScoreOffsetSeconds_),
                                        CSOUNDCFG_MYFLT, 0, &minValF, NULL,
                                        Str("Start score playback at the specified"
                                            " time, skipping earlier events"),
                                        NULL);
    }
    csoundCreateConfigurationVariable(csound, "ignore_csopts",
                                      &(csound->disable_csd_options),
                                      CSOUNDCFG_BOOLEAN, 0, NULL, NULL,
                                      Str("Ignore <CsOptions> in CSD files"
                                          " (default: no)"), NULL);
#endif    
}

PUBLIC int csoundGetDebug(CSOUND *csound)
{
    return csound->oparms_.odebug;
}

PUBLIC void csoundSetDebug(CSOUND *csound, int debug)
{
    csound->oparms_.odebug = debug;
}

PUBLIC int csoundTableLength(CSOUND *csound, int table)
{
    MYFLT *tablePtr;
    return csoundGetTable(csound, &tablePtr, table);
}

PUBLIC MYFLT csoundTableGet(CSOUND *csound, int table, int index)
{
    return csound->flist[table]->ftable[index];
}

void csoundTableSetInternal(CSOUND *csound,
                                   int table, int index, MYFLT value)
{
    if (csound->oparms->realtime) csoundLockMutex(csound->init_pass_threadlock);
    csound->flist[table]->ftable[index] = value;
    if (csound->oparms->realtime) csoundUnlockMutex(csound->init_pass_threadlock);
}

static int csoundDoCallback_(CSOUND *csound, void *p, unsigned int type)
{
    if (csound->csoundCallbacks_ != NULL) {
      CsoundCallbackEntry_t *pp;
      pp = (CsoundCallbackEntry_t*) csound->csoundCallbacks_;
      do {
        if (pp->typeMask & type) {
          int   retval = pp->func(pp->userData, p, type);
          if (retval != CSOUND_SUCCESS)
            return retval;
        }
        pp = pp->nxt;
      } while (pp != (CsoundCallbackEntry_t*) NULL);
    }
    return 1;
}

/**
 * Sets a callback function that will be called on keyboard
 * events. The callback is preserved on csoundReset(), and multiple
 * callbacks may be set and will be called in reverse order of
 * registration. If the same function is set again, it is only moved
 * in the list of callbacks so that it will be called first, and the
 * user data and type mask parameters are updated. 'typeMask' can be the
 * bitwise OR of callback types for which the function should be called,
 * or zero for all types.
 * Returns zero on success, CSOUND_ERROR if the specified function
 * pointer or type mask is invalid, and CSOUND_MEMORY if there is not
 * enough memory.
 *
 * The callback function takes the following arguments:
 *   void *userData
 *     the "user data" pointer, as specified when setting the callback
 *   void *p
 *     data pointer, depending on the callback type
 *   unsigned int type
 *     callback type, can be one of the following (more may be added in
 *     future versions of Csound):
 *       CSOUND_CALLBACK_KBD_EVENT
 *       CSOUND_CALLBACK_KBD_TEXT
 *         called by the sensekey opcode to fetch key codes. The data
 *         pointer is a pointer to a single value of type 'int', for
 *         returning the key code, which can be in the range 1 to 65535,
 *         or 0 if there is no keyboard event.
 *         For CSOUND_CALLBACK_KBD_EVENT, both key press and release
 *         events should be returned (with 65536 (0x10000) added to the
 *         key code in the latter case) as unshifted ASCII codes.
 *         CSOUND_CALLBACK_KBD_TEXT expects key press events only as the
 *         actual text that is typed.
 * The return value should be zero on success, negative on error, and
 * positive if the callback was ignored (for example because the type is
 * not known).
 */

PUBLIC int csoundRegisterKeyboardCallback(CSOUND *csound,
                                          int (*func)(void *userData, void *p,
                                                      unsigned int type),
                                          void *userData, unsigned int typeMask)
{
    CsoundCallbackEntry_t *pp;

    if (UNLIKELY(func == (int (*)(void *, void *, unsigned int)) NULL ||
                 (typeMask
                  & (~(CSOUND_CALLBACK_KBD_EVENT | CSOUND_CALLBACK_KBD_TEXT)))
                 != 0U))
        return CSOUND_ERROR;
    csoundRemoveKeyboardCallback(csound, func);
    pp = (CsoundCallbackEntry_t*) malloc(sizeof(CsoundCallbackEntry_t));
    if (UNLIKELY(pp == (CsoundCallbackEntry_t*) NULL))
        return CSOUND_MEMORY;
    pp->typeMask = (typeMask ? typeMask : 0xFFFFFFFFU);
    pp->nxt = (CsoundCallbackEntry_t*) csound->csoundCallbacks_;
    pp->userData = userData;
    pp->func = func;
    csound->csoundCallbacks_ = (void*) pp;

    return CSOUND_SUCCESS;
}


/**
 * Removes a callback previously set with csoundSetCallback().
 */

PUBLIC void csoundRemoveKeyboardCallback(CSOUND *csound,
                                 int (*func)(void *, void *, unsigned int))
{
    CsoundCallbackEntry_t *pp, *prv;

    pp = (CsoundCallbackEntry_t*) csound->csoundCallbacks_;
    prv = (CsoundCallbackEntry_t*) NULL;
    while (pp != (CsoundCallbackEntry_t*) NULL) {
      if (pp->func == func) {
        if (prv != (CsoundCallbackEntry_t*) NULL)
          prv->nxt = pp->nxt;
        else
          csound->csoundCallbacks_ = (void*) pp->nxt;
        free((void*) pp);
        return;
      }
      prv = pp;
      pp = pp->nxt;
    }
}


PUBLIC void csoundSetFileOpenCallback(CSOUND *p,
                                      void (*fileOpenCallback)(CSOUND*,
                                                               const char*,
                                                               int, int, int))
{
    p->FileOpenCallback_ = fileOpenCallback;
}

/* csoundNotifyFileOpened() should be called by plugins via
   csoundNotifyFileOpened() to let Csound know that they opened a file
   without using one of the standard mechanisms (csoundFileOpenWithType() or
   ldmemfile2withCB()).  The notification is passed on to the host if it
   has set the FileOpen callback. */
void csoundNotifyFileOpened(CSOUND* csound, const char* pathname,
                            int csFileType, int writing, int temporary)
{
    if (csound->FileOpenCallback_ != NULL)
      csound->FileOpenCallback_(csound, pathname, csFileType, writing,
                                temporary);
    return;

}

/* -------- IV - Jan 27 2005: timer functions -------- */

#ifdef HAVE_GETTIMEOFDAY
#undef HAVE_GETTIMEOFDAY
#endif
#if defined(__gnu_linux__) || defined(__unix) || defined(__unix__) || defined(__MACH__)
#define HAVE_GETTIMEOFDAY 1
#include <sys/time.h>
#endif

/* enable use of high resolution timer (Linux/i586/GCC only) */
/* could in fact work under any x86/GCC system, but do not   */
/* know how to query the actual CPU frequency ...            */

//#define HAVE_RDTSC  1

/* ------------------------------------ */

#if defined(HAVE_RDTSC)
#if !(defined(__gnu_linux__) && defined(__GNUC__) && defined(__i386__))
#undef HAVE_RDTSC
#endif
#endif

/* hopefully cannot change during performance */
static double timeResolutionSeconds = -1.0;

/* find out CPU frequency based on /proc/cpuinfo */

static int getTimeResolution(void)
{
#if defined(HAVE_RDTSC)
    FILE    *f;
    char    buf[256];

    /* if frequency is not known yet */
    f = fopen("/proc/cpuinfo", "r");
    if (UNLIKELY(f == NULL)) {
      fprintf(stderr, Str("Cannot open /proc/cpuinfo. "
                          "Support for RDTSC is not available.\n"));
      return -1;
    }
    /* find CPU frequency */
    while (fgets(buf, 256, f) != NULL) {
      int     i;
      char    *s = (char*) buf - 1;
      buf[255] = '\0';          /* safety */
      if (strlen(buf) < 9)
        continue;                       /* too short, skip */
      while (*++s != '\0')
        if (isupper(*s))
          *s = tolower(*s);             /* convert to lower case */
      if (strncmp(buf, "cpu mhz", 7) != 0)
        continue;                       /* check key name */
      s = strchr(buf, ':');             /* find frequency value */
      if (s == NULL) continue;          /* invalid entry */
      do {
        s++;
      } while (isblank(*s));            /* skip white space */
      i = CS_SSCANF(s, "%lf", &timeResolutionSeconds);

      if (i < 1 || timeResolutionSeconds < 1.0) {
        timeResolutionSeconds = -1.0;       /* invalid entry */
        continue;
      }
    }
    fclose(f);
    if (UNLIKELY(timeResolutionSeconds <= 0.0)) {
      fprintf(stderr, Str("No valid CPU frequency entry "
                          "was found in /proc/cpuinfo.\n"));
      return -1;
    }
    /* MHz -> seconds */
    timeResolutionSeconds = 0.000001 / timeResolutionSeconds;
#elif defined(_WIN32)
    LARGE_INTEGER tmp1;
    int_least64_t tmp2;
    QueryPerformanceFrequency(&tmp1);
    tmp2 = (int_least64_t) tmp1.LowPart + ((int_least64_t) tmp1.HighPart << 32);
    timeResolutionSeconds = 1.0 / (double) tmp2;
#elif defined(HAVE_GETTIMEOFDAY)
    timeResolutionSeconds = 0.000001;
#else
    timeResolutionSeconds = 1.0;
#endif
#ifdef CHECK_TIME_RESOLUTION // BETA
    fprintf(stderr, "time resolution is %.3f ns\n",
            1.0e9 * timeResolutionSeconds);
#endif
    return 0;
}

/* function for getting real time */

static inline int_least64_t get_real_time(void)
{
#if defined(HAVE_RDTSC)
    /* optimised high resolution timer for Linux/i586/GCC only */
    uint32_t  l, h;
#ifndef __STRICT_ANSI__
    asm volatile ("rdtsc" : "=a" (l), "=d" (h));
#else
    __asm__ volatile ("rdtsc" : "=a" (l), "=d" (h));
#endif
    return ((int_least64_t) l + ((int_least64_t) h << 32));
#elif defined(_WIN32)
    /* Win32: use QueryPerformanceCounter - resolution depends on system, */
    /* but is expected to be better than 1 us. GetSystemTimeAsFileTime    */
    /* seems to have much worse resolution under Win95.                   */
    LARGE_INTEGER tmp;
    QueryPerformanceCounter(&tmp);
    return ((int_least64_t) tmp.LowPart + ((int_least64_t) tmp.HighPart <<32));
#elif defined(HAVE_GETTIMEOFDAY)
    /* UNIX: use gettimeofday() - allows 1 us resolution */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((int_least64_t) tv.tv_usec
            + (int_least64_t) ((uint32_t) tv.tv_sec * (uint64_t) 1000000));
#else
    /* other systems: use time() - allows 1 second resolution */
    return ((int_least64_t) time(NULL));
#endif
}

/* function for getting CPU time */

static inline int_least64_t get_CPU_time(void)
{
    return ((int_least64_t) ((uint32_t) clock()));
}

/* initialise a timer structure */

PUBLIC void csoundInitTimerStruct(RTCLOCK *p)
{
    p->starttime_real = get_real_time();
    p->starttime_CPU = get_CPU_time();
}

/**
 * return the elapsed real time (in seconds) since the specified timer
 * structure was initialised
 */
PUBLIC double csoundGetRealTime(RTCLOCK *p)
{
    return ((double) (get_real_time() - p->starttime_real)
            * (double) timeResolutionSeconds);
}

/**
 * return the elapsed CPU time (in seconds) since the specified timer
 * structure was initialised
 */
PUBLIC double csoundGetCPUTime(RTCLOCK *p)
{
    return ((double) ((uint32_t) get_CPU_time() - (uint32_t) p->starttime_CPU)
            * (1.0 / (double) CLOCKS_PER_SEC));
}

/* return a 32-bit unsigned integer to be used as seed from current time */

PUBLIC uint32_t csoundGetRandomSeedFromTime(void)
{
    return (uint32_t) get_real_time();
}

/**
 * Return the size of MYFLT in bytes.
 */
PUBLIC int csoundGetSizeOfMYFLT(void)
{
    return (int) sizeof(MYFLT);
}

/**
 * Return pointer to user data pointer for real time audio input.
 */
PUBLIC void **csoundGetRtRecordUserData(CSOUND *csound)
{
    return &(csound->rtRecord_userdata);
}

/**
 * Return pointer to user data pointer for real time audio output.
 */
PUBLIC void **csoundGetRtPlayUserData(CSOUND *csound)
{
    return &(csound->rtPlay_userdata);
}

/**
 * Register a function to be called at note deactivation.
 * Should be called from the initialisation routine of an opcode.
 * 'p' is a pointer to the OPDS structure of the opcode, and 'func'
 * is the function to be called, with the same arguments and return
 * value as in the case of opcode init/perf functions.
 * The functions are called in reverse order of registration.
 * Returns zero on success.
 */

int csoundRegisterDeinitCallback(CSOUND *csound, void *p,
                                 int (*func)(CSOUND *, void *))
{
    INSDS           *ip = ((OPDS*) p)->insdshead;
    opcodeDeinit_t  *dp = (opcodeDeinit_t*) malloc(sizeof(opcodeDeinit_t));

    (void) csound;
    if (UNLIKELY(dp == NULL))
      return CSOUND_MEMORY;
    dp->p = p;
    dp->func = func;
    dp->nxt = ip->nxtd;
    ip->nxtd = dp;
    return CSOUND_SUCCESS;
}

/**
 * Register a function to be called by csoundReset(), in reverse order
 * of registration, before unloading external modules. The function takes
 * the Csound instance pointer as the first argument, and the pointer
 * passed here as 'userData' as the second, and is expected to return zero
 * on success.
 * The return value of csoundRegisterResetCallback() is zero on success.
 */

int csoundRegisterResetCallback(CSOUND *csound, void *userData,
                                int (*func)(CSOUND *, void *))
{
    resetCallback_t *dp = (resetCallback_t*) malloc(sizeof(resetCallback_t));

    if (UNLIKELY(dp == NULL))
      return CSOUND_MEMORY;
    dp->userData = userData;
    dp->func = func;
    dp->nxt = csound->reset_list;
    csound->reset_list = (void*) dp;
    return CSOUND_SUCCESS;
}

/**
 * Returns the name of the opcode of which the data structure
 * is pointed to by 'p'.
 */
char *csoundGetOpcodeName(void *p)
{
    return ((OPDS*) p)->optext->t.oentry->opname;
}

/** Returns the CS_TYPE for an opcode's arg pointer */

CS_TYPE* csoundGetTypeForArg(void* argPtr) {
    char* ptr = (char*)argPtr;
    CS_TYPE* varType = *(CS_TYPE**)(ptr - CS_VAR_TYPE_OFFSET);
    return varType;
}

/**
 * Returns the number of input arguments for opcode 'p'.
 */
int csoundGetInputArgCnt(void *p)
{
    return (int) ((OPDS*) p)->optext->t.inArgCount;
}


/**
 * Returns the name of input argument 'n' (counting from 0) for opcode 'p'.
 */
char *csoundGetInputArgName(void *p, int n)
{
    if ((unsigned int) n >=
        (unsigned int) ((OPDS*) p)->optext->t.inArgCount)
      return (char*) NULL;
    return (char*) ((OPDS*) p)->optext->t.inlist->arg[n];
}

/**
 * Returns the number of output arguments for opcode 'p'.
 */
int csoundGetOutputArgCnt(void *p)
{
    return (int) ((OPDS*) p)->optext->t.outArgCount;
}

/**
 * Returns the name of output argument 'n' (counting from 0) for opcode 'p'.
 */
char *csoundGetOutputArgName(void *p, int n)
{
    if ((unsigned int) n
        >= (unsigned int) ((OPDS*) p)->optext->t.outArgCount)
      return (char*) NULL;
    return (char*) ((OPDS*) p)->optext->t.outlist->arg[n];
}

/**
 * Returns MIDI channel number (0 to 15) for the instrument instance
 * that called opcode 'p'.
 * In the case of score notes, -1 is returned.
 */
int csoundGetMidiChannelNumber(void *p)
{
    MCHNBLK *chn = ((OPDS*) p)->insdshead->m_chnbp;
    int     i;
    if (chn == NULL)
      return -1;
    for (i = 0; i < 256; i++) {
      if (chn == ((OPDS*) p)->insdshead->csound->m_chnbp[i])
        return i;
    }
    return -1;
}

/**
 * Returns a pointer to the MIDI channel structure for the instrument
 * instance that called opcode 'p'.
 * In the case of score notes, NULL is returned.
 */
MCHNBLK *csoundGetMidiChannel(void *p)
{
    return ((OPDS*) p)->insdshead->m_chnbp;
}

/**
 * Returns MIDI note number (in the range 0 to 127) for opcode 'p'.
 * If the opcode was not called from a MIDI activated instrument
 * instance, the return value is undefined.
 */
int csoundGetMidiNoteNumber(void *p)
{
    return (int) ((OPDS*) p)->insdshead->m_pitch;
}

/**
 * Returns MIDI velocity (in the range 0 to 127) for opcode 'p'.
 * If the opcode was not called from a MIDI activated instrument
 * instance, the return value is undefined.
 */
int csoundGetMidiVelocity(void *p)
{
    return (int) ((OPDS*) p)->insdshead->m_veloc;
}

/**
 * Returns non-zero if the current note (owning opcode 'p') is releasing.
 */
int csoundGetReleaseFlag(void *p)
{
    return (int) ((OPDS*) p)->insdshead->relesing;
}

/**
 * Returns the note-off time in seconds (measured from the beginning of
 * performance) of the current instrument instance, from which opcode 'p'
 * was called. The return value may be negative if the note has indefinite
 * duration.
 */
double csoundGetOffTime(void *p)
{
    return (double) ((OPDS*) p)->insdshead->offtim;
}

/**
 * Returns the array of p-fields passed to the instrument instance
 * that owns opcode 'p', starting from p0. Only p1, p2, and p3 are
 * guaranteed to be available. p2 is measured in seconds from the
 * beginning of the current section.
 */
MYFLT *csoundGetPFields(void *p)
{

    /* FIXME - this is no longer valid, should return CS_VAR_MEM*
       and use ->p0_type */
    return (MYFLT*) &(((OPDS*) p)->insdshead->p0);
}

/**
 * Returns the instrument number (p1) for opcode 'p'.
 */
int csoundGetInstrumentNumber(void *p)
{
    return (int) ((OPDS*) p)->insdshead->p1.value;
}

typedef struct csMsgStruct_ {
  struct csMsgStruct_  *nxt;
  int         attr;
  char        s[1];
} csMsgStruct;

typedef struct csMsgBuffer_ {
  void        *mutex_;
  csMsgStruct *firstMsg;
  csMsgStruct *lastMsg;
  int         msgCnt;
  char        *buf;
} csMsgBuffer;

// callback for storing messages in the buffer only
static void csoundMessageBufferCallback_1_(CSOUND *csound, int attr,
                                           const char *fmt, va_list args);

// callback for writing messages to the buffer, and also stdout/stderr
static void csoundMessageBufferCallback_2_(CSOUND *csound, int attr,
                                           const char *fmt, va_list args);

/**
 * Creates a buffer for storing messages printed by Csound.
 * Should be called after creating a Csound instance; note that
 * the message buffer uses the host data pointer, and the buffer
 * should be freed by calling csoundDestroyMessageBuffer() before
 * deleting the Csound instance.
 * If 'toStdOut' is non-zero, the messages are also printed to
 * stdout and stderr (depending on the type of the message),
 * in addition to being stored in the buffer.
 */

void PUBLIC csoundCreateMessageBuffer(CSOUND *csound, int toStdOut)
{
    csMsgBuffer *pp;
    size_t      nBytes;

    pp = (csMsgBuffer*) csound->message_buffer;
    if (pp) {
        csoundDestroyMessageBuffer(csound);
    }
    nBytes = sizeof(csMsgBuffer);
    if (!toStdOut) {
        nBytes += (size_t) 16384;
    }
    pp = (csMsgBuffer*) malloc(nBytes);
    pp->mutex_ = csoundCreateMutex(0);
    pp->firstMsg = (csMsgStruct*) NULL;
    pp->lastMsg = (csMsgStruct*) NULL;
    pp->msgCnt = 0;
    if (!toStdOut) {
        pp->buf = (char*) pp + (int) sizeof(csMsgBuffer);
        pp->buf[0] = (char) '\0';
    } else {
        pp->buf = (char*) NULL;
    }
    csound->message_buffer = (void*) pp;
    if (toStdOut) {
        csoundSetMessageCallback(csound, csoundMessageBufferCallback_2_);
    } else {
        csoundSetMessageCallback(csound, csoundMessageBufferCallback_1_);
    }
}

/**
 * Returns the first message from the buffer.
 */
#ifdef MSVC
const char PUBLIC *csoundGetFirstMessage(CSOUND *csound)
#else
const char */*PUBLIC*/ csoundGetFirstMessage(CSOUND *csound)
#endif
{
    csMsgBuffer *pp = (csMsgBuffer*) csound->message_buffer;
    char        *msg = NULL;

    if (pp && pp->msgCnt) {
      csoundLockMutex(pp->mutex_);
      if (pp->firstMsg)
        msg = &(pp->firstMsg->s[0]);
      csoundUnlockMutex(pp->mutex_);
    }
    return msg;
}

/**
 * Returns the attribute parameter (see msg_attr.h) of the first message
 * in the buffer.
 */

int PUBLIC csoundGetFirstMessageAttr(CSOUND *csound)
{
    csMsgBuffer *pp = (csMsgBuffer*) csound->message_buffer;
    int         attr = 0;

    if (pp && pp->msgCnt) {
        csoundLockMutex(pp->mutex_);
        if (pp->firstMsg) {
            attr = pp->firstMsg->attr;
        }
        csoundUnlockMutex(pp->mutex_);
    }
    return attr;
}

/**
 * Removes the first message from the buffer.
 */

void PUBLIC csoundPopFirstMessage(CSOUND *csound)
{
    csMsgBuffer *pp = (csMsgBuffer*) csound->message_buffer;

    if (pp) {
      csMsgStruct *tmp;
      csoundLockMutex(pp->mutex_);
      tmp = pp->firstMsg;
      if (tmp) {
        pp->firstMsg = tmp->nxt;
        pp->msgCnt--;
        if (!pp->firstMsg)
          pp->lastMsg = (csMsgStruct*) 0;
      }
      csoundUnlockMutex(pp->mutex_);
      if (tmp)
        free((void*) tmp);
    }
}

/**
 * Returns the number of pending messages in the buffer.
 */

int PUBLIC csoundGetMessageCnt(CSOUND *csound)
{
    csMsgBuffer *pp = (csMsgBuffer*) csound->message_buffer;
    int         cnt = -1;

    if (pp) {
      csoundLockMutex(pp->mutex_);
      cnt = pp->msgCnt;
      csoundUnlockMutex(pp->mutex_);
    }
    return cnt;
}

/**
 * Releases all memory used by the message buffer.
 */

void PUBLIC csoundDestroyMessageBuffer(CSOUND *csound)
{
    csMsgBuffer *pp = (csMsgBuffer*) csound->message_buffer;
    if (!pp) {
      csoundWarning(csound,
                      Str("csoundDestroyMessageBuffer: "
                          "Message buffer not allocated."));
        return;
    }
    csMsgStruct *msg = pp->firstMsg;
    while (msg) {
        csMsgStruct *tmp = msg;
        msg = tmp->nxt;
        free(tmp);
    }
    csound->message_buffer = NULL;
    csoundSetMessageCallback(csound, NULL);
    while (csoundGetMessageCnt(csound) > 0) {
        csoundPopFirstMessage(csound);
    }
    csoundSetHostData(csound, NULL);
    csoundDestroyMutex(pp->mutex_);
    free((void*) pp);
}

static void csoundMessageBufferCallback_1_(CSOUND *csound, int attr,
                                           const char *fmt, va_list args)
{
    csMsgBuffer *pp = (csMsgBuffer*) csound->message_buffer;
    csMsgStruct *p;
    int         len;

    csoundLockMutex(pp->mutex_);
    len = vsnprintf(pp->buf, 16384, fmt, args); // FIXEDME: this can overflow
    va_end(args);
    if (UNLIKELY((unsigned int) len >= (unsigned int) 16384)) {
      csoundUnlockMutex(pp->mutex_);
      fprintf(stderr, Str("csound: internal error: message buffer overflow\n"));
      exit(-1);
    }
    p = (csMsgStruct*) malloc(sizeof(csMsgStruct) + (size_t) len);
    p->nxt = (csMsgStruct*) NULL;
    p->attr = attr;
    strcpy(&(p->s[0]), pp->buf);
    if (pp->firstMsg == (csMsgStruct*) 0) {
        pp->firstMsg = p;
    } else {
        pp->lastMsg->nxt = p;
    }
    pp->lastMsg = p;
    pp->msgCnt++;
    csoundUnlockMutex(pp->mutex_);
}

static void csoundMessageBufferCallback_2_(CSOUND *csound, int attr,
                                           const char *fmt, va_list args)
{
    csMsgBuffer *pp = (csMsgBuffer*) csound->message_buffer;
    csMsgStruct *p;
    int         len = 0;
    va_list     args_save;

    va_copy(args_save, args);
    switch (attr & CSOUNDMSG_TYPE_MASK) {
    case CSOUNDMSG_ERROR:
    case CSOUNDMSG_REALTIME:
    case CSOUNDMSG_WARNING:
      len = vfprintf(stderr, fmt, args);
      break;
    default:
      len = vfprintf(stdout, fmt, args);
    }
    va_end(args);
    p = (csMsgStruct*) malloc(sizeof(csMsgStruct) + (size_t) len);
    p->nxt = (csMsgStruct*) NULL;
    p->attr = attr;
    vsnprintf(&(p->s[0]), len, fmt, args_save);
    va_end(args_save);
    csoundLockMutex(pp->mutex_);
    if (pp->firstMsg == (csMsgStruct*) NULL)
      pp->firstMsg = p;
    else
      pp->lastMsg->nxt = p;
    pp->lastMsg = p;
    pp->msgCnt++;
    csoundUnlockMutex(pp->mutex_);
}

INSTRTXT **csoundGetInstrumentList(CSOUND *csound){
  return csound->engineState.instrtxtp;
}

uint64_t csoundGetKcounter(CSOUND *csound){
  return csound->kcounter;
}

void set_util_sr(CSOUND *csound, MYFLT sr){ csound->esr = sr; }
void set_util_nchnls(CSOUND *csound, int nchnls){ csound->nchnls = nchnls; }

MYFLT csoundGetA4(CSOUND *csound) { return (MYFLT) csound->A4; }

int csoundErrCnt(CSOUND *csound) { return csound->perferrcnt; }

INSTRTXT *csoundGetInstrument(CSOUND *csound, int insno, const char *name) {
  if (name != NULL)
    insno = named_instr_find(csound, (char *)name);
  return csound->engineState.instrtxtp[insno];
}

#if 0
PUBLIC int csoundPerformKsmpsAbsolute(CSOUND *csound)
{
    int done = 0;
    int returnValue;

    /* VL: 1.1.13 if not compiled (csoundStart() not called)  */
    if (UNLIKELY(!(csound->engineStatus & CS_STATE_COMP))) {
      csoundWarning(csound,
                      Str("Csound not ready for performance: csoundStart() "
                          "has not been called\n"));
      return CSOUND_ERROR;
    }
    /* setup jmp for return after an exit() */
    if (UNLIKELY((returnValue = setjmp(csound->exitjmp)))) {
#ifndef __APPLE__
      csoundMessage(csound, Str("Early return from csoundPerformKsmps().\n"));
#endif
      return ((returnValue - CSOUND_EXITJMP_SUCCESS) | CSOUND_EXITJMP_SUCCESS);
    }
    csoundLockMutex(csound->API_lock);
    do {
      done |= sensevents(csound);
    } while (csound->kperf(csound));
    csoundUnlockMutex(csound->API_lock);
    return done;
}
#endif


//#ifdef __cplusplus
//}
//#endif
