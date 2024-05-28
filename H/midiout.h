/*
    midiout.h:

    Copyright (C) 1997 Gabriel Maldonado, John ffitch

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

/****************************************/
/** midiout UGs by Gabriel Maldonado   **/
/****************************************/

#pragma once

#include "csoundCore_common.h"

typedef int BOOL;
#ifndef TRUE
#   define TRUE (1)
#endif
#ifndef FALSE
#   define FALSE (0)
#endif

typedef struct {
    OPDS   h;
    MYFLT  *r;
} REL;

typedef struct {
    OPDS   h;
    MYFLT  *extradur;
} XTRADUR;

typedef struct {
    OPDS        h;
    MYFLT       *freq;
    MYFLT       period, clock_tics;
    int         beginning_flag;
} MCLOCK;

typedef struct {
    OPDS        h;
    MYFLT       *message;
} MRT;

typedef struct {
    OPDS   h;
    MYFLT  *ichn,*inum,*ivel;
} OUT_ON;

typedef struct {
    OPDS   h;
    MYFLT  *ichn,*inum,*ivel,*idur;
    MYFLT  istart_time;
    int    chn, num, vel;
    BOOL   fl_expired, fl_extra_dur;
} OUT_ON_DUR;

typedef struct {
    OPDS   h;
    MYFLT  *kchn,*knum,*kvel,*kdur,*kpause;
    MYFLT  istart_time;
    int    last_chn, last_num, last_vel;
    MYFLT  last_dur, last_pause;
    BOOL   fl_note_expired, fl_first_note, fl_end_note;
} MOSCIL;

typedef struct {
    OPDS   h;
    MYFLT  *kchn,*knum,*kvel;
    int    last_chn, last_num, last_vel;
    BOOL   fl_note_expired, fl_first_note;
} KOUT_ON;

typedef struct {
    OPDS   h;
    MYFLT  *chn,*num, *value, *min, *max;
  int    last_value, lastchn, lastctrl;
} OUT_CONTR;

typedef struct {
    OPDS   h;
    MYFLT  *chn, *msb_num, *lsb_num, *value, *min, *max;
    int    last_value, lastchn, lastctrl;
} OUT_CONTR14;

typedef struct {
    OPDS   h;
    MYFLT  *chn, *value, *min, *max;
  int    last_value,  lastchn;
} OUT_PB;

typedef struct {
    OPDS   h;
    MYFLT  *chn, *value, *min, *max;
  int    last_value,  lastchn;
} OUT_ATOUCH;

typedef struct {
    OPDS   h;
    MYFLT  *chn, *prog_num, *min, *max;
  int    last_prog_num,  lastchn;
} OUT_PCHG;

typedef struct {
    OPDS   h;
    MYFLT  *chn, *num, *value, *min, *max;
  int    last_value, lastchn, lastctrl;
} OUT_POLYATOUCH;

typedef struct {
    OPDS   h;
    MYFLT  *kchn,*knum,*kvel,*ktrig;
        int     last_chn, last_num, last_vel;
        BOOL fl_note_expired/*, fl_first_note*/;
} KON2;

typedef struct {
    OPDS   h;
    MYFLT  *in_type, *in_chan, *in_dat1, *in_dat2;
} MIDIOUT;

typedef struct {
    OPDS   h;
    MYFLT  *chan, *parm_num, *parm_value;
    int old_chan, old_parm, old_value;
} NRPN;

typedef struct {
    unsigned char status;
    unsigned char dat1;
    unsigned char dat2;
    MYFLT   delay;
} DELTAB;

#define DELTAB_LENGTH 1000

typedef struct {
    OPDS   h;
    MYFLT  *in_status, *in_chan, *in_dat1, *in_dat2, *kdelay;
    unsigned char status[DELTAB_LENGTH];
    unsigned char chan[DELTAB_LENGTH];
    unsigned char dat1[DELTAB_LENGTH];
    unsigned char dat2[DELTAB_LENGTH];
    MYFLT             time[DELTAB_LENGTH];
    unsigned int  write_index, read_index;
} MDELAY;

int32_t midiout(CSOUND *, MIDIOUT *);
int32_t iout_on(CSOUND *, OUT_ON *), iout_off(CSOUND *, OUT_ON *);
int32_t out_controller(CSOUND *, OUT_CONTR *), iout_on_dur_set(CSOUND *, OUT_ON_DUR *);
int32_t iout_on_dur(CSOUND *, OUT_ON_DUR *), iout_on_dur2(CSOUND *, OUT_ON_DUR *);
int32_t moscil_set(CSOUND *, MOSCIL *), moscil(CSOUND *, MOSCIL *);
int32_t kvar_out_on_set(CSOUND *, KOUT_ON *);
int32_t kvar_out_on(CSOUND *, KOUT_ON *), out_controller14(CSOUND *, OUT_CONTR14 *);
int32_t out_pitch_bend(CSOUND *, OUT_PB *), out_aftertouch(CSOUND *, OUT_ATOUCH *);
int32_t out_poly_aftertouch(CSOUND*, OUT_POLYATOUCH*), out_progchange(CSOUND*, OUT_PCHG*);
int32_t release_set(CSOUND *, REL *), release(CSOUND *, REL *);
int32_t xtratim(CSOUND *, XTRADUR *);
int32_t mclock_set(CSOUND *, MCLOCK *), mclock(CSOUND *, MCLOCK *);
int32_t mrtmsg(CSOUND *, MRT *);
int32_t kon2_set(CSOUND *, KON2 *), kon2(CSOUND *, KON2 *);
int32_t nrpn(CSOUND *, NRPN *);
int32_t mdelay(CSOUND *, MDELAY *), mdelay_set(CSOUND *, MDELAY *);