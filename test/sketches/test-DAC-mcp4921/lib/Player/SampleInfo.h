#ifndef SampleInfo_H_
#define SampleInfo_H_

#include "WavFile.h"

typedef struct SampleInfo_s
{    
    wavProps_t wavProps;
    String filepath;
} SampleInfo_t;

#endif //SampleInfo_H_