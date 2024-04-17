/*
  rtauhal.c:

  Copyright (C) 2011 Victor Lazzarini

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

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/AudioHardware.h>
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
#include <stdint.h>
#include "csdl.h"
#include "soundio.h"
#include "memalloc.h"

/* Modified from BSD sources for strlcpy */
/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
/* modifed for speed -- JPff */
char *
strNcpy(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit or until NULL */
    if (n != 0) {
      while (--n != 0) {
        if ((*d++ = *s++) == '\0')
          break;
      }
    }

    /* Not enough room in dst, add NUL */
    if (n == 0) {
      if (siz != 0)
        *d = '\0';                /* NUL-terminate dst */

      //while (*s++) ;
    }
    return dst;        /* count does not include NUL */
}

#if !defined(MAC_OS_X_VERSION_10_6)
/* the API was changed for 10.6, these make it backwards compatible  */
typedef ComponentInstance AudioComponentInstance;
typedef Component AudioComponent;
typedef ComponentDescription AudioComponentDescription;
#define AudioComponentFindNext FindNextComponent
#define AudioComponentInstanceNew OpenAComponent
#define  AudioComponentInstanceDispose CloseComponent
typedef float AudioUnitSampleType;
#endif


typedef struct {
  char name[128];
  int outchannels;
  int inchannels;
  int indevnum;
  int outdevnum;
} Device_Info;


typedef struct csdata_ {
  AudioDeviceID dev;
  AudioStreamBasicDescription format;
  int         inBufSamples;
  int         outBufSamples;
  int         currentInputIndex;
  int         currentOutputIndex;
  MYFLT       *inputBuffer;
  MYFLT       *outputBuffer;
  csRtAudioParams *inParm;
  csRtAudioParams *outParm;
  int onchnls, inchnls;
  AudioComponentInstance outunit;
  AudioComponentInstance inunit;
  CSOUND *csound;
  AudioBufferList *inputdata;
  int disp;
  AudioDeviceID defdevin;
  AudioDeviceID defdevout;
  int devnos;
  int devin;
  int devout;
  void *incb;
  void *outcb;
} csdata;


OSStatus  Csound_Input(void *inRefCon,
                       AudioUnitRenderActionFlags *ioActionFlags,
                       const AudioTimeStamp *inTimeStamp,
                       UInt32 inBusNumber,
                       UInt32 inNumberFrames,
                       AudioBufferList *ioData);

OSStatus  Csound_Render(void *inRefCon,
                        AudioUnitRenderActionFlags *ioActionFlags,
                        const AudioTimeStamp *inTimeStamp,
                        UInt32 dump,
                        UInt32 inNumberFrames,
                        AudioBufferList *ioData);

static void DAC_channels(CSOUND *csound, int chans){
    int *dachans = (int *) csoundQueryGlobalVariable(csound, "_DAC_CHANNELS_");
    if (dachans == NULL) {
      if (csoundCreateGlobalVariable(csound, "_DAC_CHANNELS_",
                                       sizeof(int)) != 0)
        return;
      dachans = (int *) csoundQueryGlobalVariable(csound, "_DAC_CHANNELS_");
      *dachans = chans;
    }
}

static void ADC_channels(CSOUND *csound, int chans){
    int *dachans = (int *) csoundQueryGlobalVariable(csound, "_ADC_CHANNELS_");
    if (dachans == NULL) {
      if (csoundCreateGlobalVariable(csound, "_ADC_CHANNELS_",
                                       sizeof(int)) != 0)
        return;
      dachans = (int *) csoundQueryGlobalVariable(csound, "_ADC_CHANNELS_");
      *dachans = chans;
    }
}

