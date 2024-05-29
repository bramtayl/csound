#include "csoundCore_internal.h"
#include "argdecode.h"
#include "argdecode_internal.h"
#include "memalloc.h"
#include "csound_orc_semantics_public.h"
#include "envvar_public.h"
#include "opcode_internal.h"
#include "one_file.h"
#include "libsnd_internal.h"
#include "csmodule.h"

PUBLIC int csoundSetOption(CSOUND *csound, const char *option){
    /* if already compiled and running, return */
    if (csound->engineStatus & CS_STATE_COMP) return 1;
    else {
    const char *args[2] = {"csound", option};
    csound->info_message_request = 1;
    return (argdecode(csound, 1, args) ? 0 : 1);
    }
}

PUBLIC void csoundSetParams(CSOUND *csound, CSOUND_PARAMS *p){
    OPARMS *oparms = csound->oparms;
    /* if already compiled and running, return */
    if (csound->engineStatus & CS_STATE_COMP) return;

    /* simple ON/OFF switches */
    oparms->odebug = p->debug_mode;
    oparms->displays = p->displays;
    oparms->graphsoff = p->ascii_graphs;
    oparms->postscript = p->postscript_graphs;
    oparms->usingcscore = p->use_cscore;
    oparms->ringbell = p->ring_bell;
    oparms->gen01defer = p->defer_gen01_load;
    oparms->termifend =  p->terminate_on_midi;
    oparms->noDefaultPaths = p->no_default_paths;
    oparms->syntaxCheckOnly = p->syntax_check_only;
    oparms->sampleAccurate = p->sample_accurate;
    oparms->realtime = p->realtime_mode;
    oparms->useCsdLineCounts = p->csd_line_counts;
    oparms->heartbeat = p->heartbeat;
    oparms->ringbell = p->ring_bell;
    oparms->daemon = p->daemon;

    /* message level */
    if (p->message_level > 0)
      oparms->msglevel = p->message_level;

    /* tempo / beatmode */
    if (p->tempo > 0) {
      oparms->cmdTempo = p->tempo;
      oparms->Beatmode = 1;
    }
    /* buffer frames */
    if (p->buffer_frames > 0)
      oparms->inbufsamps = oparms->outbufsamps = p->buffer_frames;

    /* hardware buffer frames */
    if (p->hardware_buffer_frames > 0)
      oparms->oMaxLag = p->hardware_buffer_frames;

    /* multicore threads */
    if (p->number_of_threads > 1)
      oparms->numThreads = p->number_of_threads;

    /* MIDI interop */
    if (p->midi_key > 0) oparms->midiKey = p->midi_key;
    else if (p->midi_key_cps > 0) oparms->midiKeyCps = p->midi_key_cps;
    else if (p->midi_key_pch > 0) oparms->midiKeyPch = p->midi_key_pch;
    else if (p->midi_key_oct > 0) oparms->midiKeyOct = p->midi_key_oct;

    if (p->midi_velocity > 0) oparms->midiVelocity = p->midi_velocity;
    else if (p->midi_velocity_amp > 0)
      oparms->midiVelocityAmp = p->midi_velocity_amp;

    /* CSD line counts */
    if (p->csd_line_counts > 0) oparms->useCsdLineCounts = p->csd_line_counts;

    /* kr override */
    if (p->control_rate_override > 0)
      oparms->kr_override = p->control_rate_override;

    /* sr override */
    if (p->sample_rate_override > 0)
      oparms->sr_override = p->sample_rate_override;

    oparms->nchnls_override = p->nchnls_override;
    oparms->nchnls_i_override = p->nchnls_i_override;
    oparms->e0dbfs_override = p->e0dbfs_override;

    if (p->ksmps_override > 0) oparms->ksmps_override = p->ksmps_override;
}

