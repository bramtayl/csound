#include "csound_internal.h"
#include "text.h"
#include "csoundCore_internal.h"
#include "corfile.h"
#include "scsort.h"
#include "linevent_public.h"
#include "memalloc.h"
#include "namedins_public.h"
#include "csound_threads.h"
#include "csmodule.h"

void print_engine_parameters(CSOUND *csound) {
      csoundErrorMsg(csound, Str("sr = %.1f,"), csound->esr);
      csoundErrorMsg(csound, Str(" kr = %.3f,"), csound->ekr);
      csoundErrorMsg(csound, Str(" ksmps = %d\n"), csound->ksmps);
      csoundErrorMsg(csound, Str("0dBFS level = %.1f,"), csound->e0dbfs);
      csoundErrorMsg(csound, Str(" A4 tuning = %.1f\n"), csound->A4);
}

void csoundErrorMsgS(CSOUND *csound, int attr,
                     const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    csoundMessageV(csound, CSOUNDMSG_ERROR | attr, msg, args);
    va_end(args);
}

/**
 * Set release time in control periods (1 / csound->ekr second units)
 * for opcode 'p' to 'n'. If the current release time is longer than
 * the specified value, it is not changed.
 * Returns the new release time.
 */
int csoundSetReleaseLength(void *p, int n)
{
    if (n > (int) ((OPDS*) p)->insdshead->xtratim)
      ((OPDS*) p)->insdshead->xtratim = n;
    return (int) ((OPDS*) p)->insdshead->xtratim;
}

/**
 * Set release time in seconds for opcode 'p' to 'n'.
 * If the current release time is longer than the specified value,
 * it is not changed.
 * Returns the new release time in seconds.
 */
MYFLT csoundSetReleaseLengthSeconds(void *p, MYFLT n)
{
    int kcnt = (int) (n * ((OPDS*) p)->insdshead->csound->ekr + FL(0.5));
    if (kcnt > (int) ((OPDS*) p)->insdshead->xtratim)
      ((OPDS*) p)->insdshead->xtratim = kcnt;
    return ((MYFLT) ((OPDS*) p)->insdshead->xtratim
            * ((OPDS*) p)->insdshead->csound->onedkr);
}

static const char *midi_err_msg = Str_noop("Unknown MIDI error");

/**
 * Returns pointer to a string constant storing an error massage
 * for error code 'errcode'.
 */
const char *csoundExternalMidiErrorString(CSOUND *csound, int errcode)
{
    if (csound->midiGlobals->MidiErrorStringCallback == NULL)
      return midi_err_msg;
    return (csound->midiGlobals->MidiErrorStringCallback(errcode));
}

int csoundReadScoreInternal(CSOUND *csound, const char *str)
{
    OPARMS  *O = csound->oparms;
     /* protect resource */
    if (csound->scorestr != NULL &&
       csound->scorestr->body != NULL)
      corfile_rewind(csound->scorestr);
    csound->scorestr = corfile_create_w(csound);
    corfile_puts(csound, (char *)str, csound->scorestr);
    //#ifdef SCORE_PARSER
    if (csound->engineStatus&CS_STATE_COMP)
      corfile_puts(csound, "\n#exit\n", csound->scorestr);
    else
      corfile_puts(csound, "\ne\n#exit\n", csound->scorestr);
    //#endif
    corfile_flush(csound, csound->scorestr);
    /* copy sorted score name */
    if (csound->scstr == NULL && (csound->engineStatus & CS_STATE_COMP) == 0) {
      scsortstr(csound, csound->scorestr);
      O->playscore = csound->scstr;
      //corfile_rm(csound, &(csound->scorestr));
      //printf("%s\n", O->playscore->body);
    }
    else {
      char *sc = scsortstr(csound, csound->scorestr);
      //printf("%s\n", sc);
      csoundInputMessageInternal(csound, (const char *) sc);
      mfree(csound, sc);
      corfile_rm(csound, &(csound->scorestr));
    }
    return CSOUND_SUCCESS;
}

/* call the opcode deinitialisation routines of an instrument instance */
/* called from deact() in insert.c */

int csoundDeinitialiseOpcodes(CSOUND *csound, INSDS *ip)
{
    int err = 0;

    while (ip->nxtd != NULL) {
      opcodeDeinit_t  *dp = (opcodeDeinit_t*) ip->nxtd;
      err |= dp->func(csound, dp->p);
      ip->nxtd = (void*) dp->nxt;
      free(dp);
    }
    return err;
}

/* dummy functions for the case when no real-time audio module is available */

double *get_dummy_rtaudio_globals(CSOUND *csound)
{
    double  *p;

    p = (double*) csoundQueryGlobalVariable(csound, "__rtaudio_null_state");
    if (p == NULL) {
      if (UNLIKELY(csoundCreateGlobalVariable(csound, "__rtaudio_null_state",
                                                sizeof(double) * 4) != 0))
        csoundDie(csound, Str("rtdummy: failed to allocate globals"));
      csoundMessage(csound, Str("rtaudio: dummy module enabled\n"));
      p = (double*) csoundQueryGlobalVariable(csound, "__rtaudio_null_state");
    }
    return p;
}