int AuHAL_open(CSOUND *csound, const csRtAudioParams * parm,
               csdata *cdata, int isInput)
{
    UInt32  psize, devnum, devnos;
    AudioDeviceID dev;
    AudioDeviceID *sysdevs;
    AudioStreamBasicDescription format;
    int     i;
    Device_Info *devinfo;
    UInt32  bufframes, nchnls;
    int devouts = 0, devins = 0;
    double srate;
    UInt32 enableIO, maxFPS;
    AudioComponent HALOutput;
    AudioComponentInstance *aunit;
    AudioComponentDescription cd = {kAudioUnitType_Output,
                                    kAudioUnitSubType_HALOutput,
                                    kAudioUnitManufacturer_Apple, 0, 0};
    AudioObjectPropertyAddress prop = {
      kAudioObjectPropertyName,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };
    CFStringRef devName;
    CFStringEncoding defaultEncoding = CFStringGetSystemEncoding();



    prop.mSelector = (isInput ?
                      kAudioHardwarePropertyDefaultInputDevice :
                      kAudioHardwarePropertyDefaultOutputDevice);

    psize = sizeof(AudioDeviceID);
    AudioObjectGetPropertyData(kAudioObjectSystemObject,
                               &prop, 0, NULL, &psize, &dev);

    if(isInput) cdata->defdevin = dev;
    else cdata->defdevout = dev;

    prop.mSelector = kAudioHardwarePropertyDevices;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                   &prop, 0, NULL, &psize);
    devnos = psize / sizeof(AudioDeviceID);
    sysdevs = (AudioDeviceID *) mmalloc(csound,psize);
    devinfo = (Device_Info *) mmalloc(csound,devnos*sizeof(Device_Info));
    AudioObjectGetPropertyData(kAudioObjectSystemObject,
                               &prop, 0, NULL, &psize, sysdevs);

    cdata->devnos = devnos;
    for (i = 0; (unsigned int) i < devnos; i++) {
      AudioBufferList *b;
      int devchannels, k, n;
      int numlists;
      psize = sizeof(CFStringRef);
      prop.mScope = kAudioObjectPropertyScopeGlobal;
      prop.mSelector = kAudioObjectPropertyName;
      AudioObjectGetPropertyData(sysdevs[i],
                                 &prop, 0, NULL, &psize, &devName);
      if(CFStringGetCStringPtr(devName, defaultEncoding))
        strNcpy(devinfo[i].name,
                CFStringGetCStringPtr(devName, defaultEncoding), 127);
      else
        strNcpy(devinfo[i].name, "unnamed device", 127);
      CFRelease(devName);

      devchannels = 0;
      prop.mScope = kAudioDevicePropertyScopeInput;
      prop.mSelector =  kAudioDevicePropertyStreamConfiguration;
      AudioObjectGetPropertyDataSize(sysdevs[i],
                                     &prop, 0, NULL, &psize);
      b = (AudioBufferList *) mmalloc(csound,psize);
      numlists = psize / sizeof(AudioBufferList);
      AudioObjectGetPropertyData(sysdevs[i],
                                 &prop, 0, NULL, &psize, b);
      for(n=0; n < numlists; n++){
        for(k=0; (unsigned int) k < b[n].mNumberBuffers; k++)
          devchannels += b[n].mBuffers[k].mNumberChannels;
      }
      devinfo[i].inchannels = devchannels;
      if(devchannels) {
        devins++;
        devinfo[i].indevnum = devins;
      } else devinfo[i].indevnum = -1;
      mfree(csound,b);

      devchannels = 0;
      prop.mScope = kAudioDevicePropertyScopeOutput;
      AudioObjectGetPropertyDataSize(sysdevs[i],
                                     &prop, 0, NULL, &psize);
      b = (AudioBufferList *) mmalloc(csound,psize);
      numlists = psize /sizeof(AudioBufferList);
      AudioObjectGetPropertyData(sysdevs[i],
                                 &prop, 0, NULL, &psize, b);
      for(n=0; n < numlists; n++){
        for(k=0; (unsigned int) k < b[n].mNumberBuffers; k++)
          devchannels += b[n].mBuffers[k].mNumberChannels;
      }
      devinfo[i].outchannels = devchannels;
      if(devchannels) {
        devouts++;
        devinfo[i].outdevnum = devouts;
      } else devinfo[i].outdevnum = -1;
      mfree(csound,b);
    }

     OPARMS O;
    csoundGetOParms(csound, &O);
    if(O.msglevel || O.odebug) {
    if (isInput)
      csoundMessage(csound,
                      Str("AuHAL Module: found %d input device(s):\n"), devins);
    else csoundMessage(csound,
                         Str("AuHAL Module: found %d output device(s):\n"),
                         devouts);

    for (i = 0; (unsigned int)  i < devnos; i++) {
      if (isInput) {
        if(devinfo[i].inchannels) {
          csoundMessage(csound, Str("%d: %s (%d channels)\n"),
                          devinfo[i].indevnum, devinfo[i].name,
                          devinfo[i].inchannels);
        }
      }
      else {
        if(devinfo[i].outchannels)
          csoundMessage(csound, Str("%d: %s (%d channels)\n"),
                          devinfo[i].outdevnum, devinfo[i].name,
                          devinfo[i].outchannels);
      }
    }
      }

    if (parm->devName != NULL) devnum = atoi(parm->devName);
    else devnum = parm->devNum;

    if (devnum > 0 && devnum < 1024) {
      int CoreAudioDev = -1;
      prop.mSelector = kAudioHardwarePropertyDevices;
      if (isInput) {
        for(i=0; (unsigned int)  i < devnos; i++) {
          if((unsigned int) devinfo[i].indevnum == devnum) {
            CoreAudioDev = i;
            break;
          }
        }
        if (LIKELY(CoreAudioDev >= 0)) {
          prop.mSelector = kAudioHardwarePropertyDefaultInputDevice;
          dev  = sysdevs[CoreAudioDev];
          AudioObjectSetPropertyData(kAudioObjectSystemObject, &prop,
                                     0, NULL, sizeof(AudioDeviceID), &dev);
        }
        else {
       if(O.msglevel || O.odebug)
          csoundWarning(csound, Str("requested device %d out of range"),
                             devnum);
        }

      }
      else {
        prop.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
        for(i=0;(unsigned int)  i < devnos; i++) {
          if((unsigned int) devinfo[i].outdevnum == devnum)  CoreAudioDev = i;
        }
        if (LIKELY(CoreAudioDev >= 0)) {
          dev  = sysdevs[CoreAudioDev];
          AudioObjectSetPropertyData(kAudioObjectSystemObject, &prop,
                                     0, NULL, sizeof(AudioDeviceID), &dev);

        }
        else {
          if(O.msglevel || O.odebug)
          csoundWarning(csound, Str("requested device %d (%s) out of range"),
                             devnum, devinfo[CoreAudioDev].name);
        }
      }
    }

    for(i=0; (unsigned int)  i < devnos; i++) {
      if(sysdevs[i] == dev){
        if(isInput) {
          if(devinfo[i].inchannels < parm->nChannels) {
            if(O.msglevel || O.odebug)
            csoundErrorMsg(csound,
                             Str(" *** CoreAudio: Device has not enough"
                                 " inputs (%d, requested %d)\n"),
                             devinfo[i].inchannels, parm->nChannels);
            return -1;
          }
          ADC_channels(csound, devinfo[i].inchannels);
        }
        else {
          if(devinfo[i].outchannels < parm->nChannels) {
            if(O.msglevel || O.odebug)
            csoundErrorMsg(csound,
                             Str(" *** CoreAudio: Device has not enough"
                                 " outputs (%d, requested %d)\n"),
                             devinfo[i].outchannels, parm->nChannels);
            return -1;
          }
          DAC_channels(csound, devinfo[i].outchannels);
        }
      }
    }
    mfree(csound,sysdevs);
    mfree(csound,devinfo);

    psize = sizeof(CFStringRef);
    prop.mSelector = kAudioObjectPropertyName;
    AudioObjectGetPropertyData(dev,
                               &prop, 0, NULL, &psize, &devName);
    if(O.msglevel || O.odebug) {
    if(isInput)
      csoundMessage(csound, Str("selected input device: %s\n"),
                      CFStringGetCStringPtr(devName, defaultEncoding));

    else
      csoundMessage(csound, Str("selected output device: %s\n"),
                      CFStringGetCStringPtr(devName, defaultEncoding));
    }
    CFRelease(devName);

    srate = csoundGetSr(csound);
    if(!isInput){
      nchnls =cdata->onchnls = parm->nChannels;
      bufframes = csoundGetOutputBufferSize(csound)/nchnls;
    }
    else {
      nchnls = cdata->inchnls = parm->nChannels;
      bufframes = csoundGetInputBufferSize(csound)/nchnls;
    }

    /* although the SR is set in the stream properties,
       we also need to set the device to match */
     double sr;
    prop.mSelector = kAudioDevicePropertyNominalSampleRate;
    if(!isInput){
      AudioObjectGetPropertyData(dev, &prop, 0, NULL, &psize, &sr);
      csoundSystemSr(csound, sr);
    }

    psize = sizeof(double);
    AudioObjectSetPropertyData(dev, &prop, 0, NULL, psize, &srate);
    AudioObjectGetPropertyData(dev, &prop, 0, NULL, &psize, &sr);

    if(srate < 0)
      srate  =  csoundSystemSr(csound, sr);
    if(UNLIKELY(sr != srate)) {
      if(O.msglevel || O.odebug)
       csoundWarning(csound,
                      Str("Attempted to set device SR, tried %.1f, got %.1f\n"),
                      srate, sr);
    }

    HALOutput = AudioComponentFindNext(NULL, &cd);
    if (isInput) {
      AudioComponentInstanceNew(HALOutput, &(cdata->inunit));
      enableIO = 1;
      AudioUnitSetProperty(cdata->inunit, kAudioOutputUnitProperty_EnableIO,
                           kAudioUnitScope_Input, 1, &enableIO, sizeof(enableIO));
      enableIO = 0;
      AudioUnitSetProperty(cdata->inunit, kAudioOutputUnitProperty_EnableIO,
                           kAudioUnitScope_Output, 0, &enableIO, sizeof(enableIO));
      psize = sizeof(AudioDeviceID);
      /* for input, select device AFTER enabling IO */
      AudioUnitSetProperty(cdata->inunit,kAudioOutputUnitProperty_CurrentDevice,
                           kAudioUnitScope_Global, isInput, &dev, psize);
      aunit = &(cdata->inunit);
    }
    else {
      AudioComponentInstanceNew(HALOutput, &(cdata->outunit));
      psize = sizeof(AudioDeviceID);
      /* for output, select device BEFORE enabling IO */
      AudioUnitSetProperty(cdata->outunit, kAudioOutputUnitProperty_CurrentDevice,
                           kAudioUnitScope_Global, isInput, &dev, psize);
      enableIO = 1;
      AudioUnitSetProperty(cdata->outunit, kAudioOutputUnitProperty_EnableIO,
                           kAudioUnitScope_Output, 0, &enableIO, sizeof(enableIO));
      enableIO = 0;
      AudioUnitSetProperty(cdata->outunit, kAudioOutputUnitProperty_EnableIO,
                           kAudioUnitScope_Input, 1, &enableIO, sizeof(enableIO));
      aunit = &(cdata->outunit);
    }
    /* now set the buffer size */
    psize = sizeof(AudioDeviceID);
    AudioUnitGetProperty(*aunit, kAudioOutputUnitProperty_CurrentDevice,
                         kAudioUnitScope_Global, isInput, &dev, &psize);
    prop.mSelector = kAudioDevicePropertyBufferFrameSize;
    psize = 4;
    AudioObjectSetPropertyData(dev, &prop, 0, NULL, psize, &bufframes);
    psize = sizeof(maxFPS);
    AudioUnitGetProperty(*aunit, kAudioUnitProperty_MaximumFramesPerSlice,
                         kAudioUnitScope_Global, isInput, &maxFPS, &psize);
    AudioUnitSetProperty(*aunit, kAudioUnitProperty_MaximumFramesPerSlice,
                         kAudioUnitScope_Global, isInput, &bufframes,
                         sizeof(UInt32));
    /* set the stream properties */
    psize = sizeof(AudioStreamBasicDescription);
    AudioUnitGetProperty(*aunit, kAudioUnitProperty_StreamFormat,
                         (isInput ? kAudioUnitScope_Output : kAudioUnitScope_Input),
                         isInput, &format, &psize);
    format.mSampleRate    = srate;
    format.mFormatID =  kAudioFormatLinearPCM;
    format.mFormatFlags = kAudioFormatFlagIsFloat  |
                          kAudioFormatFlagIsPacked |
                          kLinearPCMFormatFlagIsNonInterleaved;
    format.mBytesPerPacket = sizeof(Float32);
    format.mFramesPerPacket = 1;
    format.mBytesPerFrame = sizeof(Float32);
    format.mChannelsPerFrame = nchnls;
    format.mBitsPerChannel = sizeof(Float32)*8;
    AudioUnitSetProperty(*aunit, kAudioUnitProperty_StreamFormat,
                         (isInput ? kAudioUnitScope_Output : kAudioUnitScope_Input),
                         isInput, &format,
                         sizeof(AudioStreamBasicDescription));
    /* set the callbacks and open the device */
    if(!isInput) {
      AURenderCallbackStruct output;
      output.inputProc = Csound_Render;
      output.inputProcRefCon = cdata;
      AudioUnitSetProperty(*aunit, kAudioUnitProperty_SetRenderCallback,
                           kAudioUnitScope_Input, isInput, &output, sizeof(output));
      AudioUnitInitialize(*aunit);
      AudioOutputUnitStart(*aunit);

      if(O.msglevel || O.odebug)
       csoundMessage(csound,
                      Str("***** AuHAL module: output device open with %d "
                          "buffer frames\n"),
                          bufframes);
      cdata->disp = 0;
    }
    else {
      AURenderCallbackStruct input;
      AudioBufferList *CAInputData =
        (AudioBufferList*)mmalloc(csound,sizeof(UInt32)
                                 + cdata->inchnls * sizeof(AudioBuffer));
      CAInputData->mNumberBuffers = cdata->inchnls;
      for (i = 0; i < cdata->inchnls; i++) {
        CAInputData->mBuffers[i].mNumberChannels = 1;
        CAInputData->mBuffers[i].mDataByteSize =
          bufframes * sizeof(Float32);
        CAInputData->mBuffers[i].mData =
          mcalloc(csound,bufframes* sizeof(Float32));
      }
      cdata->inputdata = CAInputData;

      input.inputProc = Csound_Input;
      input.inputProcRefCon = cdata;
      AudioUnitSetProperty(*aunit, kAudioOutputUnitProperty_SetInputCallback,
                           kAudioUnitScope_Input, isInput, &input, sizeof(input));
      AudioUnitInitialize(*aunit);
      AudioOutputUnitStart(*aunit);
      if(O.msglevel || O.odebug)
       csoundMessage(csound,
                      Str("***** AuHAL module: input device open with "
                          "%d buffer frames\n"),
                      (int) bufframes);
    }

    cdata->disp = 0;
    return 0;

}

