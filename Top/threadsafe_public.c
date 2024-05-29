#include "threadsafe_public.h"

#include "bus.h"
#include "bus_public.h"
#include "csound.h"
#include "csoundCore_internal.h"
#include "csound_orc_compile.h"
#include "csound_orc_semantics_public.h"
#include "csound_threads.h"
#include "insert.h"
#include "linevent_public.h"
#include "prototyp.h"
#include "threadsafe.h"
#include "csound_internal.h"

int csoundCompileTree(CSOUND *csound, TREE *root) {
  int async = 0;
  return csoundCompileTreeInternal(csound, root, async);
}

int csoundCompileOrc(CSOUND *csound, const char *str) {
  int async = 0;
  return csoundCompileOrcInternal(csound, str, async);
}

int csoundCompileOrcAsync(CSOUND *csound, const char *str) {
  int async = 1;
  return csoundCompileOrcInternal(csound, str, async);
}

MYFLT csoundEvalCode(CSOUND *csound, const char *str) {
  int async = 0;
  if (str && csoundCompileOrcInternal(csound, str, async) == CSOUND_SUCCESS) {
    if (!(csound->engineStatus & CS_STATE_COMP)) {
      init0(csound);
    }
    return csound->instr0->instance[0].retval;
  }
#ifdef NAN
  else
    return NAN;
#else
  else
    return 0;
#endif
}

int csoundReadScore(CSOUND *csound, const char *message) {
  int res;
  csoundLockMutex(csound->API_lock);
  res = csoundReadScoreInternal(csound, message);
  csoundUnlockMutex(csound->API_lock);
  return res;
}

void csoundReadScoreAsync(CSOUND *csound, const char *message) {
  csoundReadScore_enqueue(csound, message);
}

/* VL: the following do not depend on API_lock
   therefore do not need to be in the message queue
*/

MYFLT csoundGetControlChannel(CSOUND *csound, const char *name, int *err) {
  MYFLT *pval;
  int err_;
  union {
    MYFLT d;
    MYFLT_INT_TYPE i;
  } x;
  x.d = FL(0.0);
  if (UNLIKELY(strlen(name) == 0))
    return FL(.0);
  if ((err_ = csoundGetChannelPtr(
           csound, &pval, name,
           CSOUND_CONTROL_CHANNEL | CSOUND_OUTPUT_CHANNEL)) == CSOUND_SUCCESS) {
#if defined(MSVC)
    x.i = InterlockedExchangeAdd64((MYFLT_INT_TYPE *)pval, 0);
#elif defined(HAVE_ATOMIC_BUILTIN)
    x.i = __atomic_load_n((MYFLT_INT_TYPE *)pval, __ATOMIC_SEQ_CST);
#else
    x.d = *pval;
#endif
  }
  if (err) {
    *err = err_;
  }
  return x.d;
}

void csoundSetControlChannel(CSOUND *csound, const char *name, MYFLT val) {
  MYFLT *pval;
#if defined(MSVC) || defined(HAVE_ATOMIC_BUILTIN)
  union {
    MYFLT d;
    MYFLT_INT_TYPE i;
  } x;
  x.d = val;
#endif
  if (csoundGetChannelPtr(csound, &pval, name,
                          CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL) ==
      CSOUND_SUCCESS)

#if defined(MSVC)
    InterlockedExchange64((MYFLT_INT_TYPE *)pval, x.i);
#elif defined(HAVE_ATOMIC_BUILTIN)
    __atomic_store_n((MYFLT_INT_TYPE *)pval, x.i, __ATOMIC_SEQ_CST);
#else
  {
    spin_lock_t *lock =
        (spin_lock_t *)csoundGetChannelLock(csound, (char *)name);
    csoundSpinLock(lock);
    *pval = val;
    csoundSpinUnLock(lock);
  }
#endif
}

void csoundGetAudioChannel(CSOUND *csound, const char *name, MYFLT *samples) {

  MYFLT *psamples;
  if (strlen(name) == 0)
    return;
  if (csoundGetChannelPtr(csound, &psamples, name,
                          CSOUND_AUDIO_CHANNEL | CSOUND_OUTPUT_CHANNEL) ==
      CSOUND_SUCCESS) {
    spin_lock_t *lock =
        (spin_lock_t *)csoundGetChannelLock(csound, (char *)name);
    csoundSpinLock(lock);
    memcpy(samples, psamples, csoundGetKsmps(csound) * sizeof(MYFLT));
    csoundSpinUnLock(lock);
  }
}

