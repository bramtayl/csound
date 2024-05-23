/*  bus.c:

 Copyright (C) 2004 John ffitch
 (C) 2005, 2006 Istvan Varga
 (c) 2006 Victor Lazzarini

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

/*                      BUS.C           */
#include "csoundCore_internal.h"
#include <setjmp.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#if defined(NACL) || defined(__wasi__)
#include <sys/select.h>
#endif

#include "bus.h"
#include "namedins.h"
#include "memalloc.h"
#include "csound_orc_semantics_public.h"
#include "auxfd.h"
#include "insert_public.h"
#include "bus_public.h"

/* For sensing opcodes */
#if defined(__unix) || defined(__unix__) || defined(__MACH__)
#  ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#  endif
#  ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#  endif
#  ifdef HAVE_TERMIOS_H
#    include <termios.h>
#  endif
#elif defined(_WIN32)
#  include <conio.h>
#endif


/* ------------------------------------------------------------------------ */

#ifdef HAVE_C99
#  ifdef MYFLT2LRND
#    undef MYFLT2LRND
#  endif
#  ifndef USE_DOUBLE
#    define MYFLT2LRND  lrintf
#  else
#    define MYFLT2LRND  lrint
#  endif
#endif

int32_t chani_opcode_perf_k(CSOUND *csound, CHNVAL *p)
{
    int32_t     n = (int32_t)MYFLT2LRND(*(p->a));
    char chan_name[16];
    int32_t   err;
    MYFLT *val;

    if (UNLIKELY(n < 0))
        return csoundPerfError(csound, &(p->h),Str("chani: invalid index"));

    snprintf(chan_name, 16, "%i", n);
    err = csoundGetChannelPtr(csound, &val, chan_name,
                              CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL);

    if (UNLIKELY(err))
        return csoundPerfError(csound, &(p->h),
                                 Str("chani error %d:"
                                     "channel not found or not right type"), err);
    *(p->r) = *val;
    return OK;
}

int32_t chano_opcode_perf_k(CSOUND *csound, CHNVAL *p)
{
    int32_t     n = (int32_t)MYFLT2LRND(*(p->a));
    char chan_name[16];
    int32_t   err;
    MYFLT *val;

    if (UNLIKELY(n < 0))
        return csoundPerfError(csound,&(p->h),Str("chani: invalid index"));

    snprintf(chan_name, 16, "%i", n);
    err = csoundGetChannelPtr(csound, &val, chan_name,
                              CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL);

    if (UNLIKELY(err))
        return csoundPerfError(csound, &(p->h),
                                 Str("chano error %d:"
                                     "channel not found or not right type"), err);
    *val = *(p->r);
    return OK;
}

int32_t chani_opcode_perf_a(CSOUND *csound, CHNVAL *p)
{
    int32_t     n = (int32_t)MYFLT2LRND(*(p->a));
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;

    char chan_name[16];
    int32_t   err;
    MYFLT *val;

    if (UNLIKELY(n < 0))
        return csoundPerfError(csound, &(p->h),Str("chani: invalid index"));

    snprintf(chan_name, 16, "%i", n);
    err = csoundGetChannelPtr(csound, &val, chan_name,
                              CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL);
    if (UNLIKELY(err))
        return csoundPerfError(csound, &(p->h),
                                 Str("chani error %d:"
                                     "channel not found or not right type"), err);
    if (UNLIKELY(offset)) memset(p->r, '\0', offset * sizeof(MYFLT));
    memcpy(&p->r[offset], &val[offset],
           sizeof(MYFLT) * (CS_KSMPS-offset-early));
    if (UNLIKELY(early))
        memset(&p->r[CS_KSMPS-early], '\0', early * sizeof(MYFLT));
    return OK;
}

int32_t chano_opcode_perf_a(CSOUND *csound, CHNVAL *p)
{
    int32_t     n = (int32_t)MYFLT2LRND(*(p->a));
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;

    char chan_name[16];
    int32_t   err;
    MYFLT *val;

    if (UNLIKELY(n < 0))
        return csoundPerfError(csound, &(p->h),Str("chani: invalid index"));

    snprintf(chan_name, 16, "%i", n);
    err = csoundGetChannelPtr(csound, &val, chan_name,
                              CSOUND_AUDIO_CHANNEL | CSOUND_OUTPUT_CHANNEL);
    if (UNLIKELY(err))
        return csoundPerfError(csound, &(p->h),
                                 Str("chano error %d:"
                                     "channel not found or not right type"), err);

    if (UNLIKELY(offset)) memset(&val, '\0', offset * sizeof(MYFLT));
    memcpy(&val[offset], &p->r[offset],
           sizeof(MYFLT) * (CS_KSMPS-offset-early));

    if (UNLIKELY(early))
        memset(&val[CS_KSMPS-early], '\0', early * sizeof(MYFLT));
    return OK;
}

int32_t pvsin_init(CSOUND *csound, FCHAN *p)
{
    int32_t N;
    MYFLT *pp;
    PVSDATEXT *f;
    int32_t     n = (int32_t)MYFLT2LRND(*(p->a));
    char name[16];
    snprintf(name, 16, "%i", n);
    if (csoundGetChannelPtr(csound, &pp, name,
                            CSOUND_PVS_CHANNEL | CSOUND_INPUT_CHANNEL)
        == CSOUND_SUCCESS){
        spin_lock_t  *lock = (spin_lock_t *)
                csoundGetChannelLock(csound, name);
        f = (PVSDATEXT *) pp;
        csoundSpinLock(lock);
        memcpy(&(p->init), f, sizeof(PVSDAT)-sizeof(AUXCH));
        csoundSpinUnLock(lock);
    }

    N = p->init.N = (int32_t)(*p->N ? *p->N : p->init.N);
    p->init.overlap = (int32_t) (*p->overlap ? *p->overlap : p->init.overlap);
    p->init.winsize = (int32_t) (*p->winsize ? *p->winsize : p->init.winsize);
    p->init.wintype = (int32_t)(*p->wintype);
    p->init.format = (int32_t)(*p->format);
    p->init.framecount = 0;
    memcpy(p->r, &p->init, sizeof(PVSDAT)-sizeof(AUXCH));
    if (p->r->frame.auxp == NULL ||
        p->r->frame.size < sizeof(float) * (N + 2))
        csoundAuxAlloc(csound, (N + 2) * sizeof(float), &p->r->frame);
    return OK;
}

int32_t pvsin_perf(CSOUND *csound, FCHAN *p)
{
    PVSDAT *fout = p->r;
    int32_t     n = (int32_t)MYFLT2LRND(*(p->a));
    char chan_name[16];
    int32_t   err, size;
    PVSDATEXT *fin;
    MYFLT      *pp;

    if (UNLIKELY(n < 0))
        return csoundPerfError(csound, &(p->h),Str("pvsin: invalid index"));

    snprintf(chan_name, 16, "%i", n);
    err = csoundGetChannelPtr(csound, &pp, chan_name,
                              CSOUND_PVS_CHANNEL | CSOUND_INPUT_CHANNEL);
    fin = (PVSDATEXT *) pp;
    if (UNLIKELY(err))
        return csoundPerfError(csound, &(p->h),
                                 Str("pvsin error %d:"
                                     "channel not found or not right type"), err);

    size = fin->N < fout->N ? fin->N : fout->N;
    spin_lock_t  *lock = (spin_lock_t *) csoundGetChannelLock(csound, chan_name);
    csoundSpinLock(lock);
    memcpy(fout, fin, sizeof(PVSDAT)-sizeof(AUXCH));
    //printf("fout=%p fout->frame.auxp=%p fin=%p fin->frame=%p\n",
    //       fout, fout->frame.auxp, fin, fin->frame);
    if(fin->frame != NULL)
        memcpy(fout->frame.auxp, fin->frame, sizeof(float)*(size+2));
    else memset(fout->frame.auxp, 0, sizeof(float)*(size+2));
    csoundSpinUnLock(lock);
    return OK;
}

int32_t pvsout_init(CSOUND *csound, FCHAN *p)
{
    PVSDAT *fin = p->r;
    MYFLT *pp;
    PVSDATEXT *f;
    int32_t     n = (int32_t)MYFLT2LRND(*(p->a));
    char name[16];

    snprintf(name, 16, "%i", n);
    if (csoundGetChannelPtr(csound, &pp, name,
                            CSOUND_PVS_CHANNEL | CSOUND_OUTPUT_CHANNEL)
        == CSOUND_SUCCESS){
        spin_lock_t  *lock = (spin_lock_t *)
                csoundGetChannelLock(csound, name);
        f = (PVSDATEXT *) pp;
        csoundSpinLock(lock);
        if(f->frame == NULL) {
            f->frame = mcalloc(csound, sizeof(float)*(fin->N+2));
        } else if(f->N < fin->N) {
            f->frame = mrealloc(csound, f->frame, sizeof(float)*(fin->N+2));
        }
        memcpy(f, fin, sizeof(PVSDAT)-sizeof(AUXCH));
        csoundSpinUnLock(lock);
    }
    return OK;
}


