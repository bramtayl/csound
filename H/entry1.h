/*
  entry1.h:

    Copyright (C) 1991 Barry Vercoe, John ffitch

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

#pragma once

#include "csoundCore_internal.h"         /*                      ENTRY1.H        */
#include "insert.h"
#include "aops.h"
#include "midiops.h"
#include "ugens1.h"
#include "ugens2.h"
#include "ugens3.h"
#include "ugens4.h"
#include "ugens5.h"
#include "ugens6.h"
#include "cwindow.h"
#include "windin.h"
#include "disprep.h"
#include "soundio.h"
#include "dumpf.h"
#include "cmath.h"
#include "diskin2.h"
#include "oload.h"
#include "midiout.h"
#include "sndinfUG.h"
#include "ugrw1.h"
#include "vdelay.h"
#include "pstream.h"
#include "oscils.h"
#include "midifile.h"
#include "midiinterop.h"
#include "linevent.h"
#include "str_ops.h"
#include "bus.h"
#include "pstream.h"
#include "remote.h"
#include "resize.h"
#include "cs_par_ops.h"
#include "ugtabs.h"
#include "compile_ops.h"
#include "lpred.h"

#define S(x)    sizeof(x)

int32_t init(CSOUND *, void *);

int32_t kmbset(CSOUND *, void *);
int32_t ipchmidib(CSOUND *, void *), ioctmidib(CSOUND *, void *);
int32_t kpchmidib(CSOUND *, void *), koctmidib(CSOUND *, void *);
int32_t msclset(CSOUND *, void *);
int32_t chpress(CSOUND *, void *);
int32_t midiout(CSOUND *, void *), turnon(CSOUND *, void *);
int32_t turnon_S(CSOUND *, void *);
int32_t kmapset(CSOUND *, void *), polyaft(CSOUND *, void *);
int32_t linset(CSOUND *, void *);
int32_t kline(CSOUND *, void *), aline(CSOUND *, void *);
int32_t expset(CSOUND *, void *), kexpon(CSOUND *, void *);
int32_t expon(CSOUND *, void *), lsgset(CSOUND *, void *);
int32_t klnseg(CSOUND *, void *), linseg(CSOUND *, void *);
int32_t csgset(CSOUND *, void *), kosseg(CSOUND *, void *);
int32_t csgset_bkpt(CSOUND *, void *), cosseg(CSOUND *, void *);
int32_t csgrset(CSOUND *, void *);
int32_t kcssegr(CSOUND *, void *), cossegr(CSOUND *, void *);
int32_t madsrset(CSOUND *, void *), adsrset(CSOUND *, void *);
int32_t xdsrset(CSOUND *, void *), mxdsrset(CSOUND *, void *);
int32_t expseg2(CSOUND *, void *), xsgset(CSOUND *, void *);
int32_t kxpseg(CSOUND *, void *), expseg(CSOUND *, void *);
int32_t xsgset2(CSOUND *, void *), lsgrset(CSOUND *, void *);
int32_t klnsegr(CSOUND *, void *), linsegr(CSOUND *, void *);
int32_t xsgrset(CSOUND *, void *), kxpsegr(CSOUND *, void *);
int32_t expsegr(CSOUND *, void *), lnnset(CSOUND *, void *);
int32_t klinen(CSOUND *, void *), linen(CSOUND *, void *);
int32_t lnrset(CSOUND *, void *), klinenr(CSOUND *, void *);
int32_t linenr(CSOUND *, void *), evxset(CSOUND *, void *);
int32_t knvlpx(CSOUND *, void *), envlpx(CSOUND *, void *);
int32_t evrset(CSOUND *, void *), knvlpxr(CSOUND *, void *);
int32_t envlpxr(CSOUND *, void *), phsset(CSOUND *, void *);
int32_t ephsset(CSOUND *, void *), ephsor(CSOUND *, void *);
int32_t kphsor(CSOUND *, void *), phsor(CSOUND *, void *);
int32_t ko1set(CSOUND *, void *);
int32_t kosc1(CSOUND *, void *), kosc1i(CSOUND *, void *);
int32_t oscnset(CSOUND *, void *), osciln(CSOUND *, void *);
int32_t oscset(CSOUND *, void *), koscil(CSOUND *, void *);
int32_t oscsetA(CSOUND *, void *);
int32_t osckk(CSOUND *, void *), oscka(CSOUND *, void *);
int32_t oscak(CSOUND *, void *), oscaa(CSOUND *, void *);
int32_t koscli(CSOUND *, void *), osckki(CSOUND *, void *);
int32_t osckai(CSOUND *, void *), oscaki(CSOUND *, void *);
int32_t oscaai(CSOUND *, void *), foscset(CSOUND *, void *);
int32_t foscil(CSOUND *, void *), foscili(CSOUND *, void *);
int32_t losset(CSOUND *, void *), loscil(CSOUND *, void *);
int32_t loscil3(CSOUND *, void *), koscl3(CSOUND *, void *);
int32_t osckk3(CSOUND *, void *), oscka3(CSOUND *, void *);
int32_t oscak3(CSOUND *, void *), oscaa3(CSOUND *, void *);
int32_t adset(CSOUND *, void *), adsyn(CSOUND *, void *);
int32_t bzzset(CSOUND *, void *), buzz(CSOUND *, void *);
int32_t gbzset(CSOUND *, void *), gbuzz(CSOUND *, void *);
int32_t plukset(CSOUND *, void *), pluck(CSOUND *, void *);
int32_t rndset(CSOUND *, void *), krand(CSOUND *, void *);
int32_t arand(CSOUND *, void *), rhset(CSOUND *, void *);
int32_t krandh(CSOUND *, void *), randh(CSOUND *, void *);
int32_t riset(CSOUND *, void *), krandi(CSOUND *, void *);
int32_t rcset(CSOUND *, void *), randc(CSOUND *, void *);
int32_t krandc(CSOUND *, void *);
int32_t randi(CSOUND *, void *), rndset2(CSOUND *, void *);
int32_t krand2(CSOUND *, void *), arand2(CSOUND *, void *);
int32_t rhset2(CSOUND *, void *), krandh2(CSOUND *, void *);
int32_t randh2(CSOUND *, void *), riset2(CSOUND *, void *);
int32_t krandi2(CSOUND *, void *), randi2(CSOUND *, void *);
int32_t porset(CSOUND *, void *), port(CSOUND *, void *);
int32_t tonset(CSOUND *, void *), tone(CSOUND *, void *);
int32_t atone(CSOUND *, void *), rsnset(CSOUND *, void *);
int32_t reson(CSOUND *, void *), areson(CSOUND *, void *);
int32_t resonx(CSOUND *, void *), aresonx(CSOUND *, void *);
int32_t rsnsetx(CSOUND *, void *), tonex(CSOUND *, void *);
int32_t atonex(CSOUND *, void *), tonsetx(CSOUND *, void *);
int32_t lprdset(CSOUND *, void *), lpread(CSOUND *, void *);
int32_t lpformantset(CSOUND *, void *), lpformant(CSOUND *, void*);
int32_t lprsnset(CSOUND *, void *), lpreson(CSOUND *, void *);
int32_t lpfrsnset(CSOUND *, void *), lpfreson(CSOUND *, void *);
int32_t lpslotset(CSOUND *, void *), lpitpset(CSOUND *, void *);
int32_t lpinterpol(CSOUND *, void *);
int32_t rmsset(CSOUND *, void *), rms(CSOUND *, void *);
int32_t gainset(CSOUND *, void *), gain(CSOUND *, void *);
int32_t outq1(CSOUND *, void *), outq2(CSOUND *, void *);
int32_t xyinset(CSOUND *, void *);
int32_t tempset(CSOUND *, void *), tempo(CSOUND *, void *);
int32_t old_kdmpset(CSOUND *, void *), old_kdmp2set(CSOUND *, void *);
int32_t old_kdmp3set(CSOUND *, void *), old_kdmp4set(CSOUND *, void *);
int32_t newsndinset(CSOUND *, void *), soundinew(CSOUND *, void *);
int32_t iout_on(CSOUND *, void *), iout_off(CSOUND *, void *);
int32_t out_controller(CSOUND *, void *), iout_on_dur_set(CSOUND *, void *);
int32_t iout_on_dur(CSOUND *, void *), iout_on_dur2(CSOUND *, void *);
int32_t moscil_set(CSOUND *, void *), moscil(CSOUND *, void *);
int32_t kvar_out_on_set(CSOUND *, void *), kvar_out_on_set1(CSOUND *, void *);
int32_t kvar_out_on(CSOUND *, void *), out_controller14(CSOUND *, void *);
int32_t out_pitch_bend(CSOUND *, void *), out_aftertouch(CSOUND *, void *);
int32_t out_poly_aftertouch(CSOUND*, void*), out_progchange(CSOUND*, void*);
int32_t release_set(CSOUND *, void *), release(CSOUND *, void *);
int32_t xtratim(CSOUND *, void *);
int32_t mclock_set(CSOUND *, void *), mclock(CSOUND *, void *);
int32_t mrtmsg(CSOUND *, void *);
int32_t cabasaset(CSOUND *, void *), cabasa(CSOUND *, void *);
int32_t sekereset(CSOUND *, void *), sandset(CSOUND *, void *);
int32_t stixset(CSOUND *, void *), crunchset(CSOUND *, void *);
int32_t guiroset(CSOUND *, void *), guiro(CSOUND *, void *);
int32_t sekere(CSOUND *, void *);
int32_t tambourset(CSOUND *, void *), tambourine(CSOUND *, void *);
int32_t bambooset(CSOUND *, void *), bamboo(CSOUND *, void *);
int32_t wuterset(CSOUND *, void *), wuter(CSOUND *, void *);
int32_t sleighset(CSOUND *, void *), sleighbells(CSOUND *, void *);
int32_t trig_set(CSOUND *, void *), trig(CSOUND *, void *);
int32_t kon2_set(CSOUND *, void *), kon2(CSOUND *, void *);
int32_t nrpn(CSOUND *, void *);
int32_t mdelay(CSOUND *, void *), mdelay_set(CSOUND *, void *);
int32_t sum(CSOUND *, void *), product(CSOUND *, void *);
int32_t macset(CSOUND *, void *);
int32_t mac(CSOUND *, void *), maca(CSOUND *, void *);
int32_t nestedapset(CSOUND *, void *), nestedap(CSOUND *, void *);
int32_t lorenzset(CSOUND *, void *), lorenz(CSOUND *, void *);
int32_t filelen(CSOUND *, void *), filenchnls(CSOUND *, void *);
int32_t filesr(CSOUND *, void *), filepeak(CSOUND *, void *);
int32_t filevalid(CSOUND *, void *);
int32_t filelen_S(CSOUND *, void *), filenchnls_S(CSOUND *, void *);
int32_t filesr_S(CSOUND *, void *), filepeak_S(CSOUND *, void *);
int32_t filevalid_S(CSOUND *, void *);
int32_t lp2_set(CSOUND *, void *), lp2(CSOUND *, void *);
int32_t phaser2set(CSOUND *, void *), phaser2(CSOUND *, void *);
int32_t phaser1set(CSOUND *, void *), phaser1(CSOUND *, void *);
int32_t balnset(CSOUND *, void *), balance(CSOUND *, void *);
int32_t prealloc(CSOUND *, void *);
int32_t prealloc_S(CSOUND *, void *), active_alloc(CSOUND*, void*);
int32_t wgpsetin(CSOUND *, void *);
int32_t wgpset(CSOUND *, void *), wgpluck(CSOUND *, void *);
int32_t clarinset(CSOUND *, void *), clarin(CSOUND *, void *);
int32_t fluteset(CSOUND *, void *), flute(CSOUND *, void *);
int32_t bowedset(CSOUND *, void *), bowed(CSOUND *, void *);
int32_t brassset(CSOUND *, void *), brass(CSOUND *, void *);
int32_t triginset_S(CSOUND *, void *), ktriginstr_S(CSOUND *, void *);
int32_t kill_instance(CSOUND *csound, void *p);
int32_t gettempo(CSOUND *, void *);
int32_t loopseg_set(CSOUND *, void *);
int32_t loopseg(CSOUND *, void *), lpshold(CSOUND *, void *);
int32_t lineto_set(CSOUND *, void *), lineto(CSOUND *, void *);
int32_t tlineto_set(CSOUND *, void *), tlineto(CSOUND *, void *);
int32_t vibrato_set(CSOUND *, void *), vibrato(CSOUND *, void *);
int32_t vibr_set(CSOUND *, void *), vibr(CSOUND *, void *);
int32_t randomi_set(CSOUND *, void *);
int32_t krandomi(CSOUND *, void *), randomi(CSOUND *, void *);
int32_t randomh_set(CSOUND *, void *);
int32_t krandomh(CSOUND *, void *), randomh(CSOUND *, void *);
int32_t random3_set(CSOUND *, void *);
int32_t random3(CSOUND *, void *), random3a(CSOUND *, void *);
int32_t subinstrset_S(CSOUND *, void *);
int32_t subinstrset(CSOUND *, void *), subinstr(CSOUND *, void *);
int32_t useropcdset(CSOUND *, void *), useropcd(CSOUND *, void *);
int32_t setksmpsset(CSOUND *, void *);
int32_t xinset(CSOUND *, void *), xoutset(CSOUND *, void *);
int32_t nstrnumset(CSOUND *, void *);
int32_t nstrnumset_S(CSOUND *, void *), nstrstr(CSOUND *, void *);
int32_t delete_instr(CSOUND *, void *);
int32_t insremot(CSOUND *, void *), insglobal(CSOUND *, void *);
int32_t midremot(CSOUND *, void *), midglobal(CSOUND *, void *);
int32_t remoteport(CSOUND *, void *);
int32_t globallock(CSOUND *, void *);
int32_t globalunlock(CSOUND *, void *);
int32_t filebit(CSOUND *, void *); int32_t filebit_S(CSOUND *, void *);
int32_t lsgset_bkpt(CSOUND *csound, void *p);
int32_t xsgset_bkpt(CSOUND *csound, void *p);
int32_t xsgset_bkpt(CSOUND *csound, void *p), xsgset2b(CSOUND *, void *);
int32_t tabler_init(CSOUND *csound, TABL *p);
int32_t tabl_setup(CSOUND *csound, TABL *p);
int32_t tabler_kontrol(CSOUND *csound, TABL *p);
int32_t tabler_audio(CSOUND *csound, TABL *p);
int32_t tableir_init(CSOUND *csound, TABL *p);
int32_t tableir_audio(CSOUND *csound, TABL *p);
int32_t tableir_kontrol(CSOUND *csound, TABL *p);
int32_t tableir_audio(CSOUND *csound, TABL *p);
int32_t table3r_init(CSOUND *csound, TABL *p);
int32_t table3r_kontrol(CSOUND *csound, TABL *p);
int32_t table3r_audio(CSOUND *csound, TABL *p);
int32_t tablerkt_kontrol(CSOUND *csound, TABL *p);
int32_t tablerkt_audio(CSOUND *csound, TABL *p);
int32_t tableirkt_kontrol(CSOUND *csound, TABL *p);
int32_t tableirkt_audio(CSOUND *csound, TABL *p);
int32_t table3rkt_kontrol(CSOUND *csound, TABL *p);
int32_t table3rkt_audio(CSOUND *csound, TABL *p);
int32_t tablew_init(CSOUND *csound, TABL *p);
int32_t tablew_kontrol(CSOUND *csound, TABL *p);
int32_t tablew_audio(CSOUND *csound, TABL *p);
int32_t tablewkt_kontrol(CSOUND *csound, TABL *p);
int32_t tablewkt_audio(CSOUND *csound, TABL *p);
int32_t table_length(CSOUND *csound, TLEN *p);
int32_t table_gpw(CSOUND *csound, TGP *p);
int32_t table_copy(CSOUND *csound, TGP *p);
int32_t table_mix(CSOUND *csound, TABLMIX *p);
int32_t table_ra_set(CSOUND *csound, TABLRA *p);
int32_t table_ra(CSOUND *csound, TABLRA *p);
int32_t table_wa_set(CSOUND *csound, TABLWA *p);
int32_t table_wa(CSOUND *csound, TABLWA *p);
int32_t tablkt_setup(CSOUND *csound, TABL *p);
int32_t adset_S(CSOUND *csound, void *p);
int32_t lprdset_S(CSOUND *csound, void *p);
int32_t alnnset(CSOUND *csound, void *p);
int32_t alnrset(CSOUND *csound, void *p);
int32_t aevxset(CSOUND *csound, void *p);
int32_t aevrset(CSOUND *csound, void *p);
int32_t losset_phs(CSOUND *, void *);
int32_t loscil_phs(CSOUND *, void *);
int32_t loscil3_phs(CSOUND *, void *);
int32_t balance2(CSOUND *, void *);