void csoundSetAudioChannel(CSOUND *csound, const char *name, MYFLT *samples) {
  MYFLT *psamples;
  if (csoundGetChannelPtr(csound, &psamples, name,
                          CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL) ==
      CSOUND_SUCCESS) {
    spin_lock_t *lock =
        (spin_lock_t *)csoundGetChannelLock(csound, (char *)name);
    csoundSpinLock(lock);
    memcpy(psamples, samples, csoundGetKsmps(csound) * sizeof(MYFLT));
    csoundSpinUnLock(lock);
  }
}

void csoundGetStringChannel(CSOUND *csound, const char *name, char *string) {
  MYFLT *pstring;
  char *chstring;
  int n2;
  if (strlen(name) == 0)
    return;
  if (csoundGetChannelPtr(csound, &pstring, name,
                          CSOUND_STRING_CHANNEL | CSOUND_OUTPUT_CHANNEL) ==
      CSOUND_SUCCESS) {
    spin_lock_t *lock =
        (spin_lock_t *)csoundGetChannelLock(csound, (char *)name);
    chstring = ((STRINGDAT *)pstring)->data;
    if (lock != NULL)
      csoundSpinLock(lock);
    if (string != NULL && chstring != NULL) {
      n2 = strlen(chstring);
      strNcpy(string, chstring, n2 + 1);
      // string[n2] = '\0';
    }
    if (lock != NULL)
      csoundSpinUnLock(lock);
  }
}

void csoundSetStringChannel(CSOUND *csound, const char *name, char *string) {
  MYFLT *pstring;

  if (csoundGetChannelPtr(csound, &pstring, name,
                          CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL) ==
      CSOUND_SUCCESS) {

    STRINGDAT *stringdat = (STRINGDAT *)pstring;
    int size = stringdat->size; // csoundGetChannelDatasize(csound, name);
    spin_lock_t *lock =
        (spin_lock_t *)csoundGetChannelLock(csound, (char *)name);

    if (lock != NULL) {
      csoundSpinLock(lock);
    }

    if (strlen(string) + 1 > (unsigned int)size) {
      if (stringdat->data != NULL)
        mfree(csound, stringdat->data);
      stringdat->data = cs_strdup(csound, string);
      stringdat->size = strlen(string) + 1;
      // set_channel_data_ptr(csound,name,(void*)pstring, strlen(string)+1);
    } else {
      strcpy((char *)stringdat->data, string);
    }

    if (lock != NULL) {
      csoundSpinUnLock(lock);
    }
  }
}

PUBLIC int csoundSetPvsChannel(CSOUND *csound, const PVSDATEXT *fin,
                               const char *name) {
  MYFLT *pp;
  PVSDATEXT *f;
  if (LIKELY(csoundGetChannelPtr(csound, &pp, name,
                                 CSOUND_PVS_CHANNEL | CSOUND_INPUT_CHANNEL) ==
             CSOUND_SUCCESS)) {
    spin_lock_t *lock = (spin_lock_t *)csoundGetChannelLock(csound, name);
    f = (PVSDATEXT *)pp;
    csoundSpinLock(lock);

    if (f->frame == NULL) {
      f->frame = mcalloc(csound, sizeof(float) * (fin->N + 2));
    } else if (f->N < fin->N) {
      f->frame = mrealloc(csound, f->frame, sizeof(float) * (fin->N + 2));
    }

    memcpy(f, fin, sizeof(PVSDATEXT) - sizeof(float *));
    if (fin->frame != NULL)
      memcpy(f->frame, fin->frame, (f->N + 2) * sizeof(float));
    csoundSpinUnLock(lock);
  } else {
    return CSOUND_ERROR;
  }
  return CSOUND_SUCCESS;
}

PUBLIC int csoundGetPvsChannel(CSOUND *csound, PVSDATEXT *fout,
                               const char *name) {
  MYFLT *pp;
  PVSDATEXT *f;
  if (UNLIKELY(
          csoundGetChannelPtr(csound, &pp, name,
                              CSOUND_PVS_CHANNEL | CSOUND_OUTPUT_CHANNEL) ==
          CSOUND_SUCCESS)) {
    spin_lock_t *lock = (spin_lock_t *)csoundGetChannelLock(csound, name);
    f = (PVSDATEXT *)pp;
    if (UNLIKELY(pp == NULL))
      return CSOUND_ERROR;
    csoundSpinLock(lock);
    memcpy(fout, f, sizeof(PVSDATEXT) - sizeof(float *));
    if (fout->frame != NULL && f->frame != NULL)
      memcpy(fout->frame, f->frame, sizeof(float) * (fout->N));
    csoundSpinUnLock(lock);
  } else {
    return CSOUND_ERROR;
  }
  return CSOUND_SUCCESS;
}