int32_t pvsout_perf(CSOUND *csound, FCHAN *p)
{

    PVSDAT *fin = p->r;
    int32_t     n = (int32_t)MYFLT2LRND(*(p->a));
    char chan_name[16];
    int32_t   err, size;
    PVSDATEXT *fout;
    MYFLT *pp;

    if (UNLIKELY(n < 0))
        return csoundPerfError(csound, &(p->h),Str("pvsout: invalid index"));

    snprintf(chan_name, 16, "%i", n);
    err = csoundGetChannelPtr(csound, &pp, chan_name,
                              CSOUND_PVS_CHANNEL | CSOUND_OUTPUT_CHANNEL);
    fout = (PVSDATEXT *) pp;
    if (UNLIKELY(err))
        return csoundPerfError(csound, &(p->h),
                                 Str("pvsout error %d:"
                                     "channel not found or not right type"), err);

    spin_lock_t  *lock = (spin_lock_t *) csoundGetChannelLock(csound, chan_name);
    csoundSpinLock(lock);
    size = fin->N < fout->N ? fin->N : fout->N;
    memcpy(fout, fin, sizeof(PVSDAT)-sizeof(AUXCH));
    if(fout->frame != NULL)
        memcpy(fout->frame, fin->frame.auxp, sizeof(float)*(size+2));
    csoundSpinUnLock(lock);
    return OK;
}
/* ======================================================================== */

/* "chn" opcodes and bus interface by Istvan Varga */

static int32_t delete_channel_db(CSOUND *csound, void *p)
{
    CONS_CELL *head, *values;
    IGN(p);
    if (csound->chn_db == NULL) {
        return 0;
    }

    head = values = cs_hash_table_values(csound, csound->chn_db);

    if (head != NULL) {
        while(values != NULL) {
            CHNENTRY* entry = values->value;

            if ((entry->type & CSOUND_CHANNEL_TYPE_MASK) != CSOUND_CONTROL_CHANNEL) {
                mfree(csound, entry->hints.attributes);
            }
            /* SY - 2014.07.14 - Don't free entry->data and rely on Csound memory db
               to free it; fixes issue with RTTI and chnexport of a global var, which
               maps to the CS_VAR_MEM's memblock, which is not what is allocated; other
               vars will be freed since they were Calloc'd */
            /*          mfree(csound, entry->data); */
            entry->datasize = 0;
            values = values->next;
        }
        cs_cons_free(csound, head);
    }

    cs_hash_table_mfree_complete(csound, csound->chn_db);
    csound->chn_db = NULL;
    return 0;
}

void set_channel_data_ptr(CSOUND *csound,
                          const char *name, void *ptr, int32_t newSize)
{
    find_channel(csound, name)->data = (MYFLT *) ptr;
    find_channel(csound, name)->datasize = newSize;
}

#define INIT_STRING_CHANNEL_DATASIZE 256

static CS_NOINLINE CHNENTRY *alloc_channel(CSOUND *csound,
                                           const char *name, int32_t type)
{
    CHNENTRY      *pp;
    int32_t           dsize = 0;
    switch (type & CSOUND_CHANNEL_TYPE_MASK) {
        case CSOUND_CONTROL_CHANNEL:
            dsize = sizeof(MYFLT);
            break;
        case CSOUND_AUDIO_CHANNEL:
            dsize = ((int32_t)sizeof(MYFLT) * csound->ksmps);
            break;
        case CSOUND_STRING_CHANNEL:
            dsize = (sizeof(STRINGDAT));
            break;
        case CSOUND_PVS_CHANNEL:
            dsize = (sizeof(PVSDATEXT));
            break;
    }
    pp = (CHNENTRY *) mcalloc(csound,
                                     (size_t) sizeof(CHNENTRY) + strlen(name) + 1);
    if (pp == NULL) return (CHNENTRY*) NULL;
    pp->data = (MYFLT *) mcalloc(csound, dsize);

    if ((type & CSOUND_CHANNEL_TYPE_MASK) == CSOUND_STRING_CHANNEL) {
        ((STRINGDAT*) pp->data)->size = 128;
        ((STRINGDAT*) pp->data)->data = mcalloc(csound, 128 * sizeof(char));
    }

    csoundSpinLockInit(&pp->lock);
    pp->datasize = dsize;

    return (CHNENTRY*) pp;
}

CS_NOINLINE int32_t create_new_channel(CSOUND *csound, const char *name,
                                              int32_t type)
{
    CHNENTRY      *pp;
    /* check for valid parameters and calculate hash value */
    if (UNLIKELY(!(type & 48)))
        return CSOUND_ERROR;

    /* create new empty database if not allocated */
    if (csound->chn_db == NULL) {
        csound->chn_db = cs_hash_table_create(csound);
        if (UNLIKELY(csoundRegisterResetCallback(csound, NULL,
                                                   delete_channel_db) != 0))
            return CSOUND_MEMORY;
        if (UNLIKELY(csound->chn_db == NULL))
            return CSOUND_MEMORY;
    }
    /* allocate new entry */
    pp = alloc_channel(csound, name, type);
    if (UNLIKELY(pp == NULL))
        return CSOUND_MEMORY;
    pp->hints.behav = 0;
    pp->type = type;
    strcpy(&(pp->name[0]), name);

    cs_hash_table_put(csound, csound->chn_db, (char*)name, pp);

    return CSOUND_SUCCESS;
}

int32_t bus_cmp_func(const void *p1, const void *p2)
{
    return strcmp(((controlChannelInfo_t*) p1)->name,
                  ((controlChannelInfo_t*) p2)->name);
}

/* ------------------------------------------------------------------------ */

/* perf time stub for printing "not initialised" error message */

int32_t notinit_opcode_stub(CSOUND *csound, void *p)
{
    return csoundPerfError(csound, &(((CHNGET *)p)->h),
                             Str("%s: not initialised"),
                             csoundGetOpcodeName(p));
}

/* print error message on failed channel query */

static CS_NOINLINE void print_chn_err_perf(void *p, int32_t err)
{
    CSOUND      *csound = ((OPDS*) p)->insdshead->csound;
    const char  *msg;

    if (((OPDS*) p)->opadr != (SUBR) NULL)
        ((OPDS*) p)->opadr = (SUBR) notinit_opcode_stub;
    if (err == CSOUND_MEMORY)
        msg = "memory allocation failure";
    else if (err < 0)
        msg = "invalid channel name";
    else
        msg = "channel already exists with incompatible type";
    csoundWarning(csound, "%s", Str(msg));
}

static CS_NOINLINE int32_t print_chn_err(void *p, int32_t err)
{
    CSOUND      *csound = ((OPDS*) p)->insdshead->csound;
    const char  *msg;

    if (((OPDS*) p)->opadr != (SUBR) NULL)
        ((OPDS*) p)->opadr = (SUBR) notinit_opcode_stub;
    if (err == CSOUND_MEMORY)
        msg = "memory allocation failure";
    else if (err < 0)
        msg = "invalid channel name";
    else
        msg = "channel already exists with incompatible type";
    return csoundInitError(csound, "%s", Str(msg));
}


/* receive control value from bus at performance time */
static int32_t chnget_opcode_perf_k(CSOUND* csound, CHNGET* p)
{
    if (strncmp(p->chname, p->iname->data, MAX_CHAN_NAME) || !strcmp(p->iname->data, ""))
    {
        int32_t err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                                          CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL);
        if (err==0){
            p->lock = (spin_lock_t*) csoundGetChannelLock(csound, (char*) p->iname->data);
            strNcpy(p->chname, p->iname->data, MAX_CHAN_NAME);
        }
        else {
            print_chn_err_perf(p, err);
            return OK;
        }
    }

#if defined(MSVC)
    volatile union {
    MYFLT d;
    MYFLT_INT_TYPE i;
    } x;
    x.i = InterlockedExchangeAdd64((MYFLT_INT_TYPE *) p->fp, 0);
    *(p->arg) = x.d;
#elif defined(HAVE_ATOMIC_BUILTIN)
    volatile union {
        MYFLT d;
        MYFLT_INT_TYPE i;
    } x;
    x.i = __atomic_load_n((MYFLT_INT_TYPE*) p->fp, __ATOMIC_SEQ_CST);
    *(p->arg) = x.d;
#else
    *(p->arg) = *(p->fp);
#endif
    return OK;
}

