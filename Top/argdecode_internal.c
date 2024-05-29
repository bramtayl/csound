/*
    argdecode.c:

    Copyright (C) 1998-2013 John ffitch, Victor Lazzarini

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
#include "csoundCore_internal.h"         /*                      ARGDECODE.C     */
#include "soundio.h"
#include "new_opts.h"
#include "csmodule.h"
#include "corfile.h"
#include <ctype.h>
#include "memalloc.h"
#include "csound_orc_semantics_public.h"
#include "envvar_public.h"
#include "server.h"
#include "utility.h"
#include "argdecode.h"
#include "text.h"
#include "argdecode_internal.h"
#include "opcode_internal.h"
#include "one_file.h"
#include "str_ops.h"
#include "musmon_internal.h"
#include "libsnd_internal.h"

static void list_audio_devices(CSOUND *csound, int output);
static void list_midi_devices(CSOUND *csound, int output);

#define FIND(MSG)   if (*s == '\0')  \
    if (UNLIKELY(!(--argc) || (((s = *++argv) != NULL) && *s == '-')))  \
      dieu(csound, MSG);

/* IV - Feb 19 2005 */

#ifdef EXPERIMENTAL
static FILE *logFile = NULL;    /* NOT THREAD SAFE */

void msg_callback(CSOUND *csound,
                         int attr, const char *format, va_list args)
{
    (void) csound;
    if ((attr & CSOUNDMSG_TYPE_MASK) != CSOUNDMSG_REALTIME) {
      vfprintf(logFile, format, args);
      fflush(logFile);
      return;
     }
    #if defined(_WIN32) || defined(__APPLE__)
    switch (attr & CSOUNDMSG_TYPE_MASK) {
        case CSOUNDMSG_ERROR:
        case CSOUNDMSG_WARNING:
        case CSOUNDMSG_REALTIME:
        break;
      default:
        vfprintf(logFile, format, args);
        return;
    }
    #endif

    vfprintf(stderr, format, args);
}

void nomsg_callback(CSOUND *csound,
  int attr, const char *format, va_list args){ return; }

void do_logging(char *s)
{
    int nomessages = 0;
    if (logFile) return;
    if (!strcmp(s, "NULL") || !strcmp(s, "null"))
      nomessages = 1;
    else if ((logFile = fopen(s, "w")) == NULL) {
      #ifndef __wasi__
      fprintf(stderr, Str("Error opening log file '%s': %s\n"),
              s, strerror(errno));
      exit(1);
      #endif
    }
    /* if logging to file, set message callback */
    if (logFile != NULL)
      csoundSetDefaultMessageCallback(msg_callback);
    else if (nomessages)
      csoundSetDefaultMessageCallback(nomsg_callback);
}

void end_logging(void)
{
    if (logFile != NULL)
      fclose(logFile);
}
#else
#define do_logging(x) {}
#endif

/* IV - Feb 19 2005 */
static const char *shortUsageList[] = {
  Str_noop("--help      print long usage options"),
  Str_noop("--version   print version details"),
  Str_noop("-U unam     run utility program unam"),
  Str_noop("-C          use Cscore processing of scorefile"),
  Str_noop("-j N        use N threads in performance"),
  Str_noop("-I          I-time only orch run"),
  Str_noop("-n          no sound onto disk"),
  Str_noop("-i fnam     sound input filename"),
  Str_noop("-o fnam     sound output filename"),
  Str_noop("-b N        sample frames (or -kprds) per software sound I/O buffer"),
  Str_noop("-B N        samples per hardware sound I/O buffer"),
  Str_noop("-A          create an AIFF format output soundfile"),
  Str_noop("-W          create a WAV format output soundfile"),
  Str_noop("-J          create an IRCAM format output soundfile"),
  Str_noop("-h          no header on output soundfile"),
  Str_noop("-c          8-bit signed_char sound samples"),
#ifdef never
  Str_noop("-a          alaw sound samples"),
#endif
  Str_noop("-8          8-bit unsigned_char sound samples"),
  Str_noop("-u          ulaw sound samples"),
  Str_noop("-s          short_int sound samples"),
  Str_noop("-l          long_int sound samples"),
  Str_noop("-f          float sound samples"),
  Str_noop("-3          24bit sound samples"),
  Str_noop("-r N        orchestra srate override"),
  Str_noop("-k N        orchestra krate override"),
  Str_noop("-K          do not generate PEAK chunks"),
  Str_noop("-v          verbose orch translation"),
  Str_noop("-m N        tty message level. Sum of:"),
  Str_noop("              1=note amps, 2=out-of-range msg, 4=warnings"),
  Str_noop("              0/32/64/96=note amp format (raw,dB,colors)"),
  Str_noop("              128=print benchmark information"),
  Str_noop("-d          suppress all displays"),
  Str_noop("-g          suppress graphics, use ascii displays"),
  Str_noop("-G          suppress graphics, use Postscript displays"),
  Str_noop("-x fnam     extract from score.srt using extract file 'fnam'"),
  Str_noop("-t N        use uninterpreted beats of the score, "
                       "initially at tempo N"),
  Str_noop("-t 0        use score.srt for sorted score rather than a temporary"),
  Str_noop("-L dnam     read Line-oriented realtime score events from "
                       "device 'dnam'"),
  Str_noop("-M dnam     read MIDI realtime events from device 'dnam'"),
  Str_noop("-F fnam     read MIDIfile event stream from file 'fnam'"),
  /* Str_noop("-P N        MIDI sustain pedal threshold (0 - 128)"), */
  Str_noop("-R          continually rewrite header while writing soundfile "
                       "(WAV/AIFF)"),
  Str_noop("-H#         print heartbeat style 1, 2 or 3 at each soundfile write"),
  Str_noop("-N          notify (ring the bell) when score or miditrack is done"),
  Str_noop("-T          terminate the performance when miditrack is done"),
  Str_noop("-D          defer GEN01 soundfile loads until performance time"),
  Str_noop("-Q dnam     select MIDI output device"),
  Str_noop("-z          list opcodes in this version"),
  Str_noop("-Z          dither output"),
#if defined(__gnu_linux__)
  Str_noop("--sched     set real-time priority and lock memory"),
  Str_noop("              (requires -d and real time audio (-iadc/-odac))"),
  Str_noop("--sched=N   set specified scheduling priority, and lock memory"),
  Str_noop("              (requires -d and real time audio (-iadc/-odac))"),
#endif
  NULL
};

