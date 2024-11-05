#pragma once
#include "afd_type_traits.h"

namespace avr_fast_div_impl {

/**
 * @brief Get the width of a type in bits at compile time
 * 
 * @tparam T Type to get width of
 */
template <typename T>
struct bit_width {
  static constexpr uint8_t value = sizeof(T) * CHAR_BIT;
};

// Process one step in the division algorithm for uint32_t/uint16_t
static inline uint32_t divide_step(uint32_t dividend, const uint16_t &divisor) {
    asm(
        "    lsl  %A0      ; shift\n\t"
        "    rol  %B0      ;  rem:quot\n\t"
        "    rol  %C0      ;   left\n\t"
        "    rol  %D0      ;    by 1\n\t"
        "    brcs 1f       ; if carry out, rem > divisor\n\t"
        "    cp   %C0, %A1 ; is rem less\n\t"
        "    cpc  %D0, %B1 ;  than divisor ?\n\t"
        "    brcs 2f       ; yes, when carry out\n\t"
        "1:\n\t"
        "    sub  %C0, %A1 ; compute\n\t"
        "    sbc  %D0, %B1 ;  rem -= divisor\n\t"
        "    ori  %A0, 1   ; record quotient bit as 1\n\t"
        "2:\n\t"
      : "=d" (dividend) 
      : "d" (divisor) , "0" (dividend)
      : 
    ); 
    return dividend;
}

// Process one step in the division algorithm for uint16_t/uint8_t
static inline uint16_t divide_step(uint16_t dividend, const uint8_t &divisor) {
    asm(
        "    lsl  %A0      ; shift\n\t"
        "    rol  %B0      ;  rem:quot\n\t"
        "    brcs 1f       ; if carry out, rem > divisor\n\t"
        "    cpc  %B0, %A1 ; is rem less than divisor?\n\t"
        "    brcs 2f       ; yes, when carry out\n\t"
        "1:\n\t"
        "    sub  %B0, %A1 ; compute rem -= divisor\n\t"
        "    ori  %A0, 1   ; record quotient bit as 1\n\t"
        "2:\n\t"
      : "=d" (dividend) 
      : "d" (divisor) , "0" (dividend) 
      : 
    );
    return dividend;  
}

/**
 * @brief Optimised division: uint[n]_t/uint[n/2U]_t => uint[n/2U]_t quotient + uint[n/2U]_t remainder
 * 
 * Optimised division of unsigned number by unsigned smaller data type when it is known
 * that the quotient fits into the smaller data type. I.e.
 *    uint32_t/uint16_t => uint16_t
 *    uint16_t/uint8_t => uint8_t
 * 
 * ~70% quicker than raw 32/32 => 32 division on ATMega
 * 
 * @note Bad things will likely happen if the quotient doesn't fit into the divisor.
 * @note Copied from https://stackoverflow.com/a/66593564
 * 
 * @param dividend The dividend (numerator)
 * @param divisor The divisor (denominator)
 * @return dividend/divisor 
 */
template <typename TDividend, typename TDivisor>
static inline TDivisor divide(TDividend dividend, const TDivisor &divisor) {
  static_assert(afd_type_traits::is_unsigned<TDividend>::value, "TDividend must be unsigned");
  static_assert(afd_type_traits::is_unsigned<TDivisor>::value, "TDivisor must be unsigned");
  static_assert(sizeof(TDividend)==sizeof(TDivisor)*2U, "TDivisor must half the size of TDividend");

  for (uint8_t index=0U; index<bit_width<TDivisor>::value; ++index) {
    dividend = divide_step(dividend, divisor);
  }
  // Lower word contains the quotient, upper word contains the remainder
  return (TDivisor)dividend;
}

template <typename T>
static inline bool is_aligned(const T &reference, const T &dependent) {
  static constexpr T max_bit = (T)1U << (bit_width<T>::value-1U);
  return ((T)(dependent<<(T)1U) > reference) || (dependent & max_bit);
}

/**
 * @brief Left aligns the highest set bit of the dependent with the
 * highest set bit of the reference
 *
 * *Note* modifies parameter "dependent"
 *  
 * @param reference 
 * @param dependent 
 * @return T A flag with a single bit set at the aligned bit 
 */
template <typename T>
static inline T align(const T &reference, T &dependent) {

  T bit = 1;
  while (!is_aligned(reference, dependent))
  {
    dependent = (T)(dependent<<1U);
    bit = (T)(bit<<1U);
  }
  return bit;
}

template <typename T>
static constexpr inline T get_large_divisor_threshhold(void) {
  // This is just the ?INT_MAX of the next smaller integer type.
  // E.g. if T==uint32_t, this returns UINT16_MAX
  return (T)(((T)1U << (bit_width<T>::value/2))-1U);
}

/**
 * @brief A division function, applicable when the divisor is large
 * 
 * This function will work for all combinations of dividend & divisor, but only
 * apply it when the divisor is large. I.e. >sqrt(max(dividend)).
 * 
 * In this situation, on aggregate it's quicker to align the dividend with the divisor
 * and then divide rather than call the standard divide operator.
 * 
 * @tparam T 
 * @param udividend 
 * @param udivisor 
 * @return T 
 */
template <typename T>
static inline T divide_large_divisor(T udividend, T udivisor) {
  static_assert(afd_type_traits::is_unsigned<T>::value, "T must be unsigned");

#if defined(UNIT_TEST)
  if (udivisor<=get_large_divisor_threshhold<T>()) { 
    return 0;
  }
#endif

  if (udividend<udivisor) {
    return 0;
  }
  T bit = align(udividend, udivisor);

  // align() guarentees that udivisor<udividend
  udividend = (T)(udividend-udivisor);
  T res = bit;
  bit = (T)(bit>>1U);
  udivisor = (T)(udivisor>>1U);

  while (bit)
  {
    if (udividend >= udivisor)
    {
        udividend = (T)(udividend-udivisor);
        res = (T)(res|bit);
    }
    bit = (T)(bit>>1U);
    udivisor = (T)(udivisor>>1U);
  }
  return res;
}

}