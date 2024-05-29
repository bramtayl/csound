/*
 * threadsafe.c: threadsafe API functions
 *               Copyright (c) V Lazzarini, 2013
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
 */

#include "new_orc_parser.h"
#include "threadsafe.h"

#include "csoundCore_internal.h"
#include "csound_orc.h"
#include "linevent_public.h"
#include <stdlib.h>
#include "memalloc.h"
#include "csound_orc_semantics_public.h"
#include "csound_orc_compile.h"
#include "bus_public.h"
#include "insert.h"
#include "musmon.h"
#include "fgens_public.h"
#include "csound_threads.h"
#include "csound_internal.h"

void merge_state(CSOUND *csound, ENGINE_STATE *engineState,
                 TYPE_TABLE* typetable, OPDS *ids);
void killInstance(CSOUND *csound, MYFLT instr, int insno, INSDS *ip,
                  int mode, int allow_release);
void csoundInputMessageInternal(CSOUND *csound, const char *message);
void set_channel_data_ptr(CSOUND *csound, const char *name,
                          void *ptr, int newSize);

void named_instr_assign_numbers(CSOUND *csound, ENGINE_STATE *engineState);

/* MAX QUEUE SIZE */
#define API_MAX_QUEUE 1024

/* Message queue structure */
typedef struct _message_queue {
  int64_t message;  /* message id */
  char *args;   /* args, arg pointers */
  int64_t rtn;  /* return value */
} message_queue_t;


/* atomicGetAndIncrementWithModulus */
static long atomicGet_Incr_Mod(volatile long* val, long mod) {
  volatile long oldVal;
  long newVal;
  do {
    oldVal = *val;
    newVal = (oldVal + 1) % mod;
  } while (ATOMIC_CMP_XCH(val, newVal, oldVal));
  return oldVal;
}

int csoundScoreEventInternal(CSOUND *csound, char type, const MYFLT *pfields,
                             long numFields) {
  EVTBLK evt;
  int i;
  int ret;
  memset(&evt, 0, sizeof(EVTBLK));

  evt.strarg = NULL;
  evt.scnt = 0;
  evt.opcod = type;
  evt.pcnt = (int16)numFields;
  for (i = 0; i < (int)numFields; i++)
    evt.p[i + 1] = pfields[i];
  ret = insert_score_event_at_sample(csound, &evt, csound->icurTime);
  return ret;
}

int csoundScoreEventAbsoluteInternal(CSOUND *csound, char type,
                                     const MYFLT *pfields, long numFields,
                                     double time_ofs) {
  EVTBLK evt;
  int i;
  int ret;
  memset(&evt, 0, sizeof(EVTBLK));

  evt.strarg = NULL;
  evt.scnt = 0;
  evt.opcod = type;
  evt.pcnt = (int16)numFields;
  for (i = 0; i < (int)numFields; i++)
    evt.p[i + 1] = pfields[i];
  ret = insert_score_event(csound, &evt, time_ofs);
  return ret;
}

void csoundTableCopyOutInternal(CSOUND *csound, int table, MYFLT *ptable) {
  int len;
  MYFLT *ftab;
  /* in realtime mode init pass is executed in a separate thread, so
     we need to protect it */
  if (csound->oparms->realtime)
    csoundLockMutex(csound->init_pass_threadlock);
  len = csoundGetTable(csound, &ftab, table);
  if (UNLIKELY(len > 0x00ffffff))
    len = 0x00ffffff; // As coverity is unhappy
  memcpy(ptable, ftab, (size_t)(len * sizeof(MYFLT)));
  if (csound->oparms->realtime)
    csoundUnlockMutex(csound->init_pass_threadlock);
}

void csoundTableCopyInInternal(CSOUND *csound, int table, MYFLT *ptable) {
  int len;
  MYFLT *ftab;
  /* in realtime mode init pass is executed in a separate thread, so
     we need to protect it */
  if (csound->oparms->realtime)
    csoundLockMutex(csound->init_pass_threadlock);
  len = csoundGetTable(csound, &ftab, table);
  if (UNLIKELY(len > 0x00ffffff))
    len = 0x00ffffff; // As coverity is unhappy
  memcpy(ftab, ptable, (size_t)(len * sizeof(MYFLT)));
  if (csound->oparms->realtime)
    csoundUnlockMutex(csound->init_pass_threadlock);
}

/* called by csoundCreate() at the start
   and also by csoundStart() to cover de-allocation
   by reset
*/
void allocate_message_queue(CSOUND *csound) {
  if (csound->msg_queue == NULL) {
    int i;
    csound->msg_queue = (message_queue_t **)
      mcalloc(csound, sizeof(message_queue_t*)*API_MAX_QUEUE);
    for (i = 0; i < API_MAX_QUEUE; i++) {
      csound->msg_queue[i] =
        (message_queue_t*)
        mcalloc(csound, sizeof(message_queue_t));
    }
  }
}