static const char *longUsageList[] = {
  "--format={wav,aiff,au,raw,paf,svx,nist,voc,ircam,w64,mat4,mat5",
  "          pvf,xi,htk,sds,avr,wavex,sd2,flac,caf,wve,ogg,mpc2k,rf64,mpeg}",
  "--format={alaw,ulaw,schar,uchar,float,double,short,long,24bit,vorbis}",
  Str_noop("  Set output file format"),
  Str_noop("--aiff                  set AIFF format"),
  Str_noop("--au                    set AU format"),
  Str_noop("--wave                  set WAV format"),
  Str_noop("--ircam                 set IRCAM format"),
  Str_noop("--ogg                   set OGG/VORBIS format"),
  Str_noop("--mpeg                  set MPEG format"),
  Str_noop("--noheader              raw format"),
  Str_noop("--nopeaks               do not write peak information"),
  " ",
  Str_noop("--displays              use graphic displays"),
  Str_noop("--nodisplays            suppress all displays"),
  Str_noop("--asciidisplay          suppress graphics, use ascii displays"),
  Str_noop("--postscriptdisplay     suppress graphics, use Postscript displays"),
  " ",
  Str_noop("--defer-gen1            defer GEN01 soundfile loads until "
                                   "performance time"),
  Str_noop("--iobufsamps=N          sample frames (or -kprds) per software "
                                    "sound I/O buffer"),
  Str_noop("--hardwarebufsamps=N    samples per hardware sound I/O buffer"),
  Str_noop("--cscore                use Cscore processing of scorefile"),
  Str_noop("--orc                   use orchfile without scorefile"),
  " ",
  Str_noop("--midifile=FNAME        read MIDIfile event stream from file"),
  Str_noop("--midioutfile=FNAME     write MIDI output to file FNAME"),
  Str_noop("--midi-device=FNAME     read MIDI realtime events from device"),
  Str_noop("--terminate-on-midi     terminate the performance when miditrack "
           "is done"),
  " ",
  Str_noop("--heartbeat=N           print a heartbeat style 1, 2 or 3 at "
                                    "each soundfile write"),
  Str_noop("--notify                notify (ring the bell) when score or "
           "miditrack is done"),
  Str_noop("--rewrite               continually rewrite header while writing "
                                    "soundfile (WAV/AIFF)"),
  " ",
  Str_noop("--input=FNAME           sound input filename"),
  Str_noop("--output=FNAME          sound output filename"),
  Str_noop("--logfile=FNAME         log output to file"),
  " ",
  Str_noop("--nosound               no sound onto disk or device"),
  Str_noop("--tempo=N               use uninterpreted beats of the score, "
                                   "initially at tempo N"),
  Str_noop("--i-only                I-time only orch run"),
  Str_noop("--syntax-check-only     stop after checking orchestra and "
                                    "score syntax"),
  Str_noop("--control-rate=N        orchestra krate override"),
  Str_noop("--sample-rate=N         orchestra srate override"),
  Str_noop("--score-in=FNAME        read line-oriented realtime score events "
                                    "from device"),
  Str_noop("--messagelevel=N        tty message level, sum of:"),
  Str_noop("--messageolevel=O       tty message level in octal, of:"),
  Str_noop("                          1=note amps, 2=out-of-range msg, 4=warnings,"),
  Str_noop("                          0/32/64/96=note amp format (raw,dB,colors),"),
  Str_noop("                          128=print benchmark information"),
  " ",
  Str_noop("--m-amps=[01]           messages on note amps"),
  Str_noop("--m-range=[01]          messages on range errors"),
  Str_noop("--m-warnings=[01]       message on warnings"),
  Str_noop("--m-raw=[01]            raw amp messages"),
  Str_noop("--m-dB=[01]             amp messages in dB"),
  Str_noop("--m-colours=[01]        colour amp messages"),
  Str_noop("--m-benchmarks=[01]     print benchmark information"),
  Str_noop("--csd-line-nums=[01]    controls how error line numbers are printed:"),
  Str_noop("                          1=use CSD line #s (default), 0=use "
                                   "ORC/SCO-relative line #s"),
  Str_noop("--extract-score=FNAME   extract from score.srt using extract file"),
  Str_noop("--keep-sorted-score"),
  Str_noop("--keep-sorted-score=FNAME"),
  Str_noop("--simple-sorted-score"),
  Str_noop("--simple-sorted-score=FNAME"),
  Str_noop("--env:NAME=VALUE        set environment variable NAME to VALUE"),
  Str_noop("--env:NAME+=VALUE       append VALUE to environment variable NAME"),
  Str_noop("--strsetN=VALUE         set strset table at index N to VALUE"),
  Str_noop("--utility=NAME          run utility program"),
  Str_noop("--verbose               verbose orch translation"),
  Str_noop("--list-opcodes          list opcodes in this version"),
  Str_noop("--list-opcodesN         list opcodes in style N in this version"),
  Str_noop("--dither                dither output"),
  Str_noop("--dither-triangular     dither output with triangular distribution"),
  Str_noop("--dither-uniform        dither output with rectanular distribution"),
  Str_noop("--sched                 set real-time scheduling priority and "
                                   "lock memory"),
  Str_noop("--sched=N               set priority to N and lock memory"),
  Str_noop("--opcode-dir=DIR        load all plugins from DIR"),
  Str_noop("--opcode-lib=NAMES      dynamic libraries to load"),
  Str_noop("--opcode-omit=NAMES     dynamic libraries not to load"),
  Str_noop("--omacro:XXX=YYY        set orchestra macro XXX to value YYY"),
  Str_noop("--smacro:XXX=YYY        set score macro XXX to value YYY"),
  Str_noop("--midi-key=N            route MIDI note on message"),
  Str_noop("                          key number to pfield N as "
                                   "MIDI value [0-127]"),
  Str_noop("--midi-key-cps=N        route MIDI note on message"),
  Str_noop("                          key number to pfield N as cycles per second"),
  Str_noop("--midi-key-oct=N        route MIDI note on message"),
  Str_noop("                          key number to pfield N as linear octave"),
  Str_noop("--midi-key-pch=N        route MIDI note on message"),
  Str_noop("                          key number to pfield N as oct.pch"),
  Str_noop("--midi-velocity=N       route MIDI note on message"),
  Str_noop("                          velocity number to pfield N as MIDI "
                                   "value [0-127]"),
  Str_noop("--midi-velocity-amp=N   route MIDI note on message"),
  Str_noop("                          velocity number to pfield N as amplitude"),
  Str_noop("--no-default-paths      turn off relative paths from CSD/ORC/SCO"),
  Str_noop("--sample-accurate       use sample-accurate timing of score events"),
  Str_noop("--realtime              realtime priority mode"),
  Str_noop("--nchnls=N              override number of audio channels"),
  Str_noop("--nchnls_i=N            override number of input audio channels"),
  Str_noop("--0dbfs=N               override 0dbfs (max positive signal amplitude)"),
  Str_noop("--sinesize              length of internal sine table"),
  Str_noop("--daemon                daemon mode: do not exit if CSD/orchestra is "
                                    "not given, is empty or does not compile"),
  Str_noop("--port=N                listen to UDP port N for instruments/orchestra "
                                    "code (implies --daemon)"),
  Str_noop("--vbr-quality=Ft        set quality of variable bit-rate compression"),
  Str_noop("--devices[=in|out]      list available audio devices and exit"),
  Str_noop("--midi-devices[=in|out] list available MIDI devices and exit"),
  Str_noop("--get-system-sr         print system sr and exit, requires realtime\n"
       "                        audio output (e.g. -odac) to be defined first)"),
  Str_noop("--use-system-sr         print system sr and use realtime audio\n"
           "                        output (e.g. -odac) to be defined first"),
  Str_noop("--ksmps=N               override ksmps"),
  Str_noop("--fftlib=N              actual FFT lib to use (FFTLIB=0, "
                                   "PFFFT = 1, vDSP =2)"),
  Str_noop("--udp-echo              echo UDP commands on terminal"),
  Str_noop("--aft-zero              set aftertouch to zero, not 127 (default)"),
  Str_noop("--limiter[=num]         include clipping in audio output"),
  Str_noop("--vbr                   set MPEG encoding to variable bitrate"),
  " ",
  Str_noop("--help                  long help"),
  NULL
};

