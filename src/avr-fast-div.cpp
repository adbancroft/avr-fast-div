#include "avr-fast-div.h"

#if defined(USE_OPTIMIZED_DIV)

#include "afd_implementation.hpp"

#if !defined(AFD_ZERO_DIVISOR_CHECK)
/**
 * @brief Check for zero divisor
 * 
 * Defaults to AVR behavior - return zero. However, this macro can
 * be pre-defined by the host code base if you want different behavior.
 * E.g. abort(), throw exception 
 *
 * @param dividend The dividend (numerator)
 * @param divisor The divisor (denominator)
 * @return dividend/divisor
 */
#define AFD_ZERO_DIVISOR_CHECK(dividend, divisor) \
  if ((divisor)==0U) { return 0U; }
#endif

// ===================== Public Functions =====================

uint8_t fast_div16_8(uint16_t udividend, uint8_t udivisor) {
  AFD_ZERO_DIVISOR_CHECK(udividend, udivisor);
  return (uint8_t)avr_fast_div_impl::divide(udividend, udivisor);
}

uint16_t fast_div32_16(uint32_t udividend, uint16_t udivisor) {
  AFD_ZERO_DIVISOR_CHECK(udividend, udivisor);
  return avr_fast_div_impl::divide(udividend, udivisor);
}

uint8_t fast_div(uint8_t udividend, uint8_t udivisor) {
  AFD_ZERO_DIVISOR_CHECK(udividend, udivisor);
  // u8/u8 => u8
  return udividend / udivisor;
}

uint16_t fast_div(uint16_t udividend, uint8_t udivisor) {
  AFD_ZERO_DIVISOR_CHECK(udividend, udivisor);
  // Use u16/u8=>u8 if possible
  if (udivisor > (uint8_t)(udividend >> 8U)) {
    return avr_fast_div_impl::divide(udividend, udivisor);
  } 
  // We now know:
  //    (udividend >= (udivisor * 255U))
  // && (udivisor<255U)

  // u16/u16=>u16
  return udividend / udivisor;
}

uint16_t fast_div(uint16_t udividend, uint16_t udivisor) {
  AFD_ZERO_DIVISOR_CHECK(udividend, udivisor);
  if (udivisor<=(uint16_t)UINT8_MAX) {
    return fast_div(udividend, (uint8_t)udivisor);
  }
  // We now know that udivisor > 255U. I.e. upper word bits are set
  // u16/u16=>u16
  return avr_fast_div_impl::divide_large_divisor(udividend, udivisor);
}

static inline uint32_t fast_divu32u16(uint32_t udividend, uint16_t udivisor) {
    // Use u32/u16=>u16 if possible
  if (udivisor > (uint16_t)(udividend >> 16U)) {
    return avr_fast_div_impl::divide(udividend, udivisor);
  }
  // We now know that udividend > udivisor * 65536U
  // u32/u32=>u32
  return udividend / udivisor;
}

uint32_t fast_div(uint32_t udividend, uint16_t udivisor) {
  AFD_ZERO_DIVISOR_CHECK(udividend, udivisor);
  return fast_divu32u16(udividend, udivisor);
}

uint32_t fast_div(uint32_t udividend, uint8_t udivisor) {
  AFD_ZERO_DIVISOR_CHECK(udividend, udivisor);
  return fast_divu32u16(udividend, udivisor);
}

uint32_t fast_div(uint32_t udividend, uint32_t udivisor) {
  AFD_ZERO_DIVISOR_CHECK(udividend, udivisor);
  // Shrink to u32/u16=>u32 if possible
  if (udivisor<=(uint32_t)UINT16_MAX) {
    return fast_divu32u16(udividend, (uint16_t)udivisor);
  }
  // We now know that udivisor > 65535U. I.e. upper word bits are set
  // u32/u32=>u32
  return avr_fast_div_impl::divide_large_divisor<uint32_t>(udividend, udivisor);
}

#endif