/* enqueue should be called by the relevant API function */
void *message_enqueue(CSOUND *csound, int32_t message, char *args,
                      int argsiz) {
  if(csound->msg_queue != NULL) {
    int64_t *rtn;
    volatile long items;

    /* block if queue is full */
    do {
      items = ATOMIC_GET(csound->msg_queue_items);
    } while(items >= API_MAX_QUEUE);

    message_queue_t* msg =
      csound->msg_queue[atomicGet_Incr_Mod(&csound->msg_queue_wget,
                                           API_MAX_QUEUE)];
    msg->message = message;
    if(msg->args != NULL)
      mfree(csound, msg->args);
    msg->args = (char *)mcalloc(csound, argsiz);
    memcpy(msg->args, args, argsiz);
    rtn = &msg->rtn;
    csound->msg_queue[atomicGet_Incr_Mod(&csound->msg_queue_wput,
                                         API_MAX_QUEUE)] = msg;
    ATOMIC_INCR(csound->msg_queue_items);
    return (void *) rtn;
  }
  else return NULL;
}

/* dequeue should be called by kperf_*()
   NB: these calls are already in place
*/
void message_dequeue(CSOUND *csound) {
  if(csound->msg_queue != NULL) {
    long rp = csound->msg_queue_rstart;
    long items = csound->msg_queue_items;
    long rend = rp + items;

    while(rp < rend) {
      message_queue_t* msg = csound->msg_queue[rp % API_MAX_QUEUE];
      switch(msg->message) {
      case INPUT_MESSAGE:
        {
          const char *str = msg->args;
          csoundInputMessageInternal(csound, str);
        }

        break;
      case READ_SCORE:
        {
          const char *str = msg->args;
          csoundReadScoreInternal(csound, str);
        }
        break;
      case SCORE_EVENT:
        {
          char type;
          const MYFLT *pfields;
          long numFields;
          type = msg->args[0];
          memcpy(&pfields, msg->args + ARG_ALIGN,
                 sizeof(MYFLT *));
          memcpy(&numFields, msg->args + ARG_ALIGN*2,
                 sizeof(long));

          csoundScoreEventInternal(csound, type, pfields, numFields);
        }
        break;
      case SCORE_EVENT_ABS:
        {
          char type;
          const MYFLT *pfields;
          long numFields;
          double ofs;
          type = msg->args[0];
          memcpy(&pfields, msg->args + ARG_ALIGN,
                 sizeof(MYFLT *));
          memcpy(&numFields, msg->args + ARG_ALIGN*2,
                 sizeof(long));
          memcpy(&ofs, msg->args + ARG_ALIGN*3,
                 sizeof(double));

          csoundScoreEventAbsoluteInternal(csound, type, pfields, numFields,
                                             ofs);
        }
        break;
      case TABLE_COPY_OUT:
        {
          int table;
          MYFLT *ptable;
          memcpy(&table, msg->args, sizeof(int));
          memcpy(&ptable, msg->args + ARG_ALIGN,
                 sizeof(MYFLT *));
          csoundTableCopyOutInternal(csound, table, ptable);
        }
        break;
      case TABLE_COPY_IN:
        {
          int table;
          MYFLT *ptable;
          memcpy(&table, msg->args, sizeof(int));
          memcpy(&ptable, msg->args + ARG_ALIGN,
                 sizeof(MYFLT *));
          csoundTableCopyInInternal(csound, table, ptable);
        }
        break;
      case TABLE_SET:
        {
          int table, index;
          MYFLT value;
          memcpy(&table, msg->args, sizeof(int));
          memcpy(&index, msg->args + ARG_ALIGN,
                 sizeof(int));
          memcpy(&value, msg->args + 2*ARG_ALIGN,
                 sizeof(MYFLT));
          csoundTableSetInternal(csound, table, index, value);
        }
        break;
      case MERGE_STATE:
        {
          ENGINE_STATE *e;
          TYPE_TABLE *t;
          OPDS *ids;
          memcpy(&e, msg->args, sizeof(ENGINE_STATE *));
          memcpy(&t, msg->args + ARG_ALIGN,
                 sizeof(TYPE_TABLE *));
          memcpy(&ids, msg->args + 2*ARG_ALIGN,
                 sizeof(OPDS *));
          named_instr_assign_numbers(csound, e);
          merge_state(csound, e, t, ids);
        }
        break;
      case KILL_INSTANCE:
        {
          MYFLT instr;
          int mode, insno, rls;
          INSDS *ip;
          memcpy(&instr, msg->args, sizeof(MYFLT));
          memcpy(&insno, msg->args + ARG_ALIGN,
                 sizeof(int));
          memcpy(&ip, msg->args + ARG_ALIGN*2,
                 sizeof(INSDS *));
          memcpy(&mode, msg->args + ARG_ALIGN*3,
                 sizeof(int));
          memcpy(&rls, msg->args  + ARG_ALIGN*4,
                 sizeof(int));
          killInstance(csound, instr, insno, ip, mode, rls);
        }
        break;
      }
      msg->message = 0;
      rp += 1;
    }
    ATOMIC_SUB(csound->msg_queue_items, items);
    csound->msg_queue_rstart = rp % API_MAX_QUEUE;
  }
}