PUBLIC void csoundGetParams(CSOUND *csound, CSOUND_PARAMS *p){

    OPARMS *oparms = csound->oparms;

    p->debug_mode = oparms->odebug;
    p->displays = oparms->displays;
    p->ascii_graphs = oparms->graphsoff;
    p->postscript_graphs = oparms->postscript;
    p->use_cscore = oparms->usingcscore;
    p->ring_bell = oparms->ringbell;
    p->defer_gen01_load = oparms->gen01defer;
    p->terminate_on_midi = oparms->termifend;
    p->no_default_paths = oparms->noDefaultPaths;
    p->syntax_check_only = oparms->syntaxCheckOnly;
    p->sample_accurate = oparms->sampleAccurate;
    p->realtime_mode = oparms->realtime;
    p->message_level = oparms->msglevel;
    p->tempo = oparms->cmdTempo;
    p->buffer_frames = oparms->outbufsamps;
    p->hardware_buffer_frames = oparms->oMaxLag;
    p->number_of_threads = oparms->numThreads;
    p->midi_key = oparms->midiKey;
    p->midi_key_cps = oparms->midiKeyCps;
    p->midi_key_pch = oparms->midiKeyPch;
    p->midi_key_oct = oparms->midiKeyOct;
    p->midi_velocity = oparms->midiVelocity;
    p->midi_velocity_amp = oparms->midiVelocityAmp;
    p->csd_line_counts = oparms->useCsdLineCounts;
    p->control_rate_override = oparms->kr_override;
    p->sample_rate_override = oparms->sr_override;
    p->nchnls_override = oparms->nchnls_override;
    p->nchnls_i_override = oparms->nchnls_i_override;
    p->e0dbfs_override = oparms->e0dbfs_override;
    p->heartbeat = oparms->heartbeat;
    p->ring_bell = oparms->ringbell;
    p->daemon = oparms->daemon;
    p->ksmps_override = oparms->ksmps_override;
    p->FFT_library = oparms->fft_lib;
}

PUBLIC void csoundSetOutput(CSOUND *csound, const char *name,
                            const char *type, const char *format)
{

    OPARMS *oparms = csound->oparms;
    char *typename;

    /* if already compiled and running, return */
    if (csound->engineStatus & CS_STATE_COMP) return;

    oparms->outfilename =
      mmalloc(csound, strlen(name) + 1); /* will be freed by memRESET */
    strcpy(oparms->outfilename, name); /* unsafe -- REVIEW */
    if (strcmp(oparms->outfilename, "stdout") == 0) {
      set_stdout_assign(csound, STDOUTASSIGN_SNDFILE, 1);
#if defined(_WIN32)
      csoundWarning(csound, Str("stdout not supported on this platform"));
#endif
    }
    else set_stdout_assign(csound, STDOUTASSIGN_SNDFILE, 0);

    oparms->sfwrite = 1;
    if (type != NULL) {
      int i=0;
      while ((typename = file_type_map[i].format) != NULL) {
        if (!strcmp(type,typename)) break;
        i++;
      }
      if (typename != NULL) {
        oparms->filetyp = file_type_map[i].type;
      }
    }
    if (format != NULL) {
      int i=0;
      while ((typename = sample_format_map[i].longformat) != NULL) {
        if (!strcmp(format,typename)) break;
        i++;
      }
      if (format != NULL) {
        set_output_format(oparms, sample_format_map[i].shortformat);
      }
    }
}

const char* get_output_format(OPARMS *O)
{
  int i = 0;
  char c;
    switch (O->outformat) {
    case  AE_ALAW:
      c  = 'a';
      break;
    case AE_CHAR:
      c  = 'c';
      break;
    case AE_UNCH:
      c  = '8';
      break;
    case AE_FLOAT:
      c  = 'f';
      break;
    case AE_DOUBLE:
      c  = 'd';
      break;
    case AE_SHORT:
      c  = 's';
      break;
    case AE_LONG:
      c  = 'l';
      break;
    case AE_ULAW:
      c  = 'u';
      break;
    case AE_24INT:
      c  = '3';
      break;
    case AE_VORBIS:
      c  = 'v';
      break;
    default:
      c = '\0';
    };
    while(c != sample_format_map[i].shortformat  &&
          sample_format_map[i].longformat != NULL) {
      i++;
    }
    return sample_format_map[i].longformat;
}