int listDevices(CSOUND *csound, CS_AUDIODEVICE *list, int isOutput){
    UInt32  psize, devnos;
    AudioDeviceID *sysdevs;
    Device_Info *devinfo;
    int     i;
    int devouts = 0, devins = 0;

    AudioObjectPropertyAddress prop = {
      kAudioObjectPropertyName,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };
    CFStringRef devName;
    CFStringEncoding defaultEncoding = CFStringGetSystemEncoding();
    prop.mSelector = kAudioHardwarePropertyDevices;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                   &prop, 0, NULL, &psize);
    devnos = psize / sizeof(AudioDeviceID);
    sysdevs = (AudioDeviceID *) mmalloc(csound,psize);
    devinfo = (Device_Info *) mmalloc(csound,devnos*sizeof*devinfo);
    AudioObjectGetPropertyData(kAudioObjectSystemObject,
                               &prop, 0, NULL, &psize, sysdevs);

    for (i = 0; (unsigned int) i < devnos; i++) {
      AudioBufferList *b;
      int devchannels, k, n;
      int numlists;
      psize = sizeof(CFStringRef);
      prop.mScope = kAudioObjectPropertyScopeGlobal;
      prop.mSelector = kAudioObjectPropertyName;
      AudioObjectGetPropertyData(sysdevs[i],
                                 &prop, 0, NULL, &psize, &devName);
      memset(devinfo[i].name,0,128);
      if(CFStringGetCStringPtr(devName, defaultEncoding) != NULL)
        strNcpy(devinfo[i].name,
                CFStringGetCStringPtr(devName, defaultEncoding),127);
      CFRelease(devName);

      devchannels = 0;
      prop.mScope = kAudioDevicePropertyScopeInput;
      prop.mSelector =  kAudioDevicePropertyStreamConfiguration;
      AudioObjectGetPropertyDataSize(sysdevs[i],
                                     &prop, 0, NULL, &psize);
      b = (AudioBufferList *) mmalloc(csound,psize);
      numlists = psize / sizeof(AudioBufferList);
      AudioObjectGetPropertyData(sysdevs[i],
                                 &prop, 0, NULL, &psize, b);
      for(n=0; n < numlists; n++){
        for(k=0; (unsigned int)  k < b[n].mNumberBuffers; k++)
          devchannels += b[n].mBuffers[k].mNumberChannels;
      }
      devinfo[i].inchannels = devchannels;
      if(devchannels) {
        devins++;
        devinfo[i].indevnum = devins;
      } else devinfo[i].indevnum = -1;
      mfree(csound,b);

      devchannels = 0;
      prop.mScope = kAudioDevicePropertyScopeOutput;
      AudioObjectGetPropertyDataSize(sysdevs[i],
                                     &prop, 0, NULL, &psize);
      b = (AudioBufferList *) mmalloc(csound,psize);
      numlists = psize /sizeof(AudioBufferList);
      AudioObjectGetPropertyData(sysdevs[i],
                                 &prop, 0, NULL, &psize, b);
      for(n=0; n < numlists; n++){
        for(k=0; (unsigned int) k < b[n].mNumberBuffers; k++)
          devchannels += b[n].mBuffers[k].mNumberChannels;
      }
      devinfo[i].outchannels = devchannels;
      if(devchannels) {
        devouts++;
        devinfo[i].outdevnum = devouts;
      } else devinfo[i].outdevnum = -1;
      mfree(csound,b);
    }
    if(list==NULL){
      return (isOutput ? devouts : devins);
    } else {

      char tmp[64], *s;
      int n=0, i;

      if ((s = (char*) csoundQueryGlobalVariable(csound, "_RTAUDIO")) == NULL)
        return 0;

      if(!isOutput){
        for(i=0; (unsigned int)  i < devnos; i++) {
          if(devinfo[i].inchannels) {
            strNcpy(list[n].device_name,  devinfo[i].name, 63);
            snprintf(tmp, 64, "adc%d", devinfo[i].indevnum);
            strNcpy(list[n].device_id, tmp, 63);
            strNcpy(list[n].rt_module, s, 63);
            list[n].max_nchnls = devinfo[i].inchannels;
            list[n].isOutput = 0;
            n++;
          }
        }
        return n;
      } else {
        for(i=0;(unsigned int) i < devnos; i++){
          if(devinfo[i].outchannels) {
            strNcpy(list[n].device_name,  devinfo[i].name, 63);
            snprintf(tmp, 64, "dac%d", devinfo[i].outdevnum);
            strNcpy(list[n].device_id, tmp, 63);
            strNcpy(list[n].rt_module, s, 63);
            list[n].max_nchnls = devinfo[i].outchannels;
            list[n].isOutput = 1;
            n++;
          }
        }
        return n;
      }
    }
}