/* IV - Feb 19 2005 */
void print_short_usage(CSOUND *csound)
{
    char    buf[256];
    int     i;
    i = -1;
    print_csound_version(csound);
    csoundMessage(csound, Str("\nShort options format:\n"));
    while (shortUsageList[++i] != NULL) {
      snprintf(buf, 256, "%s\n", shortUsageList[i]);
      csoundMessage(csound, "%s", Str(buf));
    }
    csoundMessage(csound,
                    Str("flag defaults: csound -s -otest -b%d -B%d -m%d\n\n"),
                    IOBUFSAMPS, IODACSAMPS, csound->oparms->msglevel);
}

static void longusage(CSOUND *p)
{
    const char **sp;
    print_short_usage(p);
    csoundMessage(p, Str("Usage:     csound [-flags] orchfile scorefile\n"));
    csoundMessage(p, Str("Legal flags are:\n"));
    csoundMessage(p, Str("Long format:\n\n"));
    for (sp = &(longUsageList[0]); *sp != NULL; sp++)
      csoundMessage(p, "%s\n", Str(*sp));
    dump_cfg_variables(p);
}

CS_NORETURN void dieu(CSOUND *csound, char *s, ...)
{
    va_list args;

    csoundMessage(csound,Str("Usage:      csound [-flags] orchfile scorefile\n"));
    csoundMessage(csound,Str("Legal flags are:\n"));
    print_short_usage(csound);
    va_start(args, s);
    csoundErrMsgV(csound, Str("Csound Command ERROR:    "), s, args);
    va_end(args);
    //// FIXME This code makes no sense
    /* if (csound->info_message_request == 0) { */
    /*   csound->info_message_request = 0; */
    /*   csoundLongJmp(csound, 1); */
    /* } */
    //Added longjump -- JPff
    csoundLongJmp(csound, 1);
}

void set_output_format(OPARMS *O, char c)
{
    switch (c) {
    case 'a':
      O->outformat = AE_ALAW;    /* a-law soundfile */
      break;

    case 'c':
      O->outformat = AE_CHAR;    /* signed 8-bit soundfile */
      break;

    case '8':
      O->outformat = AE_UNCH;    /* unsigned 8-bit soundfile */
      break;

    case 'f':
      O->outformat = AE_FLOAT;   /* float soundfile */
      break;

    case 'd':
      O->outformat = AE_DOUBLE;  /* double soundfile */
      break;

    case 's':
      O->outformat = AE_SHORT;   /* short_int soundfile*/
      break;

    case 'l':
      O->outformat = AE_LONG;    /* long_int soundfile */
      break;

    case 'u':
      O->outformat = AE_ULAW;    /* mu-law soundfile */
      break;

    case '3':
      O->outformat = AE_24INT;   /* 24bit packed soundfile*/
      break;

    case 'e':
      O->outformat = AE_FLOAT;   /* float soundfile (for rescaling) */
      break;

    case 'v':
      O->outformat = AE_VORBIS;  /* Xiph Vorbis encoding */
      break;

    default:
      return; /* do nothing */
    };
}

const SAMPLE_FORMAT_ENTRY sample_format_map[] = {
  { "alaw",   'a' },  { "schar",  'c' },  { "uchar",  '8' },
  { "float",  'f' },  { "double", 'd' },  { "long",   'l' },
  { "short",  's' },  { "ulaw",   'u' },  { "24bit",  '3' },
  { "vorbis", 'v' },  { NULL, '\0' }
};

const SOUNDFILE_TYPE_ENTRY file_type_map[] = {
    { "wav",    TYP_WAV   },  { "aiff",   TYP_AIFF  },
    { "au",     TYP_AU    },  { "raw",    TYP_RAW   },
    { "paf",    TYP_PAF   },  { "svx",    TYP_SVX   },
    { "nist",   TYP_NIST  },  { "voc",    TYP_VOC   },
    { "ircam",  TYP_IRCAM },  { "w64",    TYP_W64   },
    { "mat4",   TYP_MAT4  },  { "mat5",   TYP_MAT5  },
    { "pvf",    TYP_PVF   },  { "xi",     TYP_XI    },
    { "htk",    TYP_HTK   },  { "sds",    TYP_SDS   },
    { "avr",    TYP_AVR   },  { "wavex",  TYP_WAVEX },
    { "sd2",    TYP_SD2   },  { "mpeg",   TYP_MPEG  },
    { "flac",   TYP_FLAC  },  { "caf",    TYP_CAF   },
    { "wve",    TYP_WVE   },  { "ogg",    TYP_OGG   },
    { "mpc2k",  TYP_MPC2K },  { "rf64",   TYP_RF64  },
    { NULL , -1 }
};