/* receive audio data from bus at performance time */
static int32_t chnget_opcode_perf_a(CSOUND* csound, CHNGET* p)
{
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early = p->h.insdshead->ksmps_no_end;

    if (strncmp(p->chname, p->iname->data, MAX_CHAN_NAME)  || !strcmp(p->iname->data, ""))
    {
        int32_t err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                                          CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL);
        if (err==0){
            p->lock = (spin_lock_t*) csoundGetChannelLock(csound, (char*) p->iname->data);
            strNcpy(p->chname, p->iname->data, MAX_CHAN_NAME);
        }
        else {
            print_chn_err_perf(p, err);
            return OK;
        }
    } 
      
    

    if (CS_KSMPS==(uint32_t) csound->ksmps){
        csoundSpinLock(p->lock);
        if (UNLIKELY(offset)) memset(p->arg, '\0', offset);
        memcpy(&p->arg[offset], p->fp, sizeof(MYFLT)*(CS_KSMPS-offset-early));
        if (UNLIKELY(early))
            memset(&p->arg[CS_KSMPS-early], '\0', sizeof(MYFLT)*early);
        csoundSpinUnLock(p->lock);
    }
    else {
        csoundSpinLock(p->lock);
        if (UNLIKELY(offset)) memset(p->arg, '\0', offset);
        memcpy(&p->arg[offset], &(p->fp[offset+p->pos]),
               sizeof(MYFLT)*(CS_KSMPS-offset-early));
        if (UNLIKELY(early))
            memset(&p->arg[CS_KSMPS-early], '\0', sizeof(MYFLT)*early);
        p->pos += CS_KSMPS;
        p->pos %= (csound->ksmps-offset);
        csoundSpinUnLock(p->lock);
    }

    return OK;
}

/* receive control value from bus at init time */
int32_t chnget_opcode_init_i(CSOUND *csound, CHNGET *p)
{
    int32_t   err;
    err = csoundGetChannelPtr(csound, &(p->fp), p->iname->data,
                              CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL);
    if (UNLIKELY(err))
        return print_chn_err(p, err);
#if defined(MSVC)
    {
    union {
        MYFLT d;
        MYFLT_INT_TYPE i;
    } x;
    x.i = InterlockedExchangeAdd64((MYFLT_INT_TYPE *)p->fp, 0);
    *(p->arg) = x.d;
    }
#elif defined(HAVE_ATOMIC_BUILTIN)
    {
        union {
            MYFLT d;
            MYFLT_INT_TYPE i;
        } x;
        x.i = __atomic_load_n((MYFLT_INT_TYPE *) p->fp, __ATOMIC_SEQ_CST);
        *(p->arg) =  x.d;
    }
#else
    *(p->arg) = *(p->fp);
#endif

    return OK;
}

/* init routine for i-rate chnget array opcode (control data) */

int32_t chnget_array_opcode_init_i(CSOUND* csound, CHNGETARRAY* p)
{
    int index = 0;
    ARRAYDAT* arr = (ARRAYDAT*) p->iname;

    p->arraySize = arr->sizes[0];
    p->channels = (STRINGDAT*) arr->data;

    tabinit(csound, p->arrayDat, p->arraySize);

    int32_t err;
    MYFLT* fp;

    for (index = 0; index<p->arraySize; index++)
    {
        err = csoundGetChannelPtr(csound, &(fp), p->channels[index].data,
                                  CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL);

        if (UNLIKELY(err))
            return print_chn_err(p, err);
#if defined(MSVC)
        {
        union {
            MYFLT d;
            MYFLT_INT_TYPE i;
        } x;
        x.i = InterlockedExchangeAdd64((MYFLT_INT_TYPE *)fp, 0);
        p->arrayDat->data[index] = x.d;
        }
#elif defined(HAVE_ATOMIC_BUILTIN)
        {
            union {
                MYFLT d;
                MYFLT_INT_TYPE i;
            } x;
            x.i = __atomic_load_n((MYFLT_INT_TYPE*) fp, __ATOMIC_SEQ_CST);
            p->arrayDat->data[index] = x.d;
        }
#else
        p->arrayDat->data[index] = *(fp);
#endif
    }
    return OK;
}

/* init routine for k, a and S chnget array opcodes */
int32_t chnget_array_opcode_init(CSOUND* csound, CHNGETARRAY* p)
{
    ARRAYDAT* arr = (ARRAYDAT*) p->iname;
    int index = 0;
    p->arraySize = arr->sizes[0];
    p->channels = (STRINGDAT*) arr->data;
    p->channelPtrs = (MYFLT **) mmalloc(csound, p->arraySize*sizeof(MYFLT*)); // VL: surely an array of pointers?
    tabinit(csound, p->arrayDat, p->arraySize);

    int32_t err;
    int32_t channelType;

    if (strcmp("k", p->arrayDat->arrayType->varTypeName) == 0)
        channelType = CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL;
    else if(strcmp("a", p->arrayDat->arrayType->varTypeName) == 0)
        channelType = CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL;
    else
        channelType = CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL;

    STRINGDAT *strings = (STRINGDAT *)p->arrayDat->data;
    for (index = 0; index<p->arraySize; index++)
    {
        if(strcmp(p->channels[index].data, "")) {
            err = csoundGetChannelPtr(csound, &p->channelPtrs[index], p->channels[index].data,
                                      channelType);

            if (LIKELY(!err)) {
                p->lock = (spin_lock_t *) csoundGetChannelLock(csound, p->channels[index].data);
                strNcpy(p->chname, p->channels[index].data, MAX_CHAN_NAME);

                if(channelType == (CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL)){
                    csoundSpinLock(p->lock);
                    strings[index].data = cs_strdup(csound, ((STRINGDAT *)p->channelPtrs[index])->data);
                    strings[index].size = strlen(((STRINGDAT *)p->channelPtrs[index])->data + 1);
                    csoundSpinUnLock(p->lock);
                }
            }
        }
        else{
            return csoundInitError(csound, "%s%s", Str("invalid channel name:"),
                    !strcmp(p->channels[index].data, "") ? Str("\"empty\"") : Str(p->channels[index].data));
        }
    }

    if(channelType == (CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL))
        p->h.opadr = (SUBR) chnget_array_opcode_perf_k;
    else if(channelType == (CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL))
        p->h.opadr = (SUBR) chnget_array_opcode_perf_a;
    else
        p->h.opadr = (SUBR) chnget_array_opcode_perf_S;

    return OK;
}

/* perf routine for chnget array opcode (control data) */

int32_t chnget_array_opcode_perf_k(CSOUND* csound, CHNGETARRAY* p)
{
    (void)(csound);
    int index = 0;

    for (index = 0; index<p->arraySize; index++)
    {
#if defined(MSVC)
        volatile union {
        MYFLT d;
        MYFLT_INT_TYPE i;
        } x;
        x.i = InterlockedExchangeAdd64((MYFLT_INT_TYPE *) p->channelPtrs[index], 0);
        p->arrayDat->data[index] = x.d;
#elif defined(HAVE_ATOMIC_BUILTIN)
        volatile union {
            MYFLT d;
            MYFLT_INT_TYPE i;
        } x;
        x.i = __atomic_load_n((MYFLT_INT_TYPE*) p->channelPtrs[index], __ATOMIC_SEQ_CST);
        p->arrayDat->data[index] = x.d;
//        csoundMessage(csound, Str("%f"), p->channelPtrs[index]);
#else
        p->arrayDat->data[index] = *(p->channelPtrs[index]);
#endif

    }
    return OK;
}

/* perf routine for chnget array opcode (audio data) */

int32_t chnget_array_opcode_perf_a(CSOUND *csound, CHNGETARRAY *p)
{
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;

    int index = 0;
    int blockIndex = 0;
    MYFLT* outPtr;
    for (index = 0; index<p->arraySize; index++) {
        blockIndex = csound->ksmps*index;

        if (CS_KSMPS == (uint32_t) csound->ksmps) {
            csoundSpinLock(p->lock);
            if (UNLIKELY(offset)) memset(&p->arrayDat->data[blockIndex], '\0', offset);
            memcpy(&p->arrayDat->data[blockIndex+offset], p->channelPtrs[index], sizeof(MYFLT) * (CS_KSMPS - offset - early));
            if (UNLIKELY(early))
                memset(&p->arrayDat->data[blockIndex+CS_KSMPS - early], '\0', sizeof(MYFLT) * early);
            csoundSpinUnLock(p->lock);
        } else {
            csoundSpinLock(p->lock);
            if (UNLIKELY(offset)) memset(&outPtr, '\0', offset);
            memcpy(&p->arrayDat->data[blockIndex+offset], &(p->channelPtrs[index][offset + p->pos]),
                   sizeof(MYFLT) * (CS_KSMPS - offset - early));
            if (UNLIKELY(early))
                memset(&p->arrayDat->data[blockIndex+CS_KSMPS - early], '\0', sizeof(MYFLT) * early);
            p->pos += CS_KSMPS;
            p->pos %= (csound->ksmps - offset);
            csoundSpinUnLock(p->lock);
        }
    }

    return OK;
}