/* open for audio input */
static int recopen_(CSOUND *csound, const csRtAudioParams * parm)
{
    csdata  *cdata;
    void **recordata = csoundGetRtRecordUserData(csound);
    if (*(csoundGetRtRecordUserData(csound)) != NULL)
      return 0;

    /* allocate structure */

    if(*(csoundGetRtPlayUserData(csound) )!= NULL)
       cdata = (csdata *) *(csoundGetRtPlayUserData(csound));
    else {
       cdata = (csdata *) mcalloc(csound, sizeof(csdata));
      cdata->disp = 1;
    }

    cdata->inunit = NULL;
    *recordata = (void *) cdata;
    cdata->inParm =  (csRtAudioParams *) parm;
    cdata->csound = cdata->csound;
    cdata->inputBuffer =
      (MYFLT *) mcalloc(csound,
                               csoundGetInputBufferSize(csound)* sizeof(MYFLT));
    cdata->incb =
      csoundCreateCircularBuffer(csound,
                                   parm->bufSamp_HW*parm->nChannels, sizeof(MYFLT));


    int ret = AuHAL_open(csound, parm, cdata, 1);
    return ret;
}

/* open for audio output */
static int playopen_(CSOUND *csound, const csRtAudioParams * parm)
{
    csdata  *cdata;
    void    **playdata = csoundGetRtPlayUserData(csound);

    if(*(csoundGetRtRecordUserData(csound)) != NULL)
      cdata = (csdata *) *(csoundGetRtRecordUserData(csound));
    else {
      cdata = (csdata *) mcalloc(csound, sizeof(csdata));
      cdata->disp = 1;
    }
    cdata->outunit = NULL;
    *playdata = (void *) cdata;
    cdata->outParm =  (csRtAudioParams *) parm;
    cdata->csound = csound;
    cdata->outputBuffer =
      (MYFLT *) mcalloc(csound,
                               csoundGetOutputBufferSize(csound)* sizeof(MYFLT));
    memset(cdata->outputBuffer, 0,
           csoundGetOutputBufferSize(csound)*sizeof(MYFLT));
    cdata->outcb =
      csoundCreateCircularBuffer(csound,
                                   parm->bufSamp_HW*parm->nChannels, sizeof(MYFLT));

    return AuHAL_open(csound, parm,cdata,0);
}