PUBLIC void csoundGetOutputFormat(CSOUND *csound,
                                  char *type, char *format)
{

    OPARMS *oparms = csound->oparms;
    int i = 0;
    const char* fmt = get_output_format(oparms);
    while (file_type_map[i].type != oparms->filetyp  &&
           file_type_map[i].format  != NULL) i++;
    if(file_type_map[i].format != NULL)
      strcpy(type,file_type_map[i].format);
    else
      strcpy(type,"");
    if(fmt != NULL)
      strcpy(format, fmt);
    else
      strcpy(format,"");
}


PUBLIC void csoundSetInput(CSOUND *csound, const char *name) {
    OPARMS *oparms = csound->oparms;

    /* if already compiled and running, return */
    if (csound->engineStatus & CS_STATE_COMP) return;

    oparms->infilename =
      mmalloc(csound, strlen(name)); /* will be freed by memRESET */
    strcpy(oparms->infilename, name);
    if (strcmp(oparms->infilename, "stdin") == 0) {
      set_stdin_assign(csound, STDINASSIGN_SNDFILE, 1);
#if defined(_WIN32)
      csoundWarning(csound, Str("stdin not supported on this platform"));
#endif
    }
    else
      set_stdin_assign(csound, STDINASSIGN_SNDFILE, 0);
    oparms->sfread = 1;
}

PUBLIC void csoundSetMIDIInput(CSOUND *csound, const char *name) {
    OPARMS *oparms = csound->oparms;

    /* if already compiled and running, return */
    if (csound->engineStatus & CS_STATE_COMP) return;

    oparms->Midiname =
      mmalloc(csound, strlen(name)); /* will be freed by memRESET */
    strcpy(oparms->Midiname, name);
    if (!strcmp(oparms->Midiname, "stdin")) {
      set_stdin_assign(csound, STDINASSIGN_MIDIDEV, 1);
#if defined(_WIN32)
      csoundWarning(csound, Str("stdin not supported on this platform"));
#endif
    }
    else
      set_stdin_assign(csound, STDINASSIGN_MIDIDEV, 0);
    oparms->Midiin = 1;
}



PUBLIC void csoundSetMIDIFileInput(CSOUND *csound, const char *name) {
    OPARMS *oparms = csound->oparms;

    /* if already compiled and running, return */
    if (csound->engineStatus & CS_STATE_COMP) return;

    oparms->FMidiname =
      mmalloc(csound, strlen(name)); /* will be freed by memRESET */
    strcpy(oparms->FMidiname, name);
    if (!strcmp(oparms->FMidiname, "stdin")) {
      set_stdin_assign(csound, STDINASSIGN_MIDIFILE, 1);
#if defined(_WIN32)
      csoundWarning(csound, Str("stdin not supported on this platform"));
#endif
    }
    else
      set_stdin_assign(csound, STDINASSIGN_MIDIFILE, 0);
    oparms->FMidiin = 1;
}

PUBLIC void csoundSetMIDIOutput(CSOUND *csound, const char *name) {
    OPARMS *oparms = csound->oparms;

    /* if already compiled and running, return */
    if (csound->engineStatus & CS_STATE_COMP) return;

    oparms->Midioutname =
      mmalloc(csound, strlen(name)); /* will be freed by memRESET */
    strcpy(oparms->Midioutname, name);
}

PUBLIC void csoundSetMIDIFileOutput(CSOUND *csound, const char *name) {
    OPARMS *oparms = csound->oparms;

    /* if already compiled and running, return */
    if (csound->engineStatus & CS_STATE_COMP) return;

    oparms->FMidioutname =
      mmalloc(csound, strlen(name)); /* will be freed by memRESET */
    strcpy(oparms->FMidioutname, name);
}