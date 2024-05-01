#pragma once

#include "csound.h"


#ifdef CUSTOM_MALLOC
#ifndef MALLOC_BASE
#define MALLOC_BASE 0xC0000000  // STM32H7
#endif

static unsigned long cur = MALLOC_BASE;

void *my_malloc(unsigned long bytes) {
  unsigned long tmp = cur;
  cur += bytes;
  return (void *) tmp;
}

void *my_calloc(unsigned long items, unsigned long bytes) {
  unsigned long tmp = cur;
  cur += bytes*items;
  memset((void *) tmp, 0, bytes*items);
  return (void *) tmp;  
}

void *my_realloc(void *old, unsigned long bytes) {
    unsigned long tmp = cur;
    cur += bytes;
    memcpy((void *) tmp, old, bytes*items);
    return (void *) tmp;
}

void my_free(void *old) {
  // nothing to do
  // TODO: implement freeing
  return;
}

#define CS_MALLOC mymalloc
#define CS_CALLOC mycalloc
#define CS_REALLOC myrealloc
#define CS_FREE myfree
#else
#define CS_MALLOC malloc
#define CS_CALLOC calloc
#define CS_REALLOC realloc
#define CS_FREE free
#endif

#define MEMALLOC_DB (csound->memalloc_db)

typedef struct memAllocBlock_s {
#ifdef MEMDEBUG
    int                     magic;      /* 0x6D426C6B ("mBlk")          */
    void                    *ptr;       /* pointer to allocated area    */
#endif
    struct memAllocBlock_s  *prv;       /* previous structure in chain  */
    struct memAllocBlock_s  *nxt;       /* next structure in chain      */
} memAllocBlock_t;

void    *mmallocDebug(CSOUND *, size_t, char*, int);
void    *mcallocDebug(CSOUND *, size_t, char*, int);
void    *mreallocDebug(CSOUND *, void *, size_t, char*, int);
void    mfreeDebug(CSOUND *, void *, char*, int);
void memRESET(CSOUND *csound);