OSStatus  Csound_Input(void *inRefCon,
                       AudioUnitRenderActionFlags *ioActionFlags,
                       const AudioTimeStamp *inTimeStamp,
                       UInt32 inBusNumber,
                       UInt32 inNumberFrames,
                       AudioBufferList *ioData)
{
    csdata *cdata = (csdata *) inRefCon;
    CSOUND *csound = cdata->csound;
    int inchnls = cdata->inchnls;
    MYFLT *inputBuffer = cdata->inputBuffer;
    int j,k;
    Float32 *buffer;
    int n = inNumberFrames*inchnls;
    int l;
    IGN(ioData);

    AudioUnitRender(cdata->inunit, ioActionFlags, inTimeStamp, inBusNumber,
                    inNumberFrames, cdata->inputdata);
    /*for (k = 0; k < inchnls; k++){
      buffer = (Float32 *) cdata->inputdata->mBuffers[k].mData;
      for(j=0; (unsigned int) j < inNumberFrames; j++){
        inputBuffer[j*inchnls+k] = buffer[j];
      }
      }*/
    unsigned int i, chns;
    for (i = 0; i <  cdata->inputdata->mNumberBuffers; i++) {
      buffer = (Float32 *)  cdata->inputdata->mBuffers[i].mData;
      chns =  cdata->inputdata->mBuffers[i].mNumberChannels;
      for(j=0, l=0; (unsigned int) j < inNumberFrames*chns; j+=chns, l++) {
	for (k = 0; k < chns; k++) {
	  inputBuffer[l*inchnls+(k+1)*i] = buffer[j+k];
	}
      }
    }      
    l = csoundWriteCircularBuffer(csound, cdata->incb,inputBuffer,n);
    return 0;
}