int32_t chnget_array_opcode_perf_S(CSOUND* csound, CHNGETARRAY* p)
{
    STRINGDAT *strings = (STRINGDAT *)p->arrayDat->data;
    int32_t err;
    int index = 0;

    for (index = 0; index<p->arraySize; index++) {
        err = csoundGetChannelPtr(csound, &p->channelPtrs[index], p->channels[index].data,
                                  CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL);

        if (UNLIKELY(err))
            return print_chn_err(p, err);

        p->lock = (spin_lock_t *) csoundGetChannelLock(csound, (char *) p->channels[index].data);
        csoundSpinLock(p->lock);
        strings[index].data = cs_strdup(csound, ((STRINGDAT *)p->channelPtrs[index])->data);
        strings[index].size = strlen(((STRINGDAT *)p->channelPtrs[index])->data + 1);
        csoundSpinUnLock(p->lock);
    }

    return OK;
}

/* chnset array opcode init function for S arrays */

int32_t chnset_array_opcode_init_i(CSOUND *csound, CHNGETARRAY *p)
{
    int32_t   err;

    int index = 0;
    ARRAYDAT* valueArr = (ARRAYDAT*) p->arrayDat;
    ARRAYDAT* channelArr = (ARRAYDAT*) p->iname;
    p->arraySize = channelArr->sizes[0];
    p->channels = (STRINGDAT*) channelArr->data;
    p->channelPtrs = (MYFLT **) mmalloc(csound, p->arraySize*sizeof(MYFLT*)); // VL surely array of pointers?

    for (index = 0; index<p->arraySize; index++) {
        err = csoundGetChannelPtr(csound, &p->channelPtrs[index], (char *) p->channels[index].data,
                                  CSOUND_CONTROL_CHANNEL | CSOUND_OUTPUT_CHANNEL);
        if (UNLIKELY(err))
            return print_chn_err(p, err);

#if defined(MSVC)
        volatile union {
      MYFLT d;
      MYFLT_INT_TYPE i;
    } x;
    x.d = valueArr->data[index];
    InterlockedExchange64((MYFLT_INT_TYPE *) p->channelPtrs[index], x.i);
#elif defined(HAVE_ATOMIC_BUILTIN)
        union {
            MYFLT d;
            MYFLT_INT_TYPE i;
        } x;
        x.d = valueArr->data[index];
        __atomic_store_n((MYFLT_INT_TYPE *)(p->channelPtrs[index]),x.i, __ATOMIC_SEQ_CST);
#else
        {
      spin_lock_t *lock;       /* Need lock for the channel */
      p->lock = lock =  (spin_lock_t *)
        csoundGetChannelLock(csound, (char*) p->iname->data);
      csoundSpinLock(lock);
      *(p->channelPtrs[index]) =  valueArr->data[index];
      csoundSpinUnLock(lock);
    }
#endif
    }


    return OK;
}

/* init routine for chnset array opcodes - a, k and S */

int32_t chnset_array_opcode_init(CSOUND* csound, CHNGETARRAY* p)
{
    int32_t err;
    int index = 0;

    ARRAYDAT* channelArr = (ARRAYDAT*) p->iname;
    p->arraySize = channelArr->sizes[0];
    p->channels = (STRINGDAT*) channelArr->data;
    p->channelPtrs = mmalloc(csound, p->arraySize*sizeof(MYFLT*)); // surely an array of pointers?

    int32_t channelType;

    if (strcmp("k", p->arrayDat->arrayType->varTypeName) == 0)
        channelType = CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL;
    else if(strcmp("a", p->arrayDat->arrayType->varTypeName) == 0)
        channelType = CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL;
    else
        channelType = CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL;

    for (index = 0; index<p->arraySize; index++) {
        err = csoundGetChannelPtr(csound, &(p->channelPtrs[index]), (char *) p->channels[index].data,
                                  channelType);
        if (LIKELY(!err)) {
            p->lock = (spin_lock_t *) csoundGetChannelLock(csound, (char *) p->channels[index].data);
            strNcpy(p->chname, p->channels[index].data, MAX_CHAN_NAME);
        }
    }

    if(channelType == (CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL))
        p->h.opadr = (SUBR) chnset_array_opcode_perf_k;
    else if(channelType == (CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL))
        p->h.opadr = (SUBR) chnset_array_opcode_perf_a;
    else
        p->h.opadr = (SUBR) chnset_array_opcode_perf_S;

    return OK;
}

/* send control value to bus at performance time */

int32_t chnset_array_opcode_perf_k(CSOUND *csound, CHNGETARRAY *p)
{
    int index = 0;
    ARRAYDAT* valueArr = (ARRAYDAT*) p->arrayDat;

    for (index = 0; index<p->arraySize; index++) {
        if (strncmp(p->chname, p->channels[index].data, MAX_CHAN_NAME)) {
            int32_t err = csoundGetChannelPtr(csound, &(p->channelPtrs[index]), (char *) p->channels[index].data,
                                              CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL);
            if (err == 0) {
                p->lock = (spin_lock_t *) csoundGetChannelLock(csound, (char *) p->channels[index].data);
            } else
                print_chn_err_perf(p, err);
        }

#if defined(MSVC)
        volatile union {
          MYFLT d;
          MYFLT_INT_TYPE i;
        } x;
        x.d = valueArr->data[index];
        InterlockedExchange64((MYFLT_INT_TYPE *) p->channelPtrs[index], x.i);
#elif defined(HAVE_ATOMIC_BUILTIN)
        union {
            MYFLT d;
            MYFLT_INT_TYPE i;
        } x;
        x.d = valueArr->data[index];
        __atomic_store_n((MYFLT_INT_TYPE *) (p->channelPtrs[index]), x.i, __ATOMIC_SEQ_CST);
#else
        csoundSpinLock(p->lock);
        *(p->channelPtrs[index]) = valueArr->data[index];
        csoundSpinUnLock(p->lock);
#endif
    }
    return OK;
}

/* perf routine for chnset array opcode (audio data) */

int32_t chnset_array_opcode_perf_a(CSOUND *csound, CHNGETARRAY *p)
{
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;
    ARRAYDAT* valueArr = (ARRAYDAT*) p->arrayDat;
    int index = 0;
    int blockIndex = 0;

    for (index = 0; index<p->arraySize; index++) {
        if(CS_KSMPS == (uint32_t) csound->ksmps){
            blockIndex = csound->ksmps*index;
            /* Need lock for the channel */
            csoundSpinLock(p->lock);
            if (UNLIKELY(offset)) memset(p->channelPtrs[index], '\0', sizeof(MYFLT)*offset);
            memcpy(&p->channelPtrs[index][offset], &valueArr->data[blockIndex+offset],
                   sizeof(MYFLT)*(CS_KSMPS-offset-early));
            if (UNLIKELY(early))
                memset(&p->channelPtrs[index][early], '\0', sizeof(MYFLT)*(CS_KSMPS-early));
            csoundSpinUnLock(p->lock);
        } else {
            /* Need lock for the channel */
            csoundSpinLock(p->lock);
            if (UNLIKELY(offset)) memset(p->channelPtrs[index], '\0', sizeof(MYFLT)*offset);
            memcpy(&p->channelPtrs[index][offset+p->pos], &valueArr->data[blockIndex+offset],
                   sizeof(MYFLT)*(CS_KSMPS-offset-early));
            if (UNLIKELY(early))
                memset(&p->channelPtrs[index][early], '\0', sizeof(MYFLT)*(CS_KSMPS-early));
            p->pos += CS_KSMPS;
            p->pos %= (csound->ksmps-offset);
            csoundSpinUnLock(p->lock);
        }
    }

    return OK;
}

int32_t chnset_array_opcode_perf_S(CSOUND* csound, CHNGETARRAY* p)
{
    int32_t err;
    int index = 0;
    STRINGDAT *strings = (STRINGDAT *)p->arrayDat->data;

    for (index = 0; index<p->arraySize; index++)
    {
        if(strcmp(p->channels[index].data, "")) {
            err = csoundGetChannelPtr(csound, &p->channelPtrs[index], (char *) p->channels[index].data,
                                      CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL);

            if (UNLIKELY(err))
                return print_chn_err(p, err);

            p->lock = (spin_lock_t *) csoundGetChannelLock(csound, (char *) p->channels[index].data);
            csoundSpinLock(p->lock);
            ((STRINGDAT *)p->channelPtrs[index])->data = cs_strdup(csound, strings[index].data);
            ((STRINGDAT *)p->channelPtrs[index])->size = strlen(strings[index].data + 1);
            csoundSpinUnLock(p->lock);
        }
    }

    return OK;
}

/* init routine for chnget opcode (control data) */

int32_t chnget_opcode_init_k(CSOUND *csound, CHNGET *p)
{
    int32_t   err;
    err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                              CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL);

    if (LIKELY(!err)) {
        p->lock =   (spin_lock_t *)csoundGetChannelLock(csound, (char*) p->iname->data);
        strNcpy(p->chname, p->iname->data, MAX_CHAN_NAME);
    }

    p->h.opadr = (SUBR) chnget_opcode_perf_k;
    return OK;
}

/* init routine for chnget opcode (audio data) */