/* these are the message enqueueing functions for each relevant API function */
inline void csoundInputMessage_enqueue(CSOUND *csound,
                                              const char *str){
  message_enqueue(csound,INPUT_MESSAGE, (char *) str, strlen(str)+1);
}

inline int64_t *csoundReadScore_enqueue(CSOUND *csound, const char *str) {
  return message_enqueue(csound, READ_SCORE, (char *) str, strlen(str)+1);
}

static inline void csoundTableSet_enqueue(CSOUND *csound, int table, int index,
                                          MYFLT value)
{
  const int argsize = ARG_ALIGN*3;
  char args[ARG_ALIGN*3];
  memcpy(args, &table, sizeof(int));
  memcpy(args+ARG_ALIGN, &index, sizeof(int));
  memcpy(args+2*ARG_ALIGN, &value, sizeof(MYFLT));
  message_enqueue(csound,TABLE_SET, args, argsize);
}


inline int64_t *csoundScoreEvent_enqueue(CSOUND *csound, char type,
                                                const MYFLT *pfields,
                                                long numFields)
{
  const int argsize = ARG_ALIGN*3;
  char args[ARG_ALIGN*3];
  args[0] = type;
  memcpy(args+ARG_ALIGN, &pfields, sizeof(MYFLT *));
  memcpy(args+2*ARG_ALIGN, &numFields, sizeof(long));
  return message_enqueue(csound,SCORE_EVENT, args, argsize);
}


inline int64_t *csoundScoreEventAbsolute_enqueue(CSOUND *csound, char type,
                                                        const MYFLT *pfields,
                                                        long numFields,
                                                        double time_ofs)
{
  const int argsize = ARG_ALIGN*4;
  char args[ARG_ALIGN*4];
  args[0] = type;
  memcpy(args+ARG_ALIGN, &pfields, sizeof(MYFLT *));
  memcpy(args+2*ARG_ALIGN, &numFields, sizeof(long));
  memcpy(args+3*ARG_ALIGN, &time_ofs, sizeof(double));
  return message_enqueue(csound,SCORE_EVENT_ABS, args, argsize);
}

/* this is to be called from
   csoundKillInstanceInternal() in insert.c
*/
void killInstance_enqueue(CSOUND *csound, MYFLT instr, int insno,
                          INSDS *ip, int mode,
                          int allow_release) {
  const int argsize = ARG_ALIGN*5;
  char args[ARG_ALIGN*5];
  memcpy(args, &instr, sizeof(int));
  memcpy(args+ARG_ALIGN, &insno, sizeof(int));
  memcpy(args+ARG_ALIGN*2, &ip, sizeof(INSDS *));
  memcpy(args+ARG_ALIGN*3, &mode, sizeof(int));
  memcpy(args+ARG_ALIGN*4, &allow_release, sizeof(int));
  message_enqueue(csound,KILL_INSTANCE,args,argsize);
}

/* this is to be called from
   csoundCompileTreeInternal() in csound_orc_compile.c
*/
void mergeState_enqueue(CSOUND *csound, ENGINE_STATE *e, TYPE_TABLE* t, OPDS *ids) {
  const int argsize = ARG_ALIGN*3;
  char args[ARG_ALIGN*3];
  memcpy(args, &e, sizeof(ENGINE_STATE *));
  memcpy(args+ARG_ALIGN, &t, sizeof(TYPE_TABLE *));
  memcpy(args+2*ARG_ALIGN, &ids, sizeof(OPDS *));
  message_enqueue(csound,MERGE_STATE, args, argsize);
}

int init0(CSOUND *csound);

void csoundTableSetAsync(CSOUND *csound, int table, int index, MYFLT value)
{
  csoundTableSet_enqueue(csound, table, index, value);
}

int csoundKillInstanceAsync(CSOUND *csound, MYFLT instr, char *instrName,
                            int mode, int allow_release){
  int async = 1;
  return csoundKillInstanceInternal(csound, instr, instrName, mode,
                                    allow_release, async);
}
