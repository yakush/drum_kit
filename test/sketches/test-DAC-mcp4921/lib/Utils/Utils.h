#ifndef UTILS_H_
#define UTILS_H_

#include <Arduino.h>

int32_t fastSigmoid(int32_t x32) {

/*
 * fast sigmoid approximation (cubic taylor)
 * using only mult and shifts on 64 bit number
 *
 * from float sigmoid :
 * y = 64000*(1/(1+EXP(-x/100000))-0.5);
 *
 * to int polynomial:
 * y = ( (x*3*2^14) - (x*x*x/2^16) ) / 2^15
 *
 * about range clip:
 * above/bellow +/-32767 it's going far from the original curve
 * but those are the int32 range anyway
 *
 * also to keep int64 range in the calculation : (x*x*x) < 2^63
 * meaning that :  x < 2,097,152 = 2^21
 *
 * time improvements:
 * 13.93 uSec for float calc
 *  0.27 uSec for int64 calc
 *
 * x51 faster!!!
 *
 */

  if (x32>=  32767) return  32767;
  if (x32<= -32767) return -32768;

  int64_t x =x32;
  return ( ((x*3)<<14) - ((x*x*x)>>16) ) >>15;
}

/*
input : a potentially overflowing int32 to be sigmoid down to int32
*/
#define SIG_64_CLIP_X 3136600629
#define SIG_64_CLIP_Y 2091067086

int32_t fastSigmoid_64to32(int64_t x)
{

  /*
    y = x - x^3 * (5/2^67)
  */

  if (x >= +SIG_64_CLIP_X) return +SIG_64_CLIP_Y;
  if (x <= -SIG_64_CLIP_X) return -SIG_64_CLIP_Y;

  int64_t x3 = x >> 17;
  x3 = x3 * x3 * x3;

  return (int32_t)(x - (5 * x3) >> 16);
}

#endif