int32_t chnget_opcode_init_a(CSOUND* csound, CHNGET* p)
{
    int32_t err;
    p->pos = 0;
    err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                              CSOUND_AUDIO_CHANNEL | CSOUND_INPUT_CHANNEL);

    if (LIKELY(!err))
    {
        p->lock = (spin_lock_t*) csoundGetChannelLock(csound, (char*) p->iname->data);
        strNcpy(p->chname, p->iname->data, MAX_CHAN_NAME);
    }

    p->h.opadr = (SUBR) chnget_opcode_perf_a;
    return OK;
}

/* receive string value from bus at init time */

int32_t chnget_opcode_init_S(CSOUND* csound, CHNGET* p)
{
    int32_t err;
    char* s = ((STRINGDAT*) p->arg)->data;
    err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                              CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL);
    p->lock = (spin_lock_t*) csoundGetChannelLock(csound, (char*) p->iname->data);

    if (LIKELY(!err))
    {
        csoundSpinLock(p->lock);
        if (((STRINGDAT*) p->fp)->data!=NULL)
        {
            if (((STRINGDAT*) p->fp)->size>((STRINGDAT*) p->arg)->size)
            {
                if (s!=NULL) mfree(csound, s);
                s = cs_strdup(csound, ((STRINGDAT*) p->fp)->data);
                ((STRINGDAT*) p->arg)->data = s;
                ((STRINGDAT*) p->arg)->size = strlen(s)+1;
            }
            else strcpy(((STRINGDAT*) p->arg)->data, ((STRINGDAT*) p->fp)->data);
        }
        csoundSpinUnLock(p->lock);
    }
    return OK;
}


int32_t chnget_opcode_perf_S(CSOUND* csound, CHNGET* p)
{
    int32_t err;
    char* s = ((STRINGDAT*) p->arg)->data;
    err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                              CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL);
    p->lock = (spin_lock_t*) csoundGetChannelLock(csound, (char*) p->iname->data);

    if (UNLIKELY(err))
        return print_chn_err(p, err);

    if (s!=NULL && ((STRINGDAT*) p->fp)->data!=NULL &&
        strcmp(s, ((STRINGDAT*) p->fp)->data)==0)
        return OK;

    csoundSpinLock(p->lock);

    if (((STRINGDAT*) p->fp)->data!=NULL)
    {
        if (((STRINGDAT*) p->arg)->size<=((STRINGDAT*) p->fp)->size)
        {
            if (s!=NULL) mfree(csound, s);
            s = cs_strdup(csound, ((STRINGDAT*) p->fp)->data);
            ((STRINGDAT*) p->arg)->data = s;
            ((STRINGDAT*) p->arg)->size = strlen(s)+1;
        }
        else strcpy(((STRINGDAT*) p->arg)->data, ((STRINGDAT*) p->fp)->data);
    }
    csoundSpinUnLock(p->lock);
    return OK;
}


/* send control value to bus at performance time */

static int32_t chnset_opcode_perf_k(CSOUND *csound, CHNGET *p)
{
    if(strncmp(p->chname, p->iname->data, MAX_CHAN_NAME)){
        int32_t err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                                          CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL);
        if(err == 0) {
            p->lock = (spin_lock_t *) csoundGetChannelLock(csound, (char*) p->iname->data);
            strNcpy(p->chname, p->iname->data, MAX_CHAN_NAME);
        }
        else
            print_chn_err_perf(p, err);
    } // else return csoundPerfError(csound, &p->h, "invalid channel name");

#if defined(MSVC)
    volatile union {
      MYFLT d;
      MYFLT_INT_TYPE i;
    } x;
    x.d = *(p->arg);
    InterlockedExchange64((MYFLT_INT_TYPE *) p->fp, x.i);
#elif defined(HAVE_ATOMIC_BUILTIN)
    union {
        MYFLT d;
        MYFLT_INT_TYPE i;
    } x;
    x.d = *(p->arg);
    __atomic_store_n((MYFLT_INT_TYPE *)(p->fp),x.i, __ATOMIC_SEQ_CST);
#else
    csoundSpinLock(p->lock);
    *(p->fp) = *(p->arg);
    csoundSpinUnLock(p->lock);
#endif
    return OK;
}

/* send audio data to bus at performance time */

static int32_t chnset_opcode_perf_a(CSOUND *csound, CHNGET *p)
{
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;
    if(CS_KSMPS == (uint32_t) csound->ksmps){
        /* Need lock for the channel */
        csoundSpinLock(p->lock);
        if (UNLIKELY(offset)) memset(p->fp, '\0', sizeof(MYFLT)*offset);
        memcpy(&p->fp[offset], &p->arg[offset],
               sizeof(MYFLT)*(CS_KSMPS-offset-early));
        if (UNLIKELY(early))
            memset(&p->fp[early], '\0', sizeof(MYFLT)*(CS_KSMPS-early));
        csoundSpinUnLock(p->lock);
    } else {
        /* Need lock for the channel */
        csoundSpinLock(p->lock);
        if (UNLIKELY(offset)) memset(p->fp, '\0', sizeof(MYFLT)*offset);
        memcpy(&p->fp[offset+p->pos], &p->arg[offset],
               sizeof(MYFLT)*(CS_KSMPS-offset-early));
        if (UNLIKELY(early))
            memset(&p->fp[early], '\0', sizeof(MYFLT)*(CS_KSMPS-early));
        p->pos += CS_KSMPS;
        p->pos %= (csound->ksmps-offset);
        csoundSpinUnLock(p->lock);
    }
    return OK;
}

/* send audio data to bus at performance time, mixing to previous output */

static int32_t chnmix_opcode_perf(CSOUND *csound, CHNGET *p)
{
    uint32_t n = 0;
    IGN(csound);
    uint32_t nsmps = CS_KSMPS;
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;
    if (UNLIKELY(early)) nsmps -= early;
    /* Need lock for the channel */
    csoundSpinLock(p->lock);
    for (n=offset; n<nsmps; n++) {
        p->fp[n] += p->arg[n];
    }
    csoundSpinUnLock(p->lock);
    return OK;
}

/* clear an audio channel to zero at performance time */

static int32_t chnclear_opcode_perf(CSOUND *csound, CHNCLEAR *p)
{
    int32_t i, n=p->INCOUNT;
    /* Need lock for the channel */
    IGN(csound);
    for (i=0; i<n; i++) {
        csoundSpinLock(p->lock[i]);
        memset(p->fp[i], 0, CS_KSMPS*sizeof(MYFLT)); /* Should this leave start? */
        csoundSpinUnLock(p->lock[i]);
    }
    return OK;
}

/* send control value to bus at init time */

int32_t chnset_opcode_init_i(CSOUND *csound, CHNGET *p)
{
    int32_t   err;

    err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                              CSOUND_CONTROL_CHANNEL | CSOUND_OUTPUT_CHANNEL);
    if (UNLIKELY(err))
        return print_chn_err(p, err);


#if defined(MSVC)
    volatile union {
      MYFLT d;
      MYFLT_INT_TYPE i;
    } x;
    x.d = *(p->arg);
    InterlockedExchange64((MYFLT_INT_TYPE *) p->fp, x.i);
#elif defined(HAVE_ATOMIC_BUILTIN)
    union {
        MYFLT d;
        MYFLT_INT_TYPE i;
    } x;
    x.d = *(p->arg);
    __atomic_store_n((MYFLT_INT_TYPE *)(p->fp),x.i, __ATOMIC_SEQ_CST);
#else
    {
      spin_lock_t *lock;       /* Need lock for the channel */
      p->lock = lock =  (spin_lock_t *)
        csoundGetChannelLock(csound, (char*) p->iname->data);
      csoundSpinLock(lock);
      *(p->fp) = *(p->arg);
      csoundSpinUnLock(lock);
    }
#endif
    return OK;
}

/* init routine for chnset opcode (control data) */

int32_t chnset_opcode_init_k(CSOUND* csound, CHNGET* p)
{
    int32_t err;

    err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                              CSOUND_CONTROL_CHANNEL | CSOUND_OUTPUT_CHANNEL);
    if (LIKELY(!err)) {
        p->lock = (spin_lock_t*) csoundGetChannelLock(csound, (char*) p->iname->data);
    } else return print_chn_err(p, err);

    p->h.opadr = (SUBR) chnset_opcode_perf_k;
    return OK;
}

/* init routine for chnset opcode (audio data) */

int32_t chnset_opcode_init_a(CSOUND* csound, CHNGET* p)
{
    int32_t err;
    p->pos = 0;
    err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                              CSOUND_AUDIO_CHANNEL | CSOUND_OUTPUT_CHANNEL);
    if (!err) {
        p->lock = (spin_lock_t*) csoundGetChannelLock(csound, (char*) p->iname->data);
    } else return print_chn_err(p, err);

    p->h.opadr = (SUBR) chnset_opcode_perf_a;
    return OK;
}

/* init routine for chnmix opcode */

