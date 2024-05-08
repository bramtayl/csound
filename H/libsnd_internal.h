#pragma once

#include "csound.h"

#define STA_LIBSND(x)   (csound->libsndStatics.x)

int  check_rtaudio_name(char *fName, char **devName, int isOutput);
void iotranset(CSOUND *csound);
void sfopenin(CSOUND *csound);           /* init for continuous soundin */
void sfclosein(CSOUND *csound);
void sfcloseout(CSOUND *csound);
void sfopenout(CSOUND *csound);
void sfnopenout(CSOUND *csound);
