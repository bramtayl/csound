#pragma once

#include "csound.h"
#include "csound_type_system.h"
#include "sysdep.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

#if defined(__MACH__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <xlocale.h>
#endif

#if defined(__MACH__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <xlocale.h>
#endif

#if (defined(__MACH__) || defined(ANDROID) || defined(NACL) ||                 \
     defined(__CYGWIN__) || defined(__HAIKU__))
#include <pthread.h>
#define BARRIER_SERIAL_THREAD (-1)
typedef struct {
  pthread_mutex_t mut;
  pthread_cond_t cond;
  unsigned int count, max, iteration;
} barrier_t;

#ifndef PTHREAD_BARRIER_SERIAL_THREAD
#define pthread_barrier_t barrier_t
#endif /* PTHREAD_BARRIER_SERIAL_THREAd */
#endif /* __MACH__ */

#define OK (0)
#define NOTOK (-1)

#define DEFAULT_STRING_SIZE 64

#define CSFILE_FD_R 1
#define CSFILE_FD_W 2
#define CSFILE_STD 3
#define CSFILE_SND_R 4
#define CSFILE_SND_W 5

#define MAXINSNO (200)
#define PMAX (1998)
#define VARGMAX (1999)
#define NOT_AN_INSTRUMENT INT32_MAX

#define ORTXT h.optext->t
#define INCOUNT ORTXT.inlist->count
#define OUTCOUNT ORTXT.outlist->count /* Not used */
// #define INOCOUNT    ORTXT.inoffs->count
// #define OUTOCOUNT   ORTXT.outoffs->count
#define INOCOUNT ORTXT.inArgCount
#define OUTOCOUNT ORTXT.outArgCount
#define IS_ASIG_ARG(x) (csoundGetTypeForArg(x) == &CS_VAR_TYPE_A)
#define IS_STR_ARG(x) (csoundGetTypeForArg(x) == &CS_VAR_TYPE_S)

#define CURTIME (((double)csound->icurTime) / ((double)csound->esr))
#define CURTIME_inc (((double)csound->ksmps) / ((double)csound->esr))

#ifdef B64BIT
#define MAXLEN 0x10000000
#define FMAXLEN ((MYFLT)(MAXLEN))
#define PHMASK 0x0fffffff
#else
#define MAXLEN 0x1000000L
#define FMAXLEN ((MYFLT)(MAXLEN))
#define PHMASK 0x0FFFFFFL
#endif

#define MAX_STRING_CHANNEL_DATASIZE 16384

#define PFRAC(x) ((MYFLT)((x)&ftp->lomask) * ftp->lodiv)
#define MAXPOS 0x7FFFFFFFL

#define BYTREVS(n) ((n >> 8 & 0xFF) | (n << 8 & 0xFF00))
#define BYTREVL(n)                                                             \
  ((n >> 24 & 0xFF) | (n >> 8 & 0xFF00L) | (n << 8 & 0xFF0000L) |              \
   (n << 24 & 0xFF000000L))

#define OCTRES 8192
#define CPSOCTL(n)                                                             \
  ((MYFLT)(1 << ((int)(n) >> 13)) * csound->cpsocfrc[(int)(n)&8191])

#define LOBITS 10
#define LOFACT 1024
/* LOSCAL is 1/LOFACT as MYFLT */
#define LOSCAL FL(0.0009765625)

#define LOMASK 1023

#ifdef USE_DOUBLE
extern int64_t MYNAN;
// #define SSTRCOD    (nan("0"))
#define SSTRCOD (double)NAN
#else
extern int32 MYNAN;
#define SSTRCOD (float)NAN
// #define SSTRCOD    (nanf("0"))
#endif
// #define ISSTRCOD(X) isnan(X)
// #ifndef __MACH__
extern int ISSTRCOD(MYFLT);
// #else
// #define ISSTRCOD(X) isnan(X)
// #endif