int32_t chnmix_opcode_init(CSOUND *csound, CHNGET *p)
{
    int32_t   err;

    err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                              CSOUND_AUDIO_CHANNEL | CSOUND_OUTPUT_CHANNEL);
    if (LIKELY(!err)) {
        p->lock = (spin_lock_t *)csoundGetChannelLock(csound, (char*) p->iname->data);
        p->h.opadr = (SUBR) chnmix_opcode_perf;
        return OK;
    }
    return print_chn_err(p, err);
}

/* init routine for chnclear opcode */

int32_t chnclear_opcode_init(CSOUND *csound, CHNCLEAR *p)
{
    int32_t   err;
    int32_t   i, n = (int32_t)p->INCOUNT;
    for (i=0; i<n; i++) {
        /* NOTE: p->imode is a pointer to the channel data here */
        err = csoundGetChannelPtr(csound, &(p->fp[i]), (char*) p->iname[i]->data,
                                  CSOUND_AUDIO_CHANNEL | CSOUND_OUTPUT_CHANNEL);
        if (LIKELY(!err)) {
            p->lock[i] = (spin_lock_t *)csoundGetChannelLock(csound,
                                                             (char*) p->iname[i]->data);
        }
        else return print_chn_err(p, err);
    }
    p->h.opadr = (SUBR) chnclear_opcode_perf;
    return OK;
}

/* send string to bus at init time */

int32_t chnset_opcode_init_S(CSOUND* csound, CHNGET* p)
{
    int32_t err;
    spin_lock_t* lock;
    char* s = ((STRINGDAT*) p->arg)->data;

    err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                              CSOUND_STRING_CHANNEL | CSOUND_OUTPUT_CHANNEL);
    // size = csoundGetChannelDatasize(csound, p->iname->data);
    if (LIKELY(!err))
    {

        if (s==NULL) return NOTOK;
        p->lock = lock = (spin_lock_t*)
                csoundGetChannelLock(csound, (char*) p->iname->data);
        csoundSpinLock(lock);
        if (strlen(s)>=(uint32_t) ((STRINGDAT*) p->fp)->size)
        {
            if (((STRINGDAT*) p->fp)->data!=NULL)
                mfree(csound, ((STRINGDAT*) p->fp)->data);
            ((STRINGDAT*) p->fp)->data = cs_strdup(csound, s);
            ((STRINGDAT*) p->fp)->size = strlen(s)+1;

            //set_channel_data_ptr(csound, p->iname->data,p->fp, strlen(s)+1);
        }
        else if (((STRINGDAT*) p->fp)->data!=NULL)
            strcpy(((STRINGDAT*) p->fp)->data, s);
        csoundSpinUnLock(lock);
    } else return print_chn_err(p, err);

    return OK;
}

int32_t chnset_opcode_perf_S(CSOUND* csound, CHNGET* p)
{
    int32_t err;
    spin_lock_t* lock;
    char* s = ((STRINGDAT*) p->arg)->data;

    if ((err = csoundGetChannelPtr(csound, &(p->fp), (char*) p->iname->data,
                                   CSOUND_STRING_CHANNEL | CSOUND_OUTPUT_CHANNEL)))
        return err;
    // size = csoundGetChannelDatasize(csound, p->iname->data);

    if (s==NULL) return NOTOK;
    if (((STRINGDAT*) p->fp)->data
        && strcmp(s, ((STRINGDAT*) p->fp)->data)==0)
        return OK;

    p->lock = lock = (spin_lock_t*)
            csoundGetChannelLock(csound, (char*) p->iname->data);
    csoundSpinLock(lock);
    if (strlen(s)>=(uint32_t) ((STRINGDAT*) p->fp)->size)
    {
        if (((STRINGDAT*) p->fp)->data!=NULL)
            mfree(csound, ((STRINGDAT*) p->fp)->data);
        ((STRINGDAT*) p->fp)->data = cs_strdup(csound, s);
        ((STRINGDAT*) p->fp)->size = strlen(s)+1;
        //set_channel_data_ptr(csound, p->iname->data,p->fp, strlen(s)+1);
        //printf("p: %s: %s \n", p->iname->data, ((STRINGDAT *)p->fp)->data);
    }
    else if (((STRINGDAT*) p->fp)->data!=NULL)
        strcpy(((STRINGDAT*) p->fp)->data, s);
    csoundSpinUnLock(lock);
    //printf("%s \n", (char *)p->fp);
    return OK;
}

/* declare control channel, optionally with special parameters */

int32_t chn_k_opcode_init_(CSOUND *csound, CHN_OPCODE_K *p, int mode)
{
    MYFLT *dummy;
    int32_t   type, err;
    controlChannelHints_t hints;
    hints.attributes = NULL;
    hints.max = hints.min = hints.dflt = FL(0.0);
    hints.x = hints.y = hints.height = hints.width = 0;

    // mode = (int32_t)MYFLT2LRND(*(p->imode));
    if (UNLIKELY(mode < 1 || mode > 3))
        return csoundInitError(csound, Str("invalid mode parameter"));
    type = CSOUND_CONTROL_CHANNEL;
    if (mode & 1)
        type |= CSOUND_INPUT_CHANNEL;
    if (mode & 2)
        type |= CSOUND_OUTPUT_CHANNEL;

    err = csoundGetChannelPtr(csound, &dummy, (char*) p->iname->data, type);
    if (err)
        return print_chn_err(p, err);
    hints.behav = CSOUND_CONTROL_CHANNEL_NO_HINTS;
    if ((int)MYFLT2LRND(*(p->itype)) == 1)
        hints.behav = CSOUND_CONTROL_CHANNEL_INT;
    else if ((int32_t)MYFLT2LRND(*(p->itype)) == 2)
        hints.behav |= CSOUND_CONTROL_CHANNEL_LIN;
    else if ((int32_t)MYFLT2LRND(*(p->itype)) == 3)
        hints.behav |= CSOUND_CONTROL_CHANNEL_EXP;
    if ((int32_t)MYFLT2LRND(*(p->itype)) != 0) {
        hints.attributes = 0;
        if (p->INOCOUNT > 10) {
            hints.attributes = p->Sattributes->data;
        }
        hints.dflt = *(p->idflt);
        hints.min = *(p->imin);
        hints.max = *(p->imax);
        hints.x = *(p->ix);
        hints.y = *(p->iy);
        hints.width = *(p->iwidth);
        hints.height = *(p->iheight);
    }
    err = csoundSetControlChannelHints(csound, (char*) p->iname->data, hints);
    if (LIKELY(!err)) {
        p->lock = (spin_lock_t *)csoundGetChannelLock(csound, (char*) p->iname->data);
        return OK;
    }
    if (err == CSOUND_MEMORY)
        return print_chn_err(p, err);
    return csoundInitError(csound, Str("invalid channel parameters"));
}

int32_t chn_k_opcode_init(CSOUND *csound, CHN_OPCODE_K *p)
{
    int32_t mode = (int32_t)MYFLT2LRND(*(p->imode));
    return chn_k_opcode_init_(csound, p, mode);
}

int32_t chn_k_opcode_init_S(CSOUND *csound, CHN_OPCODE_K *p)
{
    STRINGDAT *smode = (STRINGDAT *)p->imode;
    int32_t mode;
    if(!strcmp("rw", smode->data))
        mode = 3;
    else if(!strcmp("r", smode->data))
        mode = 1;
    else if(!strcmp("w", smode->data))
        mode = 2;
    else
        return csoundInitError(csound, Str("invalid mode, should be r, w, rw"));
    return chn_k_opcode_init_(csound, p, mode);
}


/* declare audio channel */

int32_t chn_a_opcode_init(CSOUND *csound, CHN_OPCODE *p)
{
    MYFLT *dummy;
    int32_t   type, mode, err;

    mode = (int32_t)MYFLT2LRND(*(p->imode));
    if (UNLIKELY(mode < 1 || mode > 3))
        return csoundInitError(csound, Str("invalid mode parameter"));
    type = CSOUND_AUDIO_CHANNEL;
    if (mode & 1)
        type |= CSOUND_INPUT_CHANNEL;
    if (mode & 2)
        type |= CSOUND_OUTPUT_CHANNEL;
    err = csoundGetChannelPtr(csound, &dummy, (char*) p->iname->data, type);
    if (UNLIKELY(err))
        return print_chn_err(p, err);
    return OK;
}

/* declare string channel */

int32_t chn_S_opcode_init(CSOUND *csound, CHN_OPCODE *p)
{
    MYFLT *dummy;
    int32_t   type, mode, err;

    mode = (int32_t)MYFLT2LRND(*(p->imode));
    if (UNLIKELY(mode < 1 || mode > 3))
        return csoundInitError(csound, Str("invalid mode parameter"));
    type = CSOUND_STRING_CHANNEL;
    if (mode & 1)
        type |= CSOUND_INPUT_CHANNEL;
    if (mode & 2)
        type |= CSOUND_OUTPUT_CHANNEL;
    err = csoundGetChannelPtr(csound, &dummy, (char*) p->iname->data, type);
    if (UNLIKELY(err))
        return print_chn_err(p, err);
    p->lock = (spin_lock_t *) csoundGetChannelLock(csound, (char*) p->iname->data);
    return OK;
}

