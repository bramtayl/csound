#pragma once

#include "csound.h"

/* ARG LIST ALIGNMENT */
#define ARG_ALIGN 8

enum {
  INPUT_MESSAGE = 1,
  READ_SCORE,
  SCORE_EVENT,
  SCORE_EVENT_ABS,
  TABLE_COPY_OUT,
  TABLE_COPY_IN,
  TABLE_SET,
  MERGE_STATE,
  KILL_INSTANCE
};

void message_dequeue(CSOUND *csound);

extern int64_t *csoundReadScore_enqueue(CSOUND *csound, const char *str);

extern int64_t *csoundScoreEvent_enqueue(CSOUND *csound, char type,
                                         const MYFLT *pfields, long numFields);

extern int64_t *csoundScoreEventAbsolute_enqueue(CSOUND *csound, char type,
                                                 const MYFLT *pfields,
                                                 long numFields,
                                                 double time_ofs);

extern void csoundInputMessage_enqueue(CSOUND *csound, const char *str);

void *message_enqueue(CSOUND *csound, int32_t message, char *args, int argsiz);

int csoundScoreEventInternal(CSOUND *csound, char type, const MYFLT *pfields,
                             long numFields);

int csoundScoreEventAbsoluteInternal(CSOUND *csound, char type,
                                     const MYFLT *pfields, long numFields,
                                     double time_ofs);

void csoundTableCopyOutInternal(CSOUND *csound, int table, MYFLT *ptable);

void csoundTableCopyInInternal(CSOUND *csound, int table, MYFLT *ptable);

void allocate_message_queue(CSOUND *csound);