#define MICROS 1000000
static int rtrecord_(CSOUND *csound, MYFLT *inbuff_, int nbytes)
{
    csdata  *cdata;
    int n = nbytes/sizeof(MYFLT);
    int m = 0, l;//, w = n;
    //MYFLT sr = csoundGetSr(csound);
    cdata = (csdata *) *(csoundGetRtRecordUserData(csound));
    do{
      l = csoundReadCircularBuffer(csound,cdata->incb,&inbuff_[m],n);
      m += l;
      n -= l;
      //if(n) usleep(MICROS*w/sr);
    } while(n);
    return nbytes;
}

OSStatus  Csound_Render(void *inRefCon,
                        AudioUnitRenderActionFlags *ioActionFlags,
                        const AudioTimeStamp *inTimeStamp,
                        UInt32 inBusNumber,
                        UInt32 inNumberFrames,
                        AudioBufferList *ioData)
{
    csdata *cdata = (csdata *) inRefCon;
    CSOUND *csound = cdata->csound;
    int onchnls = cdata->onchnls;
    MYFLT *outputBuffer = cdata->outputBuffer;
    int j,k;
    Float32 *buffer;
    int n = inNumberFrames*onchnls;
    IGN(ioActionFlags);
    IGN(inTimeStamp);
    IGN(inBusNumber);

    n = csoundReadCircularBuffer(csound,cdata->outcb,outputBuffer,n);
    /* for (k = 0; k < onchnls; k++) { */
    /*   buffer = (Float32 *) ioData->mBuffers[k].mData; */
    /*   for(j=0; (unsigned int) j < inNumberFrames; j++){ */
    /*     buffer[j] = (Float32) outputBuffer[j*onchnls+k] ; */
    /*     outputBuffer[j*onchnls+k] = FL(0.0); */
    /*   } */
    /* } */
    unsigned int i, l = 0, chns;
    for (i = 0; i < ioData->mNumberBuffers; i++) {
      buffer = (Float32 *) ioData->mBuffers[i].mData;
      chns = ioData->mBuffers[i].mNumberChannels;
      for(j=0, l=0; (unsigned int) j < inNumberFrames*chns; j+=chns, l++) {
	for (k = 0; k < chns; k++) {
	  buffer[j+k] = (Float32) outputBuffer[l*onchnls+(k+1)*i];
	  outputBuffer[l*onchnls+k*(i+1)] = FL(0.0);
	}
      }
    }  
    return 0;
}