/* export new channel from global orchestra variable */

int32_t chnexport_opcode_init(CSOUND *csound, CHNEXPORT_OPCODE *p)
{
    MYFLT       *dummy;
    const char  *argName;
    int32_t         type = CSOUND_CONTROL_CHANNEL, mode, err;
    controlChannelHints_t hints;
    CHNENTRY *chn;

    /* must have an output argument of type 'gi', 'gk', 'ga', or 'gS' */
    if (UNLIKELY(csoundGetOutputArgCnt(p) != 1))
        goto arg_err;
    argName = csoundGetOutputArgName(p, 0);
    if (UNLIKELY(argName == NULL))
        goto arg_err;
    if (UNLIKELY(argName[0] != 'g'))
        goto arg_err;
    switch ((int32_t)argName[1]) {
        case 'i':
        case 'k':
            break;
        case 'a':
            type = CSOUND_AUDIO_CHANNEL;
            break;
        case 'S':
            type = CSOUND_STRING_CHANNEL;
            break;
        default:
            goto arg_err;
    }
    /* mode (input and/or output) */
    mode = (int32_t)MYFLT2LRND(*(p->imode));
    if (UNLIKELY(mode < 1 || mode > 3))
        return csoundInitError(csound, Str("invalid mode parameter"));
    if (mode & 1)
        type |= CSOUND_INPUT_CHANNEL;
    if (mode & 2)
        type |= CSOUND_OUTPUT_CHANNEL;
    /* check if the channel already exists (it should not) */
    err = csoundGetChannelPtr(csound, &dummy, (char*) p->iname->data, 0);
    if (UNLIKELY(err >= 0))
        return csoundInitError(csound, Str("channel already exists"));
    /* now create new channel, using output variable for data storage */
//    dummy = p->arg;
    /* THIS NEEDS A LOCK BUT DOES NOT EXIST YET */
    /* lock = csoundGetChannelLock(csound, (char*) p->iname->data); */
    /* csoundSpinLock(lock); */
    err = create_new_channel(csound, (char*) p->iname->data, type);

    /* csoundSpinLock(lock); */
    if (err)
        return print_chn_err(p, err);

    /* Now we need to find the channel entry */
    chn = find_channel(csound, (char*) p->iname->data);
    /* free the allocated memory (we will not use it) */
    mfree(csound, chn->data);
    /* point to the arg var */
    chn->data = p->arg;

    /* if control channel, set additional parameters */
    if ((type & CSOUND_CHANNEL_TYPE_MASK) != CSOUND_CONTROL_CHANNEL)
        return OK;
    // ***FIXME not used type = (int32_t)MYFLT2LRND(*(p->itype));
    hints.behav = CSOUND_CONTROL_CHANNEL_LIN;
    hints.dflt = *(p->idflt);
    hints.min = *(p->imin);
    hints.max = *(p->imax);
    hints.x = hints.y = hints.width = hints.height = 0;
    hints.attributes = NULL;
    err = csoundSetControlChannelHints(csound, (char*) p->iname->data, hints);
    if (LIKELY(!err))
        return OK;
    if (err == CSOUND_MEMORY)
        return print_chn_err(p, err);
    return csoundInitError(csound, Str("invalid channel parameters"));

    arg_err:
    return csoundInitError(csound, Str("invalid export variable"));
}

/* returns all parameters of a channel */

int32_t chnparams_opcode_init(CSOUND *csound, CHNPARAMS_OPCODE *p)
{
    MYFLT *dummy;
    int32_t   err;

    /* all values default to zero... */
    *(p->itype)    = FL(0.0);
    *(p->imode)    = FL(0.0);
    *(p->ictltype) = FL(0.0);
    *(p->idflt)    = FL(0.0);
    *(p->imin)     = FL(0.0);
    *(p->imax)     = FL(0.0);
    err = csoundGetChannelPtr(csound, &dummy, (char*) p->iname->data, 0);
    /* ...if channel does not exist */
    if (err <= 0)
        return OK;
    /* type (control/audio/string) */
    *(p->itype) = (MYFLT) (err & 15);
    /* mode (input and/or output) */
    *(p->imode) = (MYFLT) ((err & 48) >> 4);
    /* check for control channel parameters */
    if ((err & 15) == CSOUND_CONTROL_CHANNEL) {
        controlChannelHints_t hints;
        err = csoundGetControlChannelHints(csound, (char*) p->iname->data, &hints);
        if (UNLIKELY(err > 0))
            *(p->ictltype) = (MYFLT) err;
        *(p->ictltype) = hints.behav;
        *(p->idflt) = hints.dflt;
        *(p->imin) = hints.min;
        *(p->imax) = hints.max;
    }
    return OK;
}

/* ********************************************************************** */
/* *************** SENSING ********************************************** */
/* ********************************************************************** */

int32_t sensekey_perf(CSOUND *csound, KSENSE *p)
{
    int32_t     keyCode = 0;
    int32_t     retval;

    retval = csound->doCsoundCallback(csound, &keyCode,
                                      (p->keyDown != NULL ?
                                       CSOUND_CALLBACK_KBD_EVENT
                                                          : CSOUND_CALLBACK_KBD_TEXT));
    if (retval > 0) {
        if (!p->evtbuf) {
#if defined(__unix) || defined(__unix__) || defined(__MACH__)
            if (csound->inChar_ < 0) {
#  if defined(_WIN32)
                setvbuf(stdin, NULL, _IONBF, 0);  /* Does not seem to work */
#  elif defined(HAVE_TERMIOS_H)
                struct termios  tty;
                tcgetattr(0, &tty);
                tty.c_lflag &= (~ICANON);
                tcsetattr(0, TCSANOW, &tty);
#  endif
            }
#endif
            p->evtbuf = -1;
        }
        if (csound->inChar_ < 0) {
#if defined(__unix) || defined(__unix__) || defined(__MACH__)
            fd_set  rfds;
            struct timeval  tv;
            /* Watch stdin (fd 0) to see when it has input. */
            FD_ZERO(&rfds);
            FD_SET(0, &rfds);
            /* No waiting */
            tv.tv_sec = 0;
            tv.tv_usec = 0;

            retval = select(1, &rfds, NULL, NULL, &tv);

            if (retval>0) {
                char    ch = '\0';
                int32_t n=0;
                if (UNLIKELY((n=read(0, &ch, 1))<0)) {
                    csoundPerfError(csound, &(p->h),
                                      Str("read failure in sensekey\n"));
                    return NOTOK;
                }
                //if n==0 then EOF which we treat as empty
                if(n==0) ch = '\0';
                keyCode = (int32_t)((unsigned char) ch);
                /* FD_ISSET(0, &rfds) will be true. */
            }
            else if (retval<0) perror(Str("sensekey error:"));
#else
            unsigned char ch = (unsigned char) '\0';
#  ifdef _WIN32
        if (_kbhit())
          ch = (unsigned char) _getch();
#  else
        ch = (unsigned char) getchar();
#  endif
        keyCode = (int32_t)ch;
#endif
        }
        else if (csound->inChar_ > 0) {
            keyCode = csound->inChar_;
            csound->inChar_ = 0;
        }
        if (p->evtbuf != -1) {
            int32_t     tmp = keyCode;
            keyCode = p->evtbuf;
            tmp = (keyCode < 0 ? tmp : (-1 - keyCode));
            p->evtbuf = (tmp != 0 ? tmp : -1);
        }
        // *** Cannot see point of next 2 lines *** JPff
        /* else if (p->OUTOCOUNT>1 && p->keyDown != NULL) */
        /*   p->evtbuf = -1 - keyCode; */
        if (keyCode < 0)
            keyCode = 65535 - keyCode;
    }
    else if (retval < 0) {
        keyCode = 0;
    }
    *(p->ans) = (MYFLT) ((keyCode & (int32_t)0xFFFF) ?
                         (keyCode & (int32_t)0xFFFF) : -1);
    if (p->OUTOCOUNT>1 && p->keyDown != NULL)
        *(p->keyDown) = (MYFLT) ((keyCode > 0 && keyCode < 65536) ? 1 : 0);

    return OK;
}


/* k-rate and string i/o opcodes */
/* invalue and outvalue are used with the csoundAPI */
/*     ma++ ingalls      matt@sonomatics.com */

int32_t kinval(CSOUND *csound, INVAL *p)
{
    if (csound->InputChannelCallback_) {
        csound->InputChannelCallback_(csound,
                                      (char*) p->channelName.auxp,
                                      p->value, p->channelType);
    }
    else
        *(p->value) = FL(0.0);

    return OK;
}

int32_t kinvalS(CSOUND *csound, INVAL *p)
{

    if (csound->InputChannelCallback_) {
        csound->InputChannelCallback_(csound,
                                      (char*) p->channelName.auxp,
                                      ((STRINGDAT *)p->value)->data,
                                      p->channelType);

    }
    else {
        ((STRINGDAT *)p->value)->data[0]  = '\0';
    }

    return OK;
}