#define SSTRSIZ 1024
#define ALLCHNLS 0x7fff
#define DFLT_SR FL(44100.0)
#define DFLT_KR FL(4410.0)
#define DFLT_KSMPS 10
#define DFLT_NCHNLS 1
#define MAXCHNLS 256

#define MAXNAME (256)

#define DFLT_DBFS (FL(32768.0))

#define MAXOCTS 8
#define MAXCHAN 16 /* 16 MIDI channels; only one port for now */

/* A440 tuning factor */
#define ONEPT (csound->A4 / 430.5389646099018460319362438314060262605)
#define LOG10D20 0.11512925 /* for db to ampfac   */
#define DV32768 FL(0.000030517578125)

#ifndef PI
#define PI (3.141592653589793238462643383279502884197)
#endif /* pi */
#define TWOPI (6.283185307179586476925286766559005768394)
#define HALFPI (1.570796326794896619231321691639751442099)
#define PI_F ((MYFLT)PI)
#define TWOPI_F ((MYFLT)TWOPI)
#define HALFPI_F ((MYFLT)HALFPI)
#define INF (2147483647.0)
#define ROOT2 (1.414213562373095048801688724209698078569)

/* CONSTANTS FOR USE IN MSGLEVEL */
#define CS_AMPLMSG 01
#define CS_RNGEMSG 02
#define CS_WARNMSG 04
// #define CS_UNUSED1 08
#define CS_NOMSG 0x10
// #define CS_UNUSED2 0x20
#define CS_RAWMSG 0x40
#define CS_TIMEMSG 0x80
#define CS_NOQQ 0x400

#define IGN(X) (void)X

#define ARG_CONSTANT 0
#define ARG_STRING 1
#define ARG_PFIELD 2
#define ARG_GLOBAL 3
#define ARG_LOCAL 4
#define ARG_LABEL 5

#define ASYNC_GLOBAL 1
#define ASYNC_LOCAL 2

enum { FFT_LIB = 0, PFFT_LIB, VDSP_LIB };
enum { FFT_FWD = 0, FFT_INV };

/* advance declaration for
  API  message queue struct
*/
struct _message_queue;

//  typedef struct argoffs {
//    int     count;
//    int     indx[1];
//  } ARGOFFS;

typedef struct namedInstr {
  int32 instno;
  char *name;
  INSTRTXT *ip;
  struct namedInstr *next;
} INSTRNAME;

/**
 * A chain of TEXT structs. Note that this is identical with the first two
 * members of struct INSTRTEXT, and is so typecast at various points in code.
 */
typedef struct op {
  struct op *nxtop;
  TEXT t;
} OPTXT;

typedef struct fdch {
  struct fdch *nxtchp;
  /** handle returned by csound->FileOpen() */
  void *fd;
} FDCH;

typedef struct auxch {
  struct auxch *nxtchp;
  size_t size;
  void *auxp, *endp;
} AUXCH;

/**  this callback is used to notify the
     availability of new storage in AUXCH *.
     It can be used to swap the old storage
     for the new one and return it for deallocation.
*/
typedef AUXCH *(*aux_cb)(CSOUND *, void *, AUXCH *);

/**
 * AuxAllocAsync data
 */
typedef struct {
  CSOUND *csound;
  size_t nbytes;
  AUXCH *auxchp;
  void *userData;
  aux_cb notify;
} AUXASYNC;

typedef struct {
  int dimensions;
  int *sizes; /* size of each dimensions */
  int arrayMemberSize;
  CS_TYPE *arrayType;
  MYFLT *data;
  size_t allocated;
  //    AUXCH   aux;
} ARRAYDAT;

typedef struct {
  int size; /* 0...size-1 */
  MYFLT *data;
  AUXCH aux;
} TABDAT;

#define MAX_STRINGDAT_SIZE 0xFFFFFFFF

typedef struct {
  char *data;
  size_t size;
  int64_t timestamp; /*  VL: Feb 22 starting in 7.0 we have a timestamp */
} STRINGDAT;

/**
 * This struct holds the data for one score event.
 */