static void rtplay_(CSOUND *csound, const MYFLT *outbuff_, int nbytes)
{
    csdata  *cdata;
    int n = nbytes/sizeof(MYFLT);
    int m = 0, l;//, w = n;
    //MYFLT sr = csoundGetSr(csound);
    cdata = (csdata *) *(csoundGetRtPlayUserData(csound));
    do {
      l = csoundWriteCircularBuffer(csound, cdata->outcb,&outbuff_[m],n);
      m += l;
      n -= l;
      //if(n) usleep(MICROS*n/sr);
    } while(n);
}

/* close the I/O device entirely  */
/* called only when both complete */

static void rtclose_(CSOUND *csound)
{
    csdata *cdata;
    cdata = (csdata *) *(csoundGetRtRecordUserData(csound));
    if(cdata == NULL)
      cdata = (csdata *) *(csoundGetRtPlayUserData(csound));

    if (cdata != NULL) {
      usleep(1000*csoundGetOutputBufferSize(csound)/
             (csoundGetSr(csound)*csoundGetNchnls(csound)));

      if(cdata->inunit != NULL){
        AudioOutputUnitStop(cdata->inunit);
        AudioUnitUninitialize(cdata->inunit);
        AudioComponentInstanceDispose(cdata->inunit);
      }

      if(cdata->outunit != NULL){
        AudioOutputUnitStop(cdata->outunit);
        AudioUnitUninitialize(cdata->outunit);
        AudioComponentInstanceDispose(cdata->outunit);
      }

      if (cdata->outputBuffer != NULL) {
        mfree(csound,cdata->outputBuffer);
        cdata->outputBuffer = NULL;
      }
      if (cdata->inputBuffer != NULL) {
        mfree(csound,cdata->inputBuffer);
        cdata->inputBuffer = NULL;
      }

      *(csoundGetRtRecordUserData(csound)) = NULL;
      *(csoundGetRtPlayUserData(csound)) = NULL;

      if(cdata->inputdata) {
        int i;
        for (i = 0; i < cdata->inchnls; i++)
          mfree(csound,cdata->inputdata->mBuffers[i].mData);
        mfree(csound,cdata->inputdata);
      }

      if(cdata->defdevin) {
        AudioObjectPropertyAddress prop = {
          kAudioHardwarePropertyDefaultInputDevice,
          kAudioObjectPropertyScopeGlobal,
          kAudioObjectPropertyElementMaster
        };
        UInt32 psize = sizeof(AudioDeviceID);
        AudioObjectSetPropertyData(kAudioObjectSystemObject,
                                   &prop, 0, NULL, psize, &cdata->defdevin);
      }
      if(cdata->defdevout) {
        AudioObjectPropertyAddress prop = {
          kAudioHardwarePropertyDefaultOutputDevice,
          kAudioObjectPropertyScopeGlobal,
          kAudioObjectPropertyElementMaster
        };
        UInt32 psize = sizeof(AudioDeviceID);
        AudioObjectSetPropertyData(kAudioObjectSystemObject,
                                   &prop, 0, NULL, psize, &cdata->defdevout);
      }
      csoundDestroyCircularBuffer(csound, cdata->incb);
      csoundDestroyCircularBuffer(csound, cdata->outcb);
      mfree(csound,cdata);
      OPARMS O;
      csoundGetOParms(csound, &O);
      if(O.msglevel || O.odebug)
       csoundMessage(csound, "%s", Str("AuHAL module: device closed\n"));
    }
}