int32_t invalset_string_S(CSOUND *csound, INVAL *p)
{
    int32_t   err;
    int32_t type;
    STRINGDAT *out = (STRINGDAT *) p->value;

    const char  *s = ((STRINGDAT *)p->valID)->data;
    csoundAuxAlloc(csound, strlen(s) + 1, &p->channelName);
    strcpy((char*) p->channelName.auxp, s);

    p->channelType = &CS_VAR_TYPE_S;
    type = CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL;

    err = csoundGetChannelPtr(csound, (MYFLT **) &(p->channelptr),
                              (char*) p->channelName.auxp,
                              type);
    if (UNLIKELY(err))
        return print_chn_err(p, err);

    if (out->data == NULL || out->size < 256) {
        if(out->data != NULL)
            mfree(csound, out->data);
        out->data = mcalloc(csound, 256);
        out->size = 256;
    }

    /* grab input now for use during i-pass */
    kinvalS(csound, p);
    if (!csound->InputChannelCallback_) {
        csoundWarning(csound,Str("InputChannelCallback not set."));
    }
    return OK;
}

int32_t invalset_S(CSOUND *csound, INVAL *p)
{
    int32_t   err;
    int32_t type;

    const char  *s = ((STRINGDAT *)p->valID)->data;
    csoundAuxAlloc(csound, strlen(s) + 1, &p->channelName);
    strcpy((char*) p->channelName.auxp, s);

    p->channelType = &CS_VAR_TYPE_K;
    type = CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL;

    err = csoundGetChannelPtr(csound, (MYFLT **) &(p->channelptr),
                              (char*) p->channelName.auxp,
                              type);
    if (UNLIKELY(err))
        return print_chn_err(p, err);

    /* grab input now for use during i-pass */
    kinval(csound, p);
    if (!csound->InputChannelCallback_) {
        csoundWarning(csound,Str("InputChannelCallback not set."));
    }
    return OK;
}

int32_t invalsetgo(CSOUND *csound, INVAL *p)
{
    int32_t ans = invalset(csound, p);
    if (ans==OK) ans = kinval(csound, p);
    return ans;
}

int32_t invalsetSgo(CSOUND *csound, INVAL *p)
{
    int32_t ans = invalset_S(csound, p);
    if (ans==OK) ans = kinval(csound, p);
    return ans;
}


int32_t invalset_string(CSOUND *csound, INVAL *p)
{
    int32_t   err;
    int32_t type;

    /* convert numerical channel to string name */
    csoundAuxAlloc(csound, 64, &p->channelName);
    snprintf((char*) p->channelName.auxp, 64, "%d", (int32_t)MYFLT2LRND(*p->valID));

    p->channelType = &CS_VAR_TYPE_S;
    type = CSOUND_STRING_CHANNEL | CSOUND_INPUT_CHANNEL;

    err = csoundGetChannelPtr(csound, (MYFLT **) &(p->channelptr),
                              (char*) p->channelName.auxp,
                              type);
    if (UNLIKELY(err))
        return print_chn_err(p, err);

    /* grab input now for use during i-pass */
    kinvalS(csound, p);
    if (!csound->InputChannelCallback_) {
        csoundWarning(csound,Str("InputChannelCallback not set."));
    }
    return OK;
}


int32_t invalset(CSOUND *csound, INVAL *p)
{
    int32_t   err;
    int32_t type;

    /* convert numerical channel to string name */
    csoundAuxAlloc(csound, 32, &p->channelName);
    snprintf((char*) p->channelName.auxp, 32, "%d", (int32_t)MYFLT2LRND(*p->valID));

    p->channelType = &CS_VAR_TYPE_K;
    type = CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL;

    err = csoundGetChannelPtr(csound, (MYFLT **) &(p->channelptr),
                              (char*) p->channelName.auxp,
                              type);
    if (UNLIKELY(err))
        return print_chn_err(p, err);

    /* grab input now for use during i-pass */
    kinval(csound, p);
    if (!csound->InputChannelCallback_) {
        csoundWarning(csound,Str("InputChannelCallback not set."));
    }
    return OK;
}



int32_t koutvalS(CSOUND *csound, OUTVAL *p)
{
    char    *chan = (char*)p->channelName.auxp;

    if (csound->OutputChannelCallback_) {
        csound->OutputChannelCallback_(csound, chan,
                                       ((STRINGDAT *)p->value)->data,
                                       p->channelType);
    }
    return OK;
}

int32_t koutval(CSOUND *csound, OUTVAL *p)
{
    char    *chan = (char*)p->channelName.auxp;

    if (csound->OutputChannelCallback_) {
        csound->OutputChannelCallback_(csound, chan, p->value, p->channelType);
        *((MYFLT *) p->channelptr) = *(p->value);
    }

    return OK;
}

int32_t outvalset_string_S(CSOUND *csound, OUTVAL *p)
{
    int32_t type, err;
    const char  *s = ((STRINGDAT *)p->valID)->data;
    csoundAuxAlloc(csound, strlen(s) + 1, &p->channelName);
    strcpy((char*) p->channelName.auxp, s);


    p->channelType = &CS_VAR_TYPE_S;
    type = CSOUND_STRING_CHANNEL | CSOUND_OUTPUT_CHANNEL;

    err = csoundGetChannelPtr(csound, (MYFLT **) &(p->channelptr),
                              (char*) p->channelName.auxp, type);

    if (UNLIKELY(err))
        return print_chn_err(p, err);

    /* send output now for use during i-pass */
    koutvalS(csound, p);
    if (!csound->OutputChannelCallback_) {
        csoundWarning(csound,Str("OutputChannelCallback not set."));
    }

    return OK;
}



int32_t outvalset_S(CSOUND *csound, OUTVAL *p)
{
    int32_t type, err;
    const char  *s = ((STRINGDAT *)p->valID)->data;
    csoundAuxAlloc(csound, strlen(s) + 1, &p->channelName);
    strcpy((char*) p->channelName.auxp, s);

    p->channelType = &CS_VAR_TYPE_K;
    type = CSOUND_CONTROL_CHANNEL | CSOUND_OUTPUT_CHANNEL;

    err = csoundGetChannelPtr(csound, (MYFLT **) &(p->channelptr),
                              (char*) p->channelName.auxp, type);
    if (UNLIKELY(err))
        return print_chn_err(p, err);

    /* send output now for use during i-pass */
    koutval(csound, p);
    if (!csound->OutputChannelCallback_) {
        csoundWarning(csound,Str("OutputChannelCallback not set."));
    }

    return OK;
}


int32_t outvalset_string(CSOUND *csound, OUTVAL *p)
{
    int32_t type, err;

    /* convert numerical channel to string name */
    if(p->channelName.auxp == NULL)
        csoundAuxAlloc(csound, 32, &p->channelName);
    snprintf((char*)p->channelName.auxp,  32, "%d",
             (int32_t)MYFLT2LRND(*p->valID));

    p->channelType = &CS_VAR_TYPE_S;
    type = CSOUND_STRING_CHANNEL | CSOUND_OUTPUT_CHANNEL;

    err = csoundGetChannelPtr(csound, (MYFLT **) &(p->channelptr),
                              (char*) p->channelName.auxp, type);
    if (UNLIKELY(err))
        return print_chn_err(p, err);

    /* send output now for use during i-pass */
    koutvalS(csound, p);
    if (!csound->OutputChannelCallback_) {
        csoundWarning(csound,Str("OutputChannelCallback not set."));
    }

    return OK;
}

int32_t outvalset(CSOUND *csound, OUTVAL *p)
{
    int32_t type, err;

    /* convert numerical channel to string name */
    csoundAuxAlloc(csound, 64, &p->channelName);
    snprintf((char*)p->channelName.auxp,  64, "%d",
             (int32_t)MYFLT2LRND(*p->valID));

    p->channelType = &CS_VAR_TYPE_K;
    type = CSOUND_CONTROL_CHANNEL | CSOUND_OUTPUT_CHANNEL;

    err = csoundGetChannelPtr(csound,(MYFLT **)  &(p->channelptr),
                              (char*) p->channelName.auxp, type);
    if (UNLIKELY(err))
        return print_chn_err(p, err);

    /* send output now for use during i-pass */
    koutval(csound, p);
    if (!csound->OutputChannelCallback_) {
        csoundWarning(csound,Str("OutputChannelCallback not set."));
    }

    return OK;
}

int32_t outvalsetgo(CSOUND *csound, OUTVAL *p)
{
    int32_t ans = outvalset(csound,p);
    // if (ans==OK) ans = koutval(csound,p);
    return ans;
}

int32_t outvalsetSgo(CSOUND *csound, OUTVAL *p)
{
    int32_t ans = outvalset_S(csound,p);
    // if (ans==OK) ans = koutval(csound,p);
    return ans;
}