int csoundScoreEvent(CSOUND *csound, char type, const MYFLT *pfields,
                     long numFields) {

  csoundLockMutex(csound->API_lock);
  csoundScoreEventInternal(csound, type, pfields, numFields);
  csoundUnlockMutex(csound->API_lock);
  return OK;
}

void csoundScoreEventAsync(CSOUND *csound, char type, const MYFLT *pfields,
                           long numFields) {
  csoundScoreEvent_enqueue(csound, type, pfields, numFields);
}

int csoundScoreEventAbsolute(CSOUND *csound, char type, const MYFLT *pfields,
                             long numFields, double time_ofs) {
  csoundLockMutex(csound->API_lock);
  csoundScoreEventAbsoluteInternal(csound, type, pfields, numFields, time_ofs);
  csoundUnlockMutex(csound->API_lock);
  return OK;
}

void csoundScoreEventAbsoluteAsync(CSOUND *csound, char type,
                                   const MYFLT *pfields, long numFields,
                                   double time_ofs) {

  csoundScoreEventAbsolute_enqueue(csound, type, pfields, numFields, time_ofs);
}

/*  VL: These functions are slated to
    be converted to message enqueueing
    in the next API revision.
*/
void csoundInputMessage(CSOUND *csound, const char *message) {
  csoundLockMutex(csound->API_lock);
  csoundInputMessageInternal(csound, message);
  csoundUnlockMutex(csound->API_lock);
}

/** Async versions of the functions above
    To be removed once everything is made async
*/
void csoundInputMessageAsync(CSOUND *csound, const char *message) {
  csoundInputMessage_enqueue(csound, message);
}

int csoundKillInstance(CSOUND *csound, MYFLT instr, char *instrName, int mode,
                       int allow_release) {
  int async = 0;
  return csoundKillInstanceInternal(csound, instr, instrName, mode,
                                    allow_release, async);
}

void csoundTableSet(CSOUND *csound, int table, int index, MYFLT value) {
  csoundLockMutex(csound->API_lock);
  csoundTableSetInternal(csound, table, index, value);
  csoundUnlockMutex(csound->API_lock);
}

void csoundTableCopyOut(CSOUND *csound, int table, MYFLT *ptable) {

  csoundLockMutex(csound->API_lock);
  csoundTableCopyOutInternal(csound, table, ptable);
  csoundUnlockMutex(csound->API_lock);
}

static inline void csoundTableCopyOut_enqueue(CSOUND *csound, int table,
                                              MYFLT *ptable) {
  const int argsize = ARG_ALIGN * 2;
  char args[ARG_ALIGN * 2];
  memcpy(args, &table, sizeof(int));
  memcpy(args + ARG_ALIGN, &ptable, sizeof(MYFLT *));
  message_enqueue(csound, TABLE_COPY_OUT, args, argsize);
}

void csoundTableCopyOutAsync(CSOUND *csound, int table, MYFLT *ptable) {
  csoundTableCopyOut_enqueue(csound, table, ptable);
}

void csoundTableCopyIn(CSOUND *csound, int table, MYFLT *ptable) {
  csoundLockMutex(csound->API_lock);
  csoundTableCopyInInternal(csound, table, ptable);
  csoundUnlockMutex(csound->API_lock);
}

static inline void csoundTableCopyIn_enqueue(CSOUND *csound, int table,
                                             MYFLT *ptable) {
  const int argsize = ARG_ALIGN * 2;
  char args[ARG_ALIGN * 2];
  memcpy(args, &table, sizeof(int));
  memcpy(args + ARG_ALIGN, &ptable, sizeof(MYFLT *));
  message_enqueue(csound, TABLE_COPY_IN, args, argsize);
}

void csoundTableCopyInAsync(CSOUND *csound, int table, MYFLT *ptable) {
  csoundTableCopyIn_enqueue(csound, table, ptable);
}

int csoundCompileTreeAsync(CSOUND *csound, TREE *root) {
  int async = 1;
  return csoundCompileTreeInternal(csound, root, async);
}