int csoundModuleInit(CSOUND *csound)
{
    char   *drv;
    OPARMS O;
    csoundGetOParms(csound, &O);
    module_list_add(csound, "auhal", "audio");
    drv = (char *) csoundQueryGlobalVariable(csound, "_RTAUDIO");
    if (drv == NULL)
      return 0;
    if (!(strcmp(drv, "auhal") == 0 || strcmp(drv, "AuHal") == 0 ||
          strcmp(drv, "AUHAL") == 0 ||
          strcmp(drv, "coreaudio") == 0 || strcmp(drv, "CoreAudio") == 0 ||
          strcmp(drv, "COREAUDIO") == 0))
      return 0;
   if(O.msglevel || O.odebug)
    csoundMessage(csound, "%s", Str("rtaudio: coreaaudio-AuHAL module enabled\n"));
    csoundSetPlayopenCallback(csound, playopen_);
    csoundSetRecopenCallback(csound, recopen_);
    csoundSetRtplayCallback(csound, rtplay_);
    csoundSetRtrecordCallback(csound, rtrecord_);
    csoundSetRtcloseCallback(csound, rtclose_);
    csoundSetAudioDeviceListCallback(csound, listDevices);
    return 0;
}

int csoundModuleCreate(CSOUND *csound)
{
    IGN(csound);
    return 0;
}