int playopen_dummy(CSOUND *csound, const csRtAudioParams *parm)
{
    double  *p;
    char    *s;

    /* find out if the use of dummy real-time audio functions was requested, */
    /* or an unknown plugin name was specified; the latter case is an error  */
    s = (char*) csoundQueryGlobalVariable(csound, "_RTAUDIO");
    if (s != NULL && !(strcmp(s, "null") == 0 || strcmp(s, "Null") == 0 ||
                       strcmp(s, "NULL") == 0)) {
      if (s[0] == '\0')
        csoundErrorMsg(csound,
                       Str(" *** error: rtaudio module set to empty string"));
      else {
        // print_opcodedir_warning(csound);
        csoundErrorMsg(csound,
                       Str(" unknown rtaudio module: '%s', using dummy module"),
                       s);
      }
      // return CSOUND_ERROR;
    }
    p = get_dummy_rtaudio_globals(csound);
    csound->rtPlay_userdata = (void*) p;
    p[0] = csoundGetRealTime(csound->csRtClock);
    p[1] = 1.0 / ((double) ((int) sizeof(MYFLT) * parm->nChannels)
                  * (double) parm->sampleRate);
    return CSOUND_SUCCESS;
}

void dummy_rtaudio_timer(CSOUND *csound, double *p)
{
    double  timeWait;
    int     i;

    timeWait = p[0] - csoundGetRealTime(csound->csRtClock);
    i = (int) (timeWait * 1000.0 + 0.5);
    if (i > 0)
      csoundSleep((size_t) i);
}

void rtplay_dummy(CSOUND *csound, const MYFLT *outBuf, int nbytes)
{
    double  *p = (double*) csound->rtPlay_userdata;
    (void) outBuf;
    p[0] += ((double) nbytes * p[1]);
    dummy_rtaudio_timer(csound, p);
}

int recopen_dummy(CSOUND *csound, const csRtAudioParams *parm)
{
    double  *p;
    char    *s;

    /* find out if the use of dummy real-time audio functions was requested, */
    /* or an unknown plugin name was specified; the latter case is an error  */
    s = (char*) csoundQueryGlobalVariable(csound, "_RTAUDIO");
    if (s != NULL && !(strcmp(s, "null") == 0 || strcmp(s, "Null") == 0 ||
                       strcmp(s, "NULL") == 0)) {
      if (s[0] == '\0')
        csoundErrorMsg(csound,
                       Str(" *** error: rtaudio module set to empty string"));
      else {
        // print_opcodedir_warning(csound);
        csoundErrorMsg(csound,
                       Str(" unknown rtaudio module: '%s', using dummy module"),
                       s);
      }
      // return CSOUND_ERROR;
    }
    p = (double*) get_dummy_rtaudio_globals(csound) + 2;
    csound->rtRecord_userdata = (void*) p;
    p[0] = csoundGetRealTime(csound->csRtClock);
    p[1] = 1.0 / ((double) ((int) sizeof(MYFLT) * parm->nChannels)
                  * (double) parm->sampleRate);
    return CSOUND_SUCCESS;
}

int rtrecord_dummy(CSOUND *csound, MYFLT *inBuf, int nbytes)
{
    double  *p = (double*) csound->rtRecord_userdata;

    /* for (i = 0; i < (nbytes / (int) sizeof(MYFLT)); i++) */
    /*   ((MYFLT*) inBuf)[i] = FL(0.0); */
    memset(inBuf, 0, nbytes);

    p[0] += ((double) nbytes * p[1]);
    dummy_rtaudio_timer(csound, p);

    return nbytes;
}

void rtclose_dummy(CSOUND *csound)
{
    csound->rtPlay_userdata = NULL;
    csound->rtRecord_userdata = NULL;
}

int  audio_dev_list_dummy(CSOUND *csound,
                                 CS_AUDIODEVICE *list, int isOutput)
{
  IGN(csound); IGN(list); IGN(isOutput);
  return 0;
}

int  midi_dev_list_dummy(CSOUND *csound, CS_MIDIDEVICE *list, int isOutput){
  IGN(csound); IGN(list); IGN(isOutput);
  return 0;
}


/* dummy real time MIDI functions */
int DummyMidiInOpen(CSOUND *csound, void **userData,
                           const char *devName)
{
    char *s;

    (void) devName;
    *userData = NULL;
    s = (char*) csoundQueryGlobalVariable(csound, "_RTMIDI");
    if (UNLIKELY(s == NULL ||
        (strcmp(s, "null") == 0 || strcmp(s, "Null") == 0 ||
         strcmp(s, "NULL") == 0))) {
      csoundMessage(csound, Str("!!WARNING: real time midi input disabled, "
                                "using dummy functions\n"));
      return 0;
    }
    if (s[0] == '\0')
      csoundErrorMsg(csound, Str("error: -+rtmidi set to empty string"));
    else {
      print_opcodedir_warning(csound);
      csoundErrorMsg(csound, Str("error: -+rtmidi='%s': unknown module"), s);
    }
    return -1;
}

int DummyMidiRead(CSOUND *csound, void *userData,
                         unsigned char *buf, int nbytes)
{
    (void) csound;
    (void) userData;
    (void) buf;
    (void) nbytes;
    return 0;
}

int DummyMidiOutOpen(CSOUND *csound, void **userData,
                            const char *devName)
{
    char *s;

    (void) devName;
    *userData = NULL;
    s = (char*) csoundQueryGlobalVariable(csound, "_RTMIDI");
    if (s == NULL ||
        (strcmp(s, "null") == 0 || strcmp(s, "Null") == 0 ||
         strcmp(s, "NULL") == 0)) {
      csoundMessage(csound, Str("WARNING: real time midi output disabled, "
                                "using dummy functions\n"));
      return 0;
    }
    if (s[0] == '\0')
      csoundErrorMsg(csound, Str("error: -+rtmidi set to empty string"));
    else {
      print_opcodedir_warning(csound);
      csoundErrorMsg(csound, Str("error: -+rtmidi='%s': unknown module"), s);
    }
    return -1;
}

int DummyMidiWrite(CSOUND *csound, void *userData,
                          const unsigned char *buf, int nbytes)
{
    (void) csound;
    (void) userData;
    (void) buf;
    return nbytes;
}