static int decode_long(CSOUND *csound, char *s, int argc, char **argv)
{
    OPARMS  *O = csound->oparms;
    /* Add other long options here */
    if (UNLIKELY(O->odebug))
      csoundMessage(csound, "decode_long %s\n", s);
    if (!(strncmp(s, "omacro:", 7))) {
      //if (csound->orcname_mode) return 1;
      NAMES *nn = (NAMES*) mmalloc(csound, sizeof(NAMES));
      nn->mac = s;
      nn->next = csound->omacros;
      csound->omacros = nn;
     return 1;
    }
    else if (!(strncmp(s, "smacro:", 7))) {
      //if (csound->orcname_mode) return 1;
      NAMES *nn = (NAMES*) mmalloc(csound, sizeof(NAMES));
      nn->mac = s;
      nn->next = csound->smacros;
      csound->smacros = nn;
      return 1;
    }
    else if (!(strncmp(s, "format=", 7))) {
      const SAMPLE_FORMAT_ENTRY   *sfe;
      const SOUNDFILE_TYPE_ENTRY  *ff;
      s += 7;
      do {
        char  *t;
        t = strchr(s, ':');
        if (t != NULL)
          *(t++) = '\0';
        for (ff = &(file_type_map[0]); ff->format != NULL; ff++) {
          if (strcmp(ff->format, s) == 0) {
            O->filetyp = ff->type;
            goto nxtToken;
          }
        }
        for (sfe = &(sample_format_map[0]); sfe->longformat != NULL; sfe++) {
          if (strcmp(s, sfe->longformat) == 0) {
            set_output_format(O, sfe->shortformat);
            goto nxtToken;
          }
        }
        csoundErrorMsg(csound, Str("unknown output format: '%s'"), s);
        return 0;
      nxtToken:
        s = t;
      } while (s != NULL);
      return 1;
    }
    /* -A */
    else if (!(strcmp (s, "aiff"))) {
      O->filetyp = TYP_AIFF;            /* AIFF output request */
      return 1;
    }
    else if (!(strcmp (s, "au"))) {
      O->filetyp = TYP_AU;              /* AU output request */
      return 1;
    }
    else if (!(strncmp (s, "iobufsamps=", 11))) {
      s += 11;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no iobufsamps"));
      /* defaults in musmon.c */
      O->inbufsamps = O->outbufsamps = atoi(s);
      return 1;
    }
    else if (!(strncmp (s, "hardwarebufsamps=", 17))) {
      s += 17;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no hardware bufsamps"));
      O->inbufsamps = O->outbufsamps = atoi(s);
      return 1;
   }
    else if (!(strcmp (s, "orc"))) {
      csound->use_only_orchfile = 1;    /* orchfile without scorefile */
      return 1;
    }
    else if (!(strcmp (s, "cscore"))) {
      O->usingcscore = 1;               /* use cscore processing  */
      return 1;
    }
    else if (!(strcmp (s, "nodisplays"))) {
      O->displays = 0;                  /* no func displays */
      return 1;
    }
    else if (!(strcmp (s, "displays"))) {
      O->displays = 1;                  /* func displays */
      O->graphsoff = 0;
      return 1;
    }
    else if (!(strcmp (s, "defer-gen1"))) {
      O->gen01defer = 1;                /* defer GEN01 sample loads */
      return 1;                         /*   until performance time */
    }
    else if (!(strncmp (s, "midifile=", 9))) {
      s += 9;
      if (*s==3) s++;           /* skip ETX */
      if (UNLIKELY(*s == '\0')) dieu(csound, Str("no midifile name"));
      O->FMidiname = s;                 /* Midifile name */
      if (!strcmp(O->FMidiname, "stdin")) {
#if defined(_WIN32)
        csoundDie(csound, Str("-F: stdin not supported on this platform"));
#else
        set_stdin_assign(csound, STDINASSIGN_MIDIFILE, 1);
#endif
      }
      else
        set_stdin_assign(csound, STDINASSIGN_MIDIFILE, 0);
      O->FMidiin = 1;                   /***************/
      return 1;
    }
    else if (!(strncmp (s, "midioutfile=", 12))) {
      s += 12;
      if (*s==3) s++;           /* skip ETX */
      if (UNLIKELY(*s == '\0')) dieu(csound, Str("no midi output file name"));
      O->FMidioutname = s;
      return 1;
    }
    /* -g */
    else if (!(strcmp (s, "asciidisplay"))) {
      O->graphsoff = 1;                 /* don't use graphics but ASCII */
      return 1;
    }
    /* -G */
    else if (!(strcmp (s, "postscriptdisplay"))) {
      O->postscript = 1;                /* don't use graphics but PostScript */
      return 1;
    }
    /* -h */
    else if (!(strcmp (s, "noheader"))) {
      O->filetyp = TYP_RAW;             /* RAW output request */
      return 1;
    }
    else if (!(strncmp (s, "heartbeat=", 10))) {
      s += 10;
      if (*s == '\0') O->heartbeat = 1;
      else O->heartbeat = atoi(s);
      return 1;
    }
#ifdef EMBEDDED_PYTHON
    else if (strncmp(s, "pyvar=", 6) == 0) {
      s += 6;
      if (*s==3) s++;           /* skip ETX */
      if (UNLIKELY(python_add_cmdline_definition(s)))
        dieu(csound, Str("invalid python variable definition syntax"));
      return 1;
    }
#endif
    else if (!(strncmp (s, "input=", 6))) {
      s += 6;
      if (*s==3) s++;           /* skip ETX */
      if (UNLIKELY(*s == '\0')) dieu(csound, Str("no infilename"));
      O->infilename = s;                /* soundin name */
      if (UNLIKELY(strcmp(O->infilename, "stdout") == 0))
        csoundDie(csound, Str("input cannot be stdout"));
      if (strcmp(O->infilename, "stdin") == 0) {
        set_stdin_assign(csound, STDINASSIGN_SNDFILE, 1);
#if defined(_WIN32)
        csoundDie(csound, Str("stdin audio not supported"));
#endif
      }
      else
        set_stdin_assign(csound, STDINASSIGN_SNDFILE, 0);
      O->sfread = 1;
      return 1;
    }
    /*
      -I I-time only orch run
     */
    else if (!(strcmp (s, "i-only"))) {
      csound->initonly = 1;
      O->syntaxCheckOnly = 0;           /* overrides --syntax-check-only */
      O->sfwrite = 0;                   /* and implies nosound */
      return 1;
    }
    else if (!(strcmp (s, "ircam"))) {
      O->filetyp = TYP_IRCAM;           /* IRCAM output request */
      return 1;
    }
    else if (!(strcmp (s, "ogg"))) {
      O->filetyp = TYP_OGG;             /* OGG output request   */
      O->outformat = AE_VORBIS;         /* Xiph Vorbis encoding */
      return 1;
    }
    else if (!(strcmp (s, "mpeg"))) {
      O->filetyp = TYP_MPEG;             /* OGG output request   */
      O->outformat = AE_MPEG;         /* Xiph Vorbis encoding */
      return 1;
    }
    /*
      -k N orchestra krate override
     */
    else if (!(strncmp(s, "control-rate=", 13))) {
      s += 13;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no control rate"));
      O->kr_override = (float)atof(s);
      return 1;
    }
    else if (!(strncmp(s, "ksmps=", 6))) {
      s += 6;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no ksmps"));
      O->ksmps_override = atoi(s);
      return 1;
    }
    /* -K */
    else if (!(strcmp (s, "nopeaks"))) {
      csound->peakchunks = 0;           /* Do not write peak information */
      return 1;
    }
    /*
      -L dnam read Line-oriented realtime score events from device 'dnam'
     */
    else if (!(strncmp (s, "score-in=", 9))) {
      s += 9;
      if (*s==3) s++;           /* skip ETX */
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no Linein score device_name"));
      O->Linename = s;
      if (!strcmp(O->Linename, "stdin")) {
        set_stdin_assign(csound, STDINASSIGN_LINEIN, 1);
      }
      else
        set_stdin_assign(csound, STDINASSIGN_LINEIN, 0);
      O->Linein = 1;
      return 1;
    }
    /*
      -m N tty message level.
      Sum of: 1=note amps, 2=out-of-range msg, 4=warnings
     */
    else if (!(strncmp (s, "messagelevel=", 13))) {
      s += 13;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no message level"));
      O->msglevel = atoi(s);
      return 1;
    }
    else if (!(strncmp (s, "messageolevel=", 14))) {
      s += 14;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no message level"));
      sscanf(s, "%o", &(O->msglevel));
      return 1;
    }
    else if (!(strncmp (s, "m-amps=", 7))) {
      int n;
      s += 7;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no message amps"));
      sscanf(s, "%d", &n);
      if (n) O->msglevel |= CS_AMPLMSG;
      else O->msglevel &= ~CS_AMPLMSG;
      return 1;
    }
    else if (!(strncmp (s, "m-range=",8))) {
      int n;
      s += 8;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no message range"));
      sscanf(s, "%d", &n);
      if (n) O->msglevel |= CS_RNGEMSG;
      else O->msglevel &= ~CS_RNGEMSG;
      return 1;
     }
    else if (!(strncmp (s, "m-warnings=",11))) {
      int n;
      s += 11;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no message warnings"));
      sscanf(s, "%d", &n);
      if (n) O->msglevel |= CS_WARNMSG;
      else O->msglevel &= ~CS_WARNMSG;
      return 1;
    }
    else if (!(strncmp (s, "m-raw=",6))) {
      int n;
      s += 6;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no message raw"));
      sscanf(s, "%d", &n);
      if (n) O->msglevel |= 32;
      else O->msglevel &= ~32;
      return 1;
    }
    else if (!(strncmp (s, "m-dB=",5))) {
      int n;
      s += 5;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no message dB"));
      sscanf(s, "%d", &n);
      if (n) O->msglevel |= CS_RAWMSG;
      else O->msglevel &= ~CS_RAWMSG;
      return 1;
    }
    else if (!(strncmp (s, "m-colours=",10)) || !(strncmp (s, "m-colors=",9))) {
      int n;
      s += 10;
      if (*s=='\0') dieu(csound, Str("no message colours"));
      sscanf(s, "%d", &n);
      if (n) O->msglevel |= 96;
      else O->msglevel &= ~96;
      return 1;
    }
    else if (!(strncmp (s, "m-benchmarks=",13))) {
      int n;
      s += 13;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no benchmark level"));
      sscanf(s, "%d", &n);
      if (n) O->msglevel |= CS_TIMEMSG;
      else O->msglevel &= ~CS_TIMEMSG;
      return 1;
    }
    else if (!(strncmp (s, "csd-line-nums=",14))) {
      s += 14;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no value for --csd-line-nums"));
      O->useCsdLineCounts = (atoi(s) != 0);
      return 1;
    }
    /*
      -M dnam read MIDI realtime events from device 'dnam'
     */
    else if (!(strncmp (s, "midi-device=", 12))) {
      s += 12;
      if (*s==3) s++;           /* skip ETX */
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no midi device_name"));
      O->Midiname = s;
      if (!strcmp(O->Midiname, "stdin")) {
        set_stdin_assign(csound, STDINASSIGN_MIDIDEV, 1);
#if defined(_WIN32)
        csoundDie(csound, Str("-M: stdin not supported on this platform"));
#endif
      }
      else
        set_stdin_assign(csound, STDINASSIGN_MIDIDEV, 0);
      O->Midiin = 1;
      return 1;
    }
    /* -n no sound */
    else if (!(strcmp (s, "nosound"))) {
      O->sfwrite = 0;                   /* nosound        */
      return 1;
    }
    /* -N */
    else if (!(strcmp (s, "notify"))) {
      O->ringbell = 1;                  /* notify on completion */
      return 1;
    }
    else if (!(strncmp (s, "output=", 7))) {
      s += 7;
      if (*s==3) s++;           /* skip ETX */
      if (UNLIKELY(*s == '\0')) dieu(csound, Str("no outfilename"));
      O->outfilename = s;               /* soundout name */
      if (UNLIKELY(strcmp(O->outfilename, "stdin")) == 0)
        dieu(csound, Str("-o cannot be stdin"));
      if (strcmp(O->outfilename, "stdout") == 0) {
        set_stdout_assign(csound, STDOUTASSIGN_SNDFILE, 1);
#if defined(_WIN32)
        csoundDie(csound, Str("stdout audio not supported"));
#endif
      }
      else
        set_stdout_assign(csound, STDOUTASSIGN_SNDFILE, 0);
      O->sfwrite = 1;
      return 1;
    }
    else if (!(strcmp (s, "print_version"))) {
      csound->print_version = 1;
      return 1;
    }
    else if (!(strncmp (s, "logfile=", 8))) {
      s += 8;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no log file"));
      do_logging(s);
      return 1;
    }
    /* -r N */
    else if (!(strncmp (s, "sample-rate=", 12))) {
      s += 12;
      if (*s==3) s++;           /* skip ETX */
      O->sr_override = (float)atof(s);
      return 1;
    }
    /* R */
    else if (!(strcmp (s, "rewrite"))) {
      O->rewrt_hdr = 1;
      return 1;
    }
    /* -S  */
    /* tempo=N use uninterpreted beats of the score, initially at tempo N
     */
    else if (UNLIKELY(!(strncmp (s, "tempo=", 6)))) {
      s += 6;
      O->cmdTempo = atof(s);
      O->Beatmode = 1;                  /* on uninterpreted Beats */
      return 1;
    }
    /* -t0 */
    else if (!(strncmp(s, "keep-sorted-score=", 18))) {
      s += 18;
      csound->score_srt = s;
      csound->keep_tmp= 1;
      return 1;
    }
    else if (!(strcmp (s, "keep-sorted-score"))) {
      csound->keep_tmp= 1;
      return 1;
    }
    else if (!(strncmp (s, "simple-sorted-score=", 20))) {
      s +=20;
      csound->score_srt = s;
      csound->keep_tmp = 2;
      return 1;
    }
    else if (!(strcmp (s, "simple-sorted-score"))) {
      csound->keep_tmp = 2;
      return 1;
    }
    /* IV - Jan 27 2005: --expression-opt */
    /* NOTE these do nothing */
    else if (!(strcmp (s, "expression-opt"))) {
      //O->expr_opt = 1;
      csoundWarning(csound, Str("option expresson-opt has no affect\n"));
      return 1;
    }
    else if (!(strcmp (s, "no-expression-opt"))) {
      //O->expr_opt = 0;
      csoundWarning(csound, Str("option no-expresson-opt has no affect\n"));
      return 1;
    }
    else if (!(strncmp (s, "env:", 4))) {
      if (csoundParseEnv(csound, s + 4) == CSOUND_SUCCESS)
        return 1;
      else
        return 0;
    }
    else if (!(strncmp (s, "strset", 6))) {
      strset_option(csound, s + 6);
      return 1;
    }
    /* -T terminate the performance when miditrack is done */
    else if (!(strcmp (s, "terminate-on-midi"))) {
      O->termifend = 1;                 /* terminate on midifile end */
      return 1;
    }
    else if (!(strncmp (s, "utility=", 8))) {
      int retval;
      s += 8;
      if (*s==3) s++;           /* skip ETX */
      if (*s=='\0') dieu(csound, Str("no utility name"));

      retval = csoundRunUtility(csound, s, argc, argv);
      if (retval) {
        csound->info_message_request = 1;
        csound->orchname = NULL;
        return 0;
      }
      else csoundLongJmp(csound, retval);
     return 1;
    }
    /* -v */
    else if (!(strcmp (s, "verbose"))) {
      O->odebug = 1;                    /* verbose otran  */
      return 1;
    }
    /* -x fnam extract from score.srt using extract file 'fnam' */
    else if (!(strncmp(s, "extract-score=", 14))) {
      s += 14;
      if (UNLIKELY(*s=='\0')) dieu(csound, Str("no xfilename"));
      csound->xfilename = s;
      return 1;
    }
    else if (!(strcmp(s, "wave"))) {
      O->filetyp = TYP_WAV;             /* WAV output request */
      return 1;
    }
    else if (!(strncmp (s, "list-opcodes", 12))) {
      int full = 0;
      s += 12;

      if (*s != '\0') {
        if (isdigit(*s)) full = *s++ - '0';
      }
      list_opcodes(csound, full);
      return 1;
      //csoundLongJmp(csound, 0);
    }
    /* -Z */
    else if (!(strcmp (s, "dither"))) {
      csound->dither_output = 1;
      return 1;
    }
    else if (!(strcmp (s, "dither-uniform"))) {
      csound->dither_output = 2;
      return 1;
    }
    else if (!(strcmp (s, "dither-triangular"))) {
      csound->dither_output = 1;
      return 1;
    }
    else if (!(strncmp (s, "midi-key=", 9))) {
      s += 9;
      O->midiKey = atoi(s);
      return 1;
    }
    else if (!(strncmp (s, "midi-key-cps=", 13))) {
      s += 13 ;
      O->midiKeyCps = atoi(s);
      return 1;
    }
    else if (!(strncmp (s, "midi-key-oct=", 13))) {
      s += 13 ;
      O->midiKeyOct = atoi(s);
      return 1;
    }
    else if (!(strncmp (s, "midi-key-pch=", 13))) {
      s += 13 ;
      O->midiKeyPch = atoi(s);
      return 1;
    }
    else if (!(strncmp (s, "midi-velocity=", 14))) {
      s += 14;
      O->midiVelocity = atoi(s);
      return 1;
    }
    else if (!(strncmp (s, "midi-velocity-amp=", 18))) {
      s += 18;
      O->midiVelocityAmp = atoi(s);
      return 1;
    }
    else if (!(strncmp (s, "opcode-dir=", 11))){
        s += 11;
        csoundLoadPlugins(csound, s);
        return 1;
    }
    else if (!(strncmp (s, "opcode-lib=", 11))) {
      int   nbytes;
      s += 11;
      if (*s==3) s++;           /* skip ETX */
      nbytes = (int) strlen(s) + 1;
      if (csound->dl_opcodes_oplibs == NULL) {
        /* start new library list */
        csound->dl_opcodes_oplibs = (char*) mmalloc(csound, (size_t) nbytes);
        strcpy(csound->dl_opcodes_oplibs, s);
      }
      else {
        /* append to existing list */
        nbytes += ((int) strlen(csound->dl_opcodes_oplibs) + 1);
        csound->dl_opcodes_oplibs = (char*) mrealloc(csound,
                                                     csound->dl_opcodes_oplibs,
                                                     (size_t) nbytes);
        strcat(csound->dl_opcodes_oplibs, ",");
        strcat(csound->dl_opcodes_oplibs, s);
      }
      return 1;
    }
    else if (!(strcmp(s, "default-paths"))) {
      O->noDefaultPaths = 0;
      return 1;
    }
    else if (!(strcmp(s, "no-default-paths"))) {
      O->noDefaultPaths = 1;
      return 1;
    }
    else if (!(strncmp (s, "num-threads=", 12))) {
      s += 12 ;
      O->numThreads = atoi(s);
      return 1;
    }
    else if (!(strcmp (s, "syntax-check-only"))) {
      O->syntaxCheckOnly = 1;
      return 1;
    }
    else if (!(strcmp(s, "version"))) {
      print_csound_version(csound);
      csoundLongJmp(csound, 0);
    }
    else if (!(strcmp(s, "help"))) {
      longusage(csound);
      csound->info_message_request = 1;
      return 1;
      //csoundLongJmp(csound, 0);
    }
    else if (!(strcmp(s, "sample-accurate"))) {
      O->sampleAccurate = 1;
      return 1;
    }
    else if (!(strcmp(s, "realtime"))) {
      csoundMessage(csound, Str("realtime mode enabled\n"));
      O->realtime = 1;
      return 1;
    }
    else if (!(strncmp(s, "nchnls=", 7))) {
      s += 7;
      O->nchnls_override = atoi(s);
      return 1;
    }
    else if (!(strncmp(s, "0dbfs=", 6))) {
      s += 6;
      O->e0dbfs_override = atoi(s);
      return 1;
    }
    else if (!(strncmp(s, "nchnls_i=", 9))) {
      s += 9;
      O->nchnls_i_override = atoi(s);
      return 1;
    }
    else if (!(strncmp(s, "sinesize=", 9))) {
      {
        int i = 1, n;
        s += 9;
        n = atoi(s);
        while (i<=n && i< MAXLEN) i <<= 1;
        csound->sinelength = i;
        return 1;
      }
    }
    else if (!(strcmp(s, "new-parser")) ||
             !(strcmp(s, "old-parser"))) {
      return 1;  /* ignore flag, this is here for backwards compatibility */
    }
    else if (!(strcmp(s, "sco-parser"))) {
      csound->score_parser = 1;
      return 1;  /* Try new parser */
    }
    else if (!(strcmp(s, "daemon"))) {
      if(O->daemon == 0) O->daemon = 1;
      return 1;
    }
    else if (!(strncmp(s, "port=",5))) {
      s += 5;
      O->daemon = atoi(s);
      return 1;
    }
    else if (!(strncmp(s, "udp-echo",8))) {
      //s += 8;
      O->echo = 1;
      return 1;
    }
    else if (!(strncmp(s, "udp-console=",12))) {
      char *ports;
      s += 12;
      ports = strchr(s, ':');
      if(*s != '\0' && ports != NULL) {
        *ports = '\0';
        csoundUDPConsole(csound, s, atoi(ports+1),0);
      } else
        csoundWarning(csound, "UDP console: needs address and port\n");
      return 1;
    }
    else if (!(strncmp(s, "udp-mirror-console=",19))) {
      char *ports;
      s += 19;
      ports = strchr(s, ':');
      if(*s != '\0' && ports != NULL) {
        *ports = '\0';
        csoundUDPConsole(csound, s, atoi(ports+1),1);
      } else
        csoundWarning(csound, "UDP console: needs address and port\n");
      return 1;
    }
    else if (!(strncmp(s, "fftlib=",7))) {
      s += 7;
      O->fft_lib = atoi(s);
      return 1;
    }
    else if (!(strncmp(s, "vbr-quality=",12))) {
      s += 12;
      O->quality = atof(s);
      return 1;
      }
    else if (!(strncmp(s, "devices",7))) {
      csoundLoadExternals(csound);
      if (csoundInitModules(csound) != 0)
        csoundLongJmp(csound, 1);
      if (*(s+7) == '='){
        if (!strncmp(s+8,"in", 2)) {
          list_audio_devices(csound, 0);
        }
        else if (!strncmp(s+8,"out", 2))
          list_audio_devices(csound,1);
      }
      else {
        list_audio_devices(csound,0);
        list_audio_devices(csound,1);
      }
      csound->info_message_request = 1;
      return 1;
      }
     else if (!(strncmp(s, "midi-devices",12))) {
      csoundLoadExternals(csound);
      if (csoundInitModules(csound) != 0)
        csoundLongJmp(csound, 1);
      if (*(s+12) == '='){
        if (!strncmp(s+13,"in", 2)) {
          list_midi_devices(csound, 0);
        }
        else if (!strncmp(s+13,"out", 2))
          list_midi_devices(csound,1);
      }
      else {
        list_midi_devices(csound,0);
        list_midi_devices(csound,1);
      }
      csound->info_message_request = 1;
      return 1;
      }
    else if (!(strncmp(s, "get-system-sr",13)) ){
      if (O->outfilename &&
          !(strncmp(O->outfilename, "dac",3))) {
        /* these are default values to get the
           backend to open successfully */
        set_output_format(O, 'f');
        O->sr_override = FL(-1.0);
        O->inbufsamps = O->outbufsamps = 256;
        O->oMaxLag = 1024;
        csoundLoadExternals(csound);
        if (csoundInitModules(csound) != 0)
          csoundLongJmp(csound, 1);
        sfopenout(csound);
        csoundMessageS(csound, CSOUNDMSG_STDOUT,
                         "system sr: %f\n", csoundSystemSr(csound,0));
        sfcloseout(csound);
        // csoundLongJmp(csound, 0);
      }
      csound->info_message_request = 1;
      return 1;
    }
    else if(!strncmp(s, "use-system-sr",13)) {
      if (O->sr_override == FL(0.0))
          O->sr_override = FL(-1.0);
      return 1;
    }
    else if(!strncmp(s, "default-ksmps=",14)) {
      s += 14;
      O->kr_default = O->sr_default/atof(s);
      return 1;
    }

    else if (!(strcmp(s, "aft-zero"))) {
      csound->aftouch = 0;
      return 1;
    }
    else if (!(strncmp(s, "limiter=", 8)))  {
      s += 8;
      O->limiter = atof(s);
      if (O->limiter>1.0 || O->limiter<0) {
        csoundMessageS(csound, CSOUNDMSG_STDOUT,
                         Str("Ignoring invalid limiter\n"));
        O->limiter = 0;
      }
      return 1;

    }
    else if (!(strcmp(s, "limiter"))) {
      O->limiter = 0.5;
      return 1;
    }
     else if (!(strcmp(s, "vbr"))) {
  #ifdef SNDFILE_MP3    
      O->mp3_mode = SF_BITRATE_MODE_VARIABLE;
  #endif    
      return 1;
    }
    csoundErrorMsg(csound, Str("unknown long option: '--%s'"), s);
    return 0;
}

