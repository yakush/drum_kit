#ifndef UTILS_YAK_H_
#define UTILS_YAK_H_

#include <Arduino.h>

int32_t fastSigmoid(int32_t x32);

/* input : a potentially overflowing int32 to be sigmoid down to int32 */
int32_t fastSigmoid_64to32(int64_t x);

#endif