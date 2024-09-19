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

#if defined(USE_OPTIMIZED_DIV) || defined(UNIT_TEST)

// Private to the fast_div() implementation
/// @cond

// A poor, but adequate, replacement for type traits templates in the
// C++ standard library (since AVR-GCC doesn't ship with a standard library implementation).
namespace type_traits {

  /// integral_constant
  template<typename _Tp, _Tp __v>
    struct integral_constant
    {
      static constexpr _Tp                  value = __v;
      typedef _Tp                           value_type;
      typedef integral_constant<_Tp, __v>   type;
      constexpr operator value_type() const noexcept { return value; }
    };

  template<typename _Tp, _Tp __v>
    constexpr _Tp integral_constant<_Tp, __v>::value;

  template<bool __v>
    using __bool_constant = integral_constant<bool, __v>;

  /// The type used as a compile-time boolean with true value.
  typedef __bool_constant<true> true_type;

  /// The type used as a compile-time boolean with false value.
  typedef __bool_constant<false> false_type;
  

  template<typename _Pp>
    struct __not__ : public __bool_constant<!bool(_Pp::value)> { };

  // Limited replacement for std::is_unsigned
  template<typename _Tp>
    struct is_unsigned : public false_type { };

  template<>
    struct is_unsigned<uint8_t> : public true_type { };

  template<>
    struct is_unsigned<uint16_t> : public true_type { };

  template<>
    struct is_unsigned<uint32_t> : public true_type { };

  template<>
    struct is_unsigned<uint64_t> : public true_type { };

  template<typename _Tp>
    struct is_signed : public __not__<is_unsigned<_Tp>> { };

  // Limited replacement for std::make_unsigned 
  template<typename _Tp>
    struct make_unsigned { typedef _Tp type; };
  
  template<> 
    struct make_unsigned<int8_t> { typedef uint8_t type; };

  template<>
    struct make_unsigned<int16_t> { typedef uint16_t type; };

  template<>
    struct make_unsigned<int32_t> { typedef uint32_t type; };

  template<>
    struct make_unsigned<int64_t> { typedef uint64_t type; };

  template<typename _Tp>
    using make_unsigned_t = typename make_unsigned<_Tp>::type;
}

#endif

#if defined(USE_OPTIMIZED_DIV)

// Private to the fast_div() implementation
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
  static_assert(type_traits::is_unsigned<TDividend>::value, "TDividend must be unsigned");
  static_assert(type_traits::is_unsigned<TDivisor>::value, "TDivisor must be unsigned");
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
  static_assert(type_traits::is_unsigned<T>::value, "T must be unsigned");

#if defined(UNIT_TEST)
  // constexpr T threshold = ;
  if (udivisor<=get_large_divisor_threshhold<T>()) { 
    return 0;
  }
#endif

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

/// @endcond

// Public API

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

#if !defined(AFD_DEFENSIVE_CHECKS)
/**
 * @brief A single source of truth for the defensive checks in fast_div
 * 
 * These are required in order to protect the core implementation funtions
 * which don't do any defensive checks.
 * 
 * @param dividend The dividend (numerator)
 * @param divisor The divisor (denominator)
 * @return dividend/divisor
 */
#define AFD_DEFENSIVE_CHECKS(dividend, divisor) \
  AFD_ZERO_DIVISOR_CHECK(dividend, divisor) \
  if ((dividend)<(divisor)) { return 0U; } \
  if ((dividend)==(divisor)) { return 1U; }
#endif

static inline uint8_t fast_div(uint8_t udividend, uint8_t udivisor) {
  AFD_ZERO_DIVISOR_CHECK(udividend, udivisor);
  // u8/u8 => u8
  return udividend / udivisor;
}

static inline uint16_t fast_div(uint16_t udividend, uint8_t udivisor) {
  AFD_DEFENSIVE_CHECKS(udividend, udivisor);
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

static inline uint16_t fast_div(uint16_t udividend, uint16_t udivisor) {
  AFD_DEFENSIVE_CHECKS(udividend, udivisor);
  if (udivisor<=(uint16_t)UINT8_MAX) {
    return fast_div(udividend, (uint8_t)udivisor);
  }
  // We now know that udivisor > 255U. I.e. upper word bits are set
  // u16/u16=>u16
  return avr_fast_div_impl::divide_large_divisor(udividend, udivisor);
}

static inline uint32_t fast_div(uint32_t udividend, uint16_t udivisor) {
  AFD_DEFENSIVE_CHECKS(udividend, udivisor);
  // Use u32/u16=>u16 if possible
  if (udivisor > (uint16_t)(udividend >> 16U)) {
    return avr_fast_div_impl::divide(udividend, udivisor);
  }
// Fallback to 24-bit if supported
#if defined(__UINT24_MAX__)
  if((udividend<__UINT24_MAX__)) {
    return (__uint24)udividend / (__uint24)udivisor;
  }  
#endif    
  // We now know that udividend > udivisor * 65536U
  // u32/u32=>u32
  return udividend / udivisor;
}

static inline uint32_t fast_div(uint32_t udividend, uint8_t udivisor) {
  return fast_div(udividend, (uint16_t)udivisor);
}

static inline uint32_t fast_div(uint32_t udividend, uint32_t udivisor) {
  AFD_DEFENSIVE_CHECKS(udividend, udivisor);
  // Shrink to u32/u16=>u32 if possible
  if (udivisor<=(uint32_t)UINT16_MAX) {
    return fast_div(udividend, (uint16_t)udivisor);
  }
// Fallback to 24-bit if supported
#if defined(__UINT24_MAX__)
  if((udividend<__UINT24_MAX__) && (udivisor<__UINT24_MAX__)) {
    return (__uint24)udividend / (__uint24)udivisor;
  }  
#endif  
  // We now know that udivisor > 65535U. I.e. upper word bits are set
  // u32/u32=>u32
  return avr_fast_div_impl::divide_large_divisor<uint32_t>(udividend, udivisor);
}

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