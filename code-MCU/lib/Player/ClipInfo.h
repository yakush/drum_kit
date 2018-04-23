#ifndef ClipInfo_H_
#define ClipInfo_H_

#include "WavFile.h"

typedef struct ClipInfo_s
{
    wavProps_t wavProps;
    String filepath;
} ClipInfo_t;

#endif //SampleInfo_H_