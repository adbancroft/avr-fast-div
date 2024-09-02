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
namespace optimized_div_impl {

/**
 * @brief Will the result of division fit into the TDivisor type?
 * 
 * @tparam TDividend Dividend type (unsigned) 
 * @tparam TDivisor Divisor type (unsigned)
 * @param dividend The dividend (numerator)
 * @param divisor The divisor (denominator)
 * @return true If the result of dividend/divisor will fit into TDivisor, false otherwise
 */
template <typename TDividend, typename TDivisor>
static constexpr inline bool udivResultFitsInDivisor(TDividend dividend, TDivisor divisor) {
  static_assert(type_traits::is_unsigned<TDividend>::value, "TDividend must be unsigned");
  static_assert(type_traits::is_unsigned<TDivisor>::value, "TDivisor must be unsigned");
  return (dividend<=divisor) || divisor>(TDivisor)( dividend >> ((sizeof(TDividend)-sizeof(TDivisor))*8U));
}

// Not as fast or compact as the assembly code, but it is more portable.
#if 0

#if 0 // WIP

// Rotate Left
template <typename T, uint8_t n>
static inline T rotl (const T &x)
{
  constexpr unsigned int mask = (CHAR_BIT*sizeof(T)-1);  // e.g. 31
  return (x<<n) | (x>>( (-n)&mask ));
}

template <typename TDividend, typename TDivisor>
static inline bool compare_rem_divisor(const TDividend &dividend, const TDivisor &b) {
  static constexpr uint8_t bit_count = sizeof(TDivisor) * CHAR_BIT;
  TDivisor hiword  = (TDivisor)(dividend >> bit_count);
  return (hiword >= b);
}

template <typename TDividend, typename TDivisor>
static inline TDividend subtract_rem_divisor(TDividend dividend, const TDivisor &divisor) {
#if 0  
  // Very, very slow
  TDivisor *pUpper = ((TDivisor*)&dividend) + 1;
  *pUpper = (TDivisor)(*pUpper - divisor);
  return dividend;
#endif 

  // This is too slow compared to the ASM, which subtracts in-place
  static constexpr uint8_t bit_count = sizeof(divisor) * CHAR_BIT;
  static constexpr TDividend mask = ((TDividend)1U<<bit_count)-1U;
  uint16_t rem  = (uint16_t)(dividend >> bit_count);
  return (dividend & mask) | ((TDividend)(rem - divisor) << bit_count);
}

template <typename TDividend, typename TDivisor>
static inline TDividend udivCoreProcess1Bit(TDividend dividend, const TDivisor &divisor) {
  static constexpr uint8_t bit_countMajor = sizeof(TDividend) * CHAR_BIT;
  static constexpr TDividend carry_mask = (TDividend)1U << (bit_countMajor-1U);
  const bool carry = dividend & carry_mask;
  dividend = rotl<TDividend, 1U>(dividend);
  if (carry || compare_rem_divisor(dividend, divisor)) {
    dividend = subtract_rem_divisor(dividend, divisor) | (TDividend)1U;
  }
  return dividend;
}
#endif

template <typename TDividend, typename TDivisor>
static inline TDividend divide(TDividend dividend, const TDivisor &divisor) {
  static_assert(type_traits::is_unsigned<TDividend>::value, "TDividend must be unsigned");
  static_assert(type_traits::is_unsigned<TDivisor>::value, "TDivisor must be unsigned");
  static_assert(sizeof(TDividend)==sizeof(TDivisor)*2U, "TDivisor must be unsigned");

  static constexpr uint8_t bit_count_divisor = sizeof(TDivisor) * CHAR_BIT;

  if (divisor==0U) { return ((TDividend)1U<<bit_count_divisor)*2-1; }

  TDivisor quot = (TDivisor)dividend;        
  TDivisor rem  = (TDivisor)(dividend >> bit_count_divisor);  

  for (uint8_t index=0U; index<bit_count_divisor; ++index) {
    bool carry = rem >> (bit_count_divisor-1U);
    // (rem:quot) << 1, with carry out
    rem  = (TDivisor)((rem << 1U) | (quot >> (bit_count_divisor-1U)));
    quot = (TDivisor)(quot << 1U);
    // if partial remainder greater or equal to divisor, subtract divisor
    if (carry || (rem >= divisor)) {
        rem = (TDivisor)(rem - divisor);
        quot = quot | 1U;
    }
  }
  return ((TDividend)rem << bit_count_divisor) | (TDividend)quot;
}

#else

// Process one step in the division algorithm for uint32_t/uint16_t
static inline uint32_t divide_step(uint32_t dividend, const uint16_t &divisor) {
    asm(
        "    lsl  %A0      ; shift\n\t" \
        "    rol  %B0      ;  rem:quot\n\t" \
        "    rol  %C0      ;   left\n\t" \
        "    rol  %D0      ;    by 1\n\t" \
        "    brcs 1f       ; if carry out, rem > divisor\n\t" \
        "    cp   %C0, %A1 ; is rem less\n\t" \
        "    cpc  %D0, %B1 ;  than divisor ?\n\t" \
        "    brcs 2f       ; yes, when carry out\n\t" \
        "1:\n\t" \
        "    sub  %C0, %A1 ; compute\n\t" \
        "    sbc  %D0, %B1 ;  rem -= divisor\n\t" \
        "    ori  %A0, 1   ; record quotient bit as 1\n\t" \
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
        "    lsl  %A0      ; shift\n\t" \
        "    rol  %B0      ;  rem:quot\n\t" \
        "    brcs 1f       ; if carry out, rem > divisor\n\t" \
        "    cpc  %B0, %A1 ; is rem less than divisor?\n\t" \
        "    brcs 2f       ; yes, when carry out\n\t" \
        "1:\n\t" \
        "    sub  %B0, %A1 ; compute rem -= divisor\n\t" \
        "    ori  %A0, 1   ; record quotient bit as 1\n\t" \
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
static inline TDividend divide(TDividend dividend, const TDivisor &divisor) {
  static_assert(type_traits::is_unsigned<TDividend>::value, "TDividend must be unsigned");
  static_assert(type_traits::is_unsigned<TDivisor>::value, "TDivisor must be unsigned");
  static_assert(sizeof(TDividend)==sizeof(TDivisor)*2U, "TDivisor must be unsigned");

  static constexpr uint8_t bit_count = sizeof(TDivisor) * CHAR_BIT;

  if (divisor==0U) { return ((TDividend)1U<<bit_count)*2-1; }

  for (uint8_t index=0U; index<bit_count; ++index) {
    dividend = divide_step(dividend, divisor);
  }
  return dividend;
}

#endif


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

static inline uint8_t fast_div(uint8_t udividend, uint8_t udivisor) {
  return udividend / udivisor;
}

static inline uint16_t fast_div(uint16_t udividend, uint8_t udivisor) {
  // Use u16/u8=>u8 if possible
  if (optimized_div_impl::udivResultFitsInDivisor(udividend, udivisor)) {
    return optimized_div_impl::divide(udividend, udivisor) & 0x00FFU;
  } 
  return udividend / udivisor;
}

static inline uint16_t fast_div(uint16_t udividend, uint16_t udivisor) {
  if (udivisor<(uint16_t)UINT8_MAX) {
    return fast_div(udividend, (uint8_t)udivisor);
  }
  return udividend / udivisor;
}

static inline uint32_t fast_div(uint32_t udividend, uint8_t udivisor) {
  if (udividend<(uint32_t)UINT16_MAX) {
    return fast_div((uint16_t)udividend, udivisor);
  }
  return udividend / udivisor;
}

static inline uint32_t fast_div(uint32_t udividend, uint16_t udivisor) {
  // Use u32/u16=>u16 if possible
  if (optimized_div_impl::udivResultFitsInDivisor(udividend, udivisor)) {
    return optimized_div_impl::divide(udividend, udivisor) & 0x0000FFFFU;
  } 
// Fallback to 24-bit if supported
#if defined(__UINT24_MAX__)
  if((udividend<__UINT24_MAX__)) {
    return (__uint24)udividend / (__uint24)udivisor;
  }  
#endif   
  return udividend / udivisor;
}

static inline uint32_t fast_div(uint32_t udividend, uint32_t udivisor) {
  // Shrink to u32/u16=>u32 if possible
  if (udivisor<(uint32_t)UINT16_MAX) {
    return fast_div(udividend, (uint16_t)udivisor);
  }
// Fallback to 24-bit if supported
#if defined(__UINT24_MAX__)
  if((udividend<__UINT24_MAX__) && (udivisor<__UINT24_MAX__)) {
    return (__uint24)udividend / (__uint24)udivisor;
  }  
#endif  
  return udividend / udivisor;
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
  udividend_t udividend = optimized_div_impl::safe_abs(dividend);
  using udivisor_t = type_traits::make_unsigned_t<TDivisor>;
  udivisor_t udivisor = optimized_div_impl::safe_abs(divisor);

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