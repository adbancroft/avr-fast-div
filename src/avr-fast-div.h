#pragma once

/** @file
 * @brief Optimized division *for AVR only*. See @ref group-opt-div
*/

#include <stdint.h>
#include <limits.h>

/// @defgroup group-opt-div Optimised integer division for the AVR platform
///
/// @brief Optimised integer division for the AVR platform
///
/// The compiler/libcc does not do any optimisation based on number ranges - it uses the
/// standard promotion rules to get operand type equivalence then hands off to a well 
/// defined function (E.g. __divmodqi4, __divmodqi4 etc.)    
///
/// Usage:
/// @code
///      rpmDelta = fast_div(lshift<10>(toothDeltaV), (6U * toothDeltaT));
/// @endcode
///
/// @note Code is usable on all architectures, but the optimization only applies to AVR.
/// Other compilers will see a standard div operator.
/// @{
    
/// @brief Preprocessor flag to turn on optimized division.
/// If not set eternally, will be automatically set on AVR platform.
#if !defined(USE_OPTIMIZED_DIV)
#if defined(__AVR__) || defined(DOXYGEN_DOCUMENTATION_BUILD)
#define USE_OPTIMIZED_DIV
#endif
#endif

#if defined(USE_OPTIMIZED_DIV)
#include "type_traits.h"

namespace avr_fast_div_impl {

/// @brief A verion of abs that handles the edge case of INT(\d*)_MIN without overflow.
///
/// Because in 2's complement, the minimum negative is ((MAX+1)*-1). E.g.
///    INT8_MIN = -128
///    INT8_MAX =  127
///    INT16_MIN = -32768
///    INT16_MAX =  32767
///
/// See https://stackoverflow.com/a/12231604
///
/// @tparam TSigned Input signed type
/// @tparam TUnsigned Output unsigned type
/// @param svalue signed value, possible negative
/// @return absolute value of svalue
template <typename TSigned, typename TUnsigned=type_traits::make_unsigned_t<TSigned> >
static inline TUnsigned safe_abs(TSigned svalue) {
  return (TUnsigned)(svalue < 0 ? -((TUnsigned)svalue) : ((TUnsigned)svalue));
}

}

// Public API

uint8_t fast_div(uint8_t udividend, uint8_t udivisor);
uint16_t fast_div(uint16_t udividend, uint8_t udivisor);
uint16_t fast_div(uint16_t udividend, uint16_t udivisor);
uint32_t fast_div(uint32_t udividend, uint8_t udivisor);
uint32_t fast_div(uint32_t udividend, uint16_t udivisor);
uint32_t fast_div(uint32_t udividend, uint32_t udivisor);

// Overload for all signed types
template <typename TDividend, typename TDivisor>
static inline TDividend fast_div(TDividend dividend, TDivisor divisor) {
  // Do not mix signed and unsigned types: it won't generate the results you expect.
  // E.g.
  //  int a = INT_MIN;
  //  unsigned int b = UINT_MAX/2;
  //  int results = a/b;
  //  TEST_ASSERT_EQUAL(results, -1);
  // will fail - the result is 1
  static_assert(type_traits::is_signed<TDividend>::value, "TDividend must be signed");
  static_assert(type_traits::is_signed<TDivisor>::value, "TDivisor must be signed");

  // Convert to unsigned.
  using udividend_t = type_traits::make_unsigned_t<TDividend>;
  udividend_t udividend = avr_fast_div_impl::safe_abs(dividend);
  using udivisor_t = type_traits::make_unsigned_t<TDivisor>;
  udivisor_t udivisor = avr_fast_div_impl::safe_abs(divisor);

  // Call the overload specialized for the unsigned type (above) - these are optimized.
  udividend_t uresult = fast_div(udividend, udivisor);

  const bool isSameSign = ((dividend<0) == (divisor<0));
  if (!isSameSign) {
    return (TDividend)-((TDividend)uresult);
  }
  return uresult;
}

#else

// Non-AVR platforms just fallback to standard div operator
template <typename TDividend, typename TDivisor>
static inline TDividend fast_div(TDividend dividend, TDivisor divisor) {
  return dividend / divisor;
}

#endif
/// @}