PUBLIC int argdecode(CSOUND *csound, int argc, const char **argv_)
{
    OPARMS  *O = csound->oparms;
    char    *s, **argv;
    int     n;
    char    c;

    /* make a copy of the option list */
    char  *p1, *p2;
    int   nbytes, i;
    /* calculate the number of bytes to allocate */
    /* N.B. the argc value passed to argdecode is decremented by one */
    nbytes = (argc + 1) * (int) sizeof(char*);
    for (i = 0; i <= argc; i++)
      nbytes += ((int) strlen(argv_[i]) + 1);
    p1 = (char*) mmalloc(csound, nbytes); /* will be freed by memRESET() */
    p2 = (char*) p1 + ((int) sizeof(char*) * (argc + 1));
    argv = (char**) p1;
    for (i = 0; i <= argc; i++) {
      argv[i] = p2;
      strcpy(p2, argv_[i]);
      p2 = (char*) p2 + ((int) strlen(argv_[i]) + 1);
    }
    //<csound->keep_tmp = 0;

    do {
      s = *++argv;
      if (*s++ == '-') {                  /* read all flags:  */
        while ((c = *s++) != '\0') {
          switch (c) {
          case 'U':
            FIND(Str("no utility name"));
            {
              int retval = csoundRunUtility(csound, s, argc, argv);
              if (retval) {
                  csound->info_message_request = 1;
                  csound->orchname = NULL;
                  goto end;
              }
              else csoundLongJmp(csound, retval);
            }
            break;
          case 'C':
            O->usingcscore = 1;           /* use cscore processing  */
            break;
          case 'I':
            csound->initonly = 1;         /* I-only overrides */
            O->syntaxCheckOnly = 0;       /* --syntax-check-only and implies */
            /* FALLTHRU */
          case 'n':
            O->sfwrite = 0;               /* nosound        */
            break;
          case 'i':
            FIND(Str("no infilename"));
            O->infilename = s;            /* soundin name */
            s += (int) strlen(s);
            if (UNLIKELY(strcmp(O->infilename, "stdout")) == 0)
              csoundDie(csound, Str("input cannot be stdout"));
            if (strcmp(O->infilename, "stdin") == 0) {
              set_stdin_assign(csound, STDINASSIGN_SNDFILE, 1);
#if defined(_WIN32)
              csoundDie(csound, Str("stdin audio not supported"));
#endif
            }
            else
              set_stdin_assign(csound, STDINASSIGN_SNDFILE, 0);
            O->sfread = 1;
            break;
          case 'o':
            FIND(Str("no outfilename"));
            O->outfilename = s;           /* soundout name */
            s += (int) strlen(s);
            if (UNLIKELY(strcmp(O->outfilename, "stdin") == 0))
              dieu(csound, Str("-o cannot be stdin"));
            if (strcmp(O->outfilename, "stdout") == 0) {
              set_stdout_assign(csound, STDOUTASSIGN_SNDFILE, 1);
#if defined(_WIN32)
              csoundDie(csound, Str("stdout audio not supported"));
#endif
            }
            else
              set_stdout_assign(csound, STDOUTASSIGN_SNDFILE, 0);
            O->sfwrite = 1;
            break;
          case 'b':
            FIND(Str("no iobufsamps"));
            sscanf(s, "%d%n", &(O->outbufsamps), &n);
            /* defaults in musmon.c */
            O->inbufsamps = O->outbufsamps;
            s += n;
            break;
          case 'B':
            FIND(Str("no hardware bufsamps"));
            sscanf(s, "%d%n", &(O->oMaxLag), &n);
            /* defaults in rtaudio.c */
            s += n;
            break;
          case 'A':
            O->filetyp = TYP_AIFF;        /* AIFF output request*/
            break;
          case 'J':
            O->filetyp = TYP_IRCAM;       /* IRCAM output request */
            break;
          case 'W':
            O->filetyp = TYP_WAV;         /* WAV output request */
            break;
          case 'h':
            O->filetyp = TYP_RAW;         /* RAW output request */
            break;
          case 'c':
          case 'a':
          case 'u':
          case '8':
          case 's':
          case '3':
          case 'l':
          case 'f':
            set_output_format(O, c);
            break;
          case 'r':
            FIND(Str("no sample rate"));
            O->sr_override = (float)atof(s);
            while (*++s);
            break;
          case 'k':
            FIND(Str("no control rate"));
            O->kr_override = (float)atof(s);
            while (*++s);
            break;
          case 'v':
            O->odebug = 1;                /* verbose otran  */
            break;
          case 'm':
            FIND(Str("no message level"));
            sscanf(s, "%d%n", &(O->msglevel), &n);
            s += n;
            break;
          case 'd':
            O->displays = 0;              /* no func displays */
            break;
          case 'g':
            O->graphsoff = 1;             /* don't use graphics */
            break;
          case 'G':
            O->postscript = 1;            /* Postscript graphics*/
            break;
          case 'x':
            FIND(Str("no xfilename"));
            csound->xfilename = s;        /* extractfile name */
            while (*++s);
            break;
          case 't':
            FIND(Str("no tempo value"));
            {
              double val;
              sscanf(s, "%lg%n", &val, &n); /* use this tempo .. */
              s += n;
              if (UNLIKELY(val < 0.0)) dieu(csound, Str("illegal tempo"));
              else if (val == 0.0) {
                csound->keep_tmp = 1;
                break;
              }
              O->cmdTempo = val;
              O->Beatmode = 1;            /* on uninterpreted Beats */
            }
            break;
          case 'L':
            FIND(Str("no Linein score device_name"));
            O->Linename = s;              /* Linein device name */
            s += (int) strlen(s);
            if (!strcmp(O->Linename, "stdin")) {
              set_stdin_assign(csound, STDINASSIGN_LINEIN, 1);
            }
            else
              set_stdin_assign(csound, STDINASSIGN_LINEIN, 0);
            O->Linein = 1;
            break;
          case 'M':
            FIND(Str("no midi device_name"));
            O->Midiname = s;              /* Midi device name */
            s += (int) strlen(s);
            if (strcmp(O->Midiname, "stdin")==0) {
#if defined(_WIN32)
              csoundDie(csound, Str("-M: stdin not supported on this platform"));
#else
              set_stdin_assign(csound, STDINASSIGN_MIDIDEV, 1);
#endif
            }
            else
              set_stdin_assign(csound, STDINASSIGN_MIDIDEV, 0);
            O->Midiin = 1;
            break;
          case 'F':
            FIND(Str("no midifile name"));
            O->FMidiname = s;             /* Midifile name */
            s += (int) strlen(s);
            if (strcmp(O->FMidiname, "stdin")==0) {
#if defined(_WIN32)
              csoundDie(csound, Str("-F: stdin not supported on this platform"));
#else
              set_stdin_assign(csound, STDINASSIGN_MIDIFILE, 1);
#endif
            }
            else
              set_stdin_assign(csound, STDINASSIGN_MIDIFILE, 0);
            O->FMidiin = 1;               /*****************/
            break;
          case 'Q':
            FIND(Str("no MIDI output device"));
            O->Midioutname = s;
            s += (int) strlen(s);
            break;
          case 'R':
            O->rewrt_hdr = 1;
            break;
          case 'H':
            if (isdigit(*s)) {
              sscanf(s, "%d%n", &(O->heartbeat), &n);
              s += n;
            }
            else O->heartbeat = 1;
            break;
          case 'N':
            O->ringbell = 1;              /* notify on completion */
            break;
          case 'T':
            O->termifend = 1;             /* terminate on midifile end */
            break;
          case 'D':
            O->gen01defer = 1;            /* defer GEN01 sample loads
                                             until performance time */
            break;
          case 'K':
            csound->peakchunks = 0;       /* Do not write peak information */
            break;
          case 'z':
            {
              int full = 0;
              if (*s != '\0') {
                if (isdigit(*s)) full = *s++ - '0';
              }
              list_opcodes(csound, full);
            }
            csound->info_message_request = 1;
            // csoundLongJmp(csound, 0);
            break;
          case 'Z':
            {
              int full = 1;
              if (*s != '\0') {
                if (isdigit(*s)) full = *s++ - '0';
              }
              csound->dither_output = full;
            }
            break;
          case '@':
            FIND(Str("No indirection file"));
            {
              FILE *ind;
              void *fd;
              fd = csoundFileOpenWithType(csound, &ind, CSFILE_STD,
                                     s, "r", NULL, CSFTYPE_OPTIONS, 0);
              if (UNLIKELY(fd == NULL)) {
                dieu(csound, Str("Cannot open indirection file %s\n"), s);
              }
              else {
                CORFIL *cf = copy_to_corefile(csound, s, NULL, 0);
                readOptions(csound, cf, 0);
                corfile_rm(csound, &cf);
                csoundFileClose(csound, fd);
              }
              while (*s++) {};
              s--; /* semicolon on separate line to silence warning */
            }
            break;
          case 'O':
            FIND(Str("no log file"));
            do_logging(s);
            while (*s++) {};
            s--; /* semicolon on separate line to silence warning */
            break;
          case '-':
#if defined(__gnu_linux__)
            if (!(strcmp (s, "sched"))) {             /* ignore --sched */
              while (*(++s));
              break;
            }
            if (!(strncmp(s, "sched=", 6))) {
              while (*(++s));
              break;
            }
#endif
            if (!decode_long(csound, s, argc, argv))
              csoundLongJmp(csound, 1);
            while (*(++s));
            break;
          case 'j':
            FIND(Str("no number of threads"));
            sscanf(s, "%d%n", &(O->numThreads), &n);
            s += n;
            break;
          case '+':                                   /* IV - Feb 01 2005 */
            if (parse_option_as_cfgvar(csound, (char*) s - 2) != 0)
              csoundLongJmp(csound, 1);
            while (*(++s));
            break;
          default:
            if (csound->info_message_request == 0)
            dieu(csound, Str("unknown flag -%c"), c);
          }
        }
      }
      else {
        /* 0: normal, 1: ignore, 2: fail */
        if (csound->orcname_mode == 2) {
          csoundDie(csound, Str("error: orchestra and score name not "
                                  "allowed in .csound7rc"));
        }
        else if (csound->orcname_mode == 0) {
          if (csound->orchname == NULL) /* VL dec 2016: better duplicate these */
            csound->orchname = cs_strdup(csound, --s);
          else if (LIKELY(csound->scorename == NULL))
            csound->scorename = cs_strdup(csound, --s);
          else {
            csoundMessage(csound,"argc=%d Additional string \"%s\"\n",argc,--s);
            dieu(csound, Str("too many arguments"));
          }
        }
      }
    } while (--argc);
 end:
    return 1;
}