typedef struct event {
  /** String argument(s) (NULL if none) */
  int scnt;
  char *strarg;
  /* instance pointer */
  void *pinstance; /* used in nstance opcode */
  /** Event type */
  char opcod;
  /** Number of p-fields */
  int16 pcnt;
  /** Event start time */
  MYFLT p2orig;
  /** Length */
  MYFLT p3orig;
  /** All p-fields for this event (SSTRCOD: string argument) */
  MYFLT p[PMAX + 1];
  union { /* To ensure size is same as earlier */
    MYFLT *extra;
    MYFLT p[2];
  } c;
} EVTBLK;
/**
 * This struct holds the info for a concrete instrument event
 * instance in performance.
 */
typedef struct insds {
  /* Chain of init-time opcodes */
  struct opds *nxti;
  /* Chain of performance-time opcodes */
  struct opds *nxtp;
  /* Next allocated instance */
  struct insds *nxtinstance;
  /* Previous allocated instance */
  struct insds *prvinstance;
  /* Next in list of active instruments */
  struct insds *nxtact;
  /* Previous in list of active instruments */
  struct insds *prvact;
  /* Next instrument to terminate */
  struct insds *nxtoff;
  /* Chain of files used by opcodes in this instr */
  FDCH *fdchp;
  /* Extra memory used by opcodes in this instr */
  AUXCH *auxchp;
  /* Extra release time requested with xtratim opcode */
  int xtratim;
  /* MIDI note info block if event started from MIDI */
  MCHNBLK *m_chnbp;
  /* ptr to next overlapping MIDI voice */
  struct insds *nxtolap;
  /* Instrument number */
  int16 insno;
  /* Instrument def address */
  INSTRTXT *instr;
  /* non-zero for sustaining MIDI note */
  int16 m_sust;
  /* MIDI pitch, for simple access */
  unsigned char m_pitch;
  /* ...ditto velocity */
  unsigned char m_veloc;
  /* Flag to indicate we are releasing, test with release opcode */
  char relesing;
  /* Set if instr instance is active (perfing) */
  char actflg;
  /* Time to turn off event, in score beats */
  double offbet;
  /* Time to turn off event, in seconds (negative on indef/tie) */
  double offtim;
  /* Python namespace for just this instance. */
  void *pylocal;
  /* pointer to Csound engine and API for externals */
  CSOUND *csound;
  uint64_t kcounter;
  unsigned int ksmps; /* Instrument copy of ksmps */
  MYFLT ekr;          /* and of rates */
  MYFLT onedksmps, onedkr, kicvt;
  struct opds *pds;    /* Used for jumping */
  MYFLT scratchpad[4]; /* Persistent data */

  /* user defined opcode I/O buffers */
  void *opcod_iobufs;
  void *opcod_deact, *subins_deact;
  /* opcodes to be run at note deactivation */
  void *nxtd;
  uint32_t ksmps_offset; /* ksmps offset for sample accuracy */
  uint32_t no_end;       /* samps left at the end for sample accuracy
                            (calculated) */
  uint32_t ksmps_no_end; /* samps left at the end for sample accuracy
                            (used by opcodes) */
  MYFLT *spin;           /* offset into csound->spin */
  MYFLT *spout;          /* offset into csound->spout, or local spout */
  int init_done;
  int tieflag;
  int reinitflag;
  MYFLT retval;
  MYFLT *lclbas; /* base for variable memory pool */
  char *strarg;  /* string argument */
  /* Copy of required p-field values for quick access */
  CS_VAR_MEM p0;
  CS_VAR_MEM p1;
  CS_VAR_MEM p2;
  CS_VAR_MEM p3;
} INSDS;

#define CS_KSMPS (p->h.insdshead->ksmps)
#define CS_KCNT (p->h.insdshead->kcounter)
#define CS_EKR (p->h.insdshead->ekr)
#define CS_ONEDKSMPS (p->h.insdshead->onedksmps)
#define CS_ONEDKR (p->h.insdshead->onedkr)
#define CS_KICVT (p->h.insdshead->kicvt)
#define CS_ESR (csound->esr)
#define CS_PDS (p->h.insdshead->pds)
#define CS_SPIN (p->h.insdshead->spin)
#define CS_SPOUT (p->h.insdshead->spout)
typedef int (*SUBR)(CSOUND *, void *);

