#pragma once

#include <stdint.h>
#include <Unity.h>

// ============================ SET_UNITY_FILENAME ============================ 

static inline uint8_t ufname_set(const char *newFName)
{
    Unity.TestFile = newFName;
    return 1;
}

static inline void ufname_szrestore(char** __s)
{
    Unity.TestFile = *__s;
    __asm__ volatile ("" ::: "memory");
}


#define UNITY_FILENAME_RESTORE char* _ufname_saved                           \
    __attribute__((__cleanup__(ufname_szrestore))) = (char*)Unity.TestFile

#define SET_UNITY_FILENAME()                                                        \
for ( UNITY_FILENAME_RESTORE, _ufname_done = ufname_set(__FILE__);                  \
    _ufname_done; _ufname_done = 0 )

// ============================ Overflow free interpolation ============================ 

template <typename TIn, typename TOut>
static inline constexpr float compute_slope(const TIn &inMin, const TIn &inMax, const TOut &outMin, const TOut &outMax) {
  return (float)(outMax-outMin) / ((float)inMax-(float)inMin);
}

// Interpolate avoiding integer overflow.
// Very slow - do not use in production code.
template <typename TIn, typename TOut>
static inline constexpr TOut interpolate(const TIn x, const TIn inMin, const TIn inMax, const TOut outMin, const TOut outMax) {
  // We use floating point intermediate to avoid integer overflow
  return (TOut)(compute_slope(inMin, inMax,outMin, outMax) * ((float)x-(float)inMin));
}

// ============================ Dividend generation ============================ 

// Performance test divisors are created by iterating over a fixed integer range with a fixed step.
// E.g. [0, UINT16_MAX], step size 7777U
// The function functions map an unsigned divisor to a unsigned dividend by interpolation, limiting the
// upper bound of the dividend range by bit width
template <typename TDivisor, typename TDividend>
static inline TDividend generateDividendFromDivisorU(TDivisor index, TDivisor min, TDivisor max, uint8_t bitRangeWidth) {
  return interpolate<TDivisor, TDividend>(index, min, max, 1U, (TDividend)1U<<(TDividend)bitRangeWidth);
}