static void list_audio_devices(CSOUND *csound, int output){

    int i,n = csoundGetAudioDevList(csound,NULL, output);
    CS_AUDIODEVICE *devs = (CS_AUDIODEVICE *)
      mmalloc(csound, n*sizeof(CS_AUDIODEVICE));
    if (output)
      csoundMessage(csound, Str("%d audio output devices\n"), n);
    else
      csoundMessage(csound, Str("%d audio input devices\n"), n);
    csoundGetAudioDevList(csound,devs,output);
    for (i=0; i < n; i++) {
      int nchnls = devs[i].max_nchnls;
      if(nchnls > 0)
        csoundMessage(csound, " %d: %s (%s) [ch:%d]\n",
                        i, devs[i].device_id, devs[i].device_name, nchnls);
      else
        csoundMessage(csound, " %d: %s (%s)\n",
                        i, devs[i].device_id, devs[i].device_name);
    }
    mfree(csound, devs);
}

static void list_midi_devices(CSOUND *csound, int output){

    int i,n = csoundGetMIDIDevList(csound,NULL, output);
    CS_MIDIDEVICE *devs =
      (CS_MIDIDEVICE *) mmalloc(csound, n*sizeof(CS_MIDIDEVICE));
    if (output)
      csoundMessage(csound, Str("%d MIDI output devices\n"), n);
    else
      csoundMessage(csound, Str("%d MIDI input devices\n"), n);
    csoundGetMIDIDevList(csound,devs,output);
    for (i=0; i < n; i++)
      csoundMessage(csound, " %d: %s (%s)\n",
                      i, devs[i].device_id, devs[i].device_name);
    mfree(csound, devs);
}