/**
 * This struct holds the info for one opcode in a concrete
 * instrument instance in performance.
 */
typedef struct opds {
  /** Next opcode in init-time chain */
  struct opds *nxti;
  /** Next opcode in perf-time chain */
  struct opds *nxtp;
  /** Initialization (i-time) function pointer */
  SUBR iopadr;
  /** Perf-time (k- or a-rate) function pointer */
  SUBR opadr;
  /** Orch file template part for this opcode */
  OPTXT *optext;
  /** Owner instrument instance data structure */
  INSDS *insdshead;
} OPDS;

typedef struct lblblk {
  OPDS h;
  OPDS *prvi;
  OPDS *prvp;
} LBLBLK;

typedef struct {
  MYFLT *begp, *curp, *endp, feedback[6];
  int32 scount;
} OCTDAT;

typedef struct {
  int32 npts, nocts, nsamps;
  MYFLT lofrq, hifrq, looct, srate;
  OCTDAT octdata[MAXOCTS];
  AUXCH auxch;
} DOWNDAT;

typedef struct {
  uint32_t ktimstamp, ktimprd;
  int32 npts, nfreqs, dbout;
  DOWNDAT *downsrcp;
  AUXCH auxch;
} SPECDAT;

typedef struct {
  MYFLT gen01;
  MYFLT ifilno;
  MYFLT iskptim;
  MYFLT iformat;
  MYFLT channel;
  MYFLT sample_rate;
  char strarg[SSTRSIZ];
} GEN01ARGS;

typedef struct {
  /** table length, not including the guard point */
  uint32_t flen;
  /** length mask ( = flen - 1) for power of two table size, 0 otherwise */
  int32 lenmask;
  /** log2(MAXLEN / flen) for power of two table size, 0 otherwise */
  int32 lobits;
  /** 2^lobits - 1 */
  int32 lomask;
  /** 1 / 2^lobits */
  MYFLT lodiv;
  /** LOFACT * (table_sr / orch_sr), cpscvt = cvtbas / base_freq */
  MYFLT cvtbas, cpscvt;
  /** sustain loop mode (0: none, 1: forward, 2: forward and backward) */
  int16 loopmode1;
  /** release loop mode (0: none, 1: forward, 2: forward and backward) */
  int16 loopmode2;
  /** sustain loop start and end in sample frames */
  int32 begin1, end1;
  /** release loop start and end in sample frames */
  int32 begin2, end2;
  /** sound file length in sample frames (flenfrms = soundend - 1) */
  int32 soundend, flenfrms;
  /** number of channels */
  int32 nchanls;
  /** table number */
  int32 fno;
  /** args  */
  MYFLT args[PMAX - 4];
  /** arg count */
  int argcnt;
  /** GEN01 parameters */
  GEN01ARGS gen01args;
  /** table data (flen + 1 MYFLT values) */
  MYFLT *ftable;
} FUNC;

typedef struct {
  CSOUND *csound;
  int32 flen;
  int fno, guardreq;
  EVTBLK e;
} FGDATA;

typedef struct {
  char *name;
  int (*fn)(FGDATA *, FUNC *);
} NGFENS;

typedef int (*GEN)(FGDATA *, FUNC *);

typedef struct MEMFIL {
  char filename[256]; /* Made larger RWD */
  char *beginp;
  char *endp;
  int32 length;
  struct MEMFIL *next;
} MEMFIL;

typedef struct {
  int16 type;
  int16 chan;
  int16 dat1;
  int16 dat2;
} MEVENT;

#define MARGS (3)
#define MAX_INCLUDE_DEPTH 100
struct MACRO;

typedef struct MACRON {
  int n;
  unsigned int line;
  struct MACRO *s;
  char *path;
  int included;
} MACRON;

typedef struct MACRO { /* To store active macros */
  char *name;          /* Use is by name */
  int acnt;            /* Count of arguments */
  char *body;          /* The text of the macro */
  struct MACRO *next;  /* Chain of active macros */
  int margs;           /* amount of space for args */
  char *arg[MARGS];    /* With these arguments */
} MACRO;

typedef struct in_stack_s { /* Stack of active inputs */
  int16 is_marked_repeat;   /* 1 if this input created by 'n' stmnt */
  int16 args;               /* Argument count for macro */
  // CORFIL      *cf;                  /* In core file */
  // void        *fd;                  /* for closing stream */
  MACRO *mac;
  int line;
  int32 oposit;
} IN_STACK;

typedef struct marked_sections {
  char *name;
  int32 posit;
  int line;
} MARKED_SECTIONS;

typedef struct namelst {
  char *name;
  struct namelst *next;
} NAMELST;

typedef struct NAME__ {
  char *namep;
  struct NAME__ *nxt;
  int type, count;
} NAME;

/* Holds UDO information, when an instrument is
   defined as a UDO
*/
typedef struct opcodinfo {
  int32 instno;
  char *name, *intypes, *outtypes;
  int16 inchns, outchns;
  CS_VAR_POOL *out_arg_pool;
  CS_VAR_POOL *in_arg_pool;
  INSTRTXT *ip;
  struct opcodinfo *prv;
} OPCODINFO;

/**
 * This struct will hold the current engine state after compilation
 */
typedef struct engine_state {
  CS_VAR_POOL *varPool; /* global variable pool */
  CS_HASH_TABLE *constantsPool;
  CS_HASH_TABLE *stringPool;
  int maxopcno;
  INSTRTXT **instrtxtp; /* instrument list      */
  INSTRTXT instxtanchor;
  CS_HASH_TABLE *instrumentNames; /* instrument names */
  int maxinsno;
} ENGINE_STATE;

/**
 * Nen FFT interface
 */
typedef struct _FFT_SETUP {
  int N, M;
  void *setup;
  MYFLT *buffer;
  int lib;
  int d;
  int p2;
} CSOUND_FFT_SETUP;

/**
 * plugin module info
 */
typedef struct {
  char module[12];
  char type[12];
} MODULE_INFO;

#define MAX_ALLOC_QUEUE 1024

typedef struct _alloc_data_ {
  int type;
  int insno;
  EVTBLK blk;
  MCHNBLK *chn;
  MEVENT mep;
  INSDS *ip;
  OPDS *ids;
} ALLOC_DATA;

#define MAX_MESSAGE_STR 1024
typedef struct _message_queue_t_ {
  int attr;
  char str[MAX_MESSAGE_STR];
} message_string_queue_t;

#define OPCODE_INIT_FUNCTION(function_name)                                  \
    int32_t function_name(CSOUND *, OENTRY **);

// declare an external function to init opcodes (defined elsewhere)
#define EXTERN_INIT_FUNCTION(function_name)                                  \
    extern int32_t function_name(CSOUND *, OENTRY **);

/*
 * Move the C++ guards to enclose the entire file,
 * in order to enable C++ to #include this file.
 */

/*
 * Move the C++ guards to enclose the entire file,
 * in order to enable C++ to #include this file.
 */

#define LINKAGE_BUILTIN_FOR(a_function, opcodes)                               \
  int32_t a_function(CSOUND *csound, OENTRY **ep) {                               \
    (void)csound;                                                              \
    *ep = opcodes;                                                             \
    return (int32_t)(sizeof(opcodes));                                            \
  }

#define LINKAGE_BUILTIN(name) LINKAGE_BUILTIN_FOR(name##_init, name)

#define FLINKAGE_BUILTIN(name)                                                 \
  NGFENS *name##_init(CSOUND *csound) {                                        \
    (void)csound;                                                              \
    return name;                                                               \
  }

#ifdef __cplusplus
}
#endif /* __cplusplus */

