#pragma once

/** @file
 * @brief Optimized division *for AVR only*. See @ref group-opt-div
*/

#include <stdint.h>

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
  return divisor>(TDivisor)( dividend >> ((sizeof(TDividend)-sizeof(TDivisor))*8U));
}

/// @brief Return type for udivSiHi2
struct udiv16_t {
  uint16_t quot; 
  uint16_t rem;
};

/**
 * @brief Optimised division: uint32_t/uint16_t => uint16_t quotient + uint16_t remainder
 * 
 * Optimised division of unsigned 32-bit by unsigned 16-bit when it is known
 * that the quotient fits into unsigned 16-bit.
 * 
 * ~60% quicker than raw 32/32 => 32 division on ATMega
 * 
 * @note Bad things will likely happen if the quotient doesn't fit into 16-bits.
 * @note Copied from https://stackoverflow.com/a/66593564
 * 
 * @param dividend The dividend (numerator)
 * @param divisor The divisor (denominator)
 * @return udiv16_t 
 */
static inline udiv16_t udivSiHi2(uint32_t dividend, uint16_t divisor)
{
  if (divisor==0U) { return { .quot = UINT16_MAX, .rem = 0 }; }

// The assembly code is worth an additional 10% perf improvement on AVR
#define AVR_FAST_DIV_ASM
#if defined(AVR_FAST_DIV_ASM)
  #define INDEX_REG "r16"

  asm(
      "    ldi " INDEX_REG ", 16 ; bits = 16\n\t"
      "0:\n\t"
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
      "    dec " INDEX_REG " ; bits--\n\t"
      "    brne 0b           ; until bits == 0"
      : "=d" (dividend) 
      : "d" (divisor) , "0" (dividend) 
      : INDEX_REG
  );

  // 
  return { .quot = (uint16_t)(dividend & 0x0000FFFFU), .rem = (uint16_t)(dividend >> 16U) };

#else
  uint16_t quot = dividend;        
  uint16_t rem  = dividend >> 16U;  

  uint8_t bits = sizeof(uint16_t) * CHAR_BIT;     
  do {
    // (rem:quot) << 1, with carry out
    bool carry = rem >> 15U;
    rem  = (rem << 1U) | (quot >> 15U);
    quot = quot << 1U;
    // if partial remainder greater or equal to divisor, subtract divisor
    if (carry || (rem >= divisor)) {
        rem = rem - divisor;
        quot = quot | 1U;
    }
    --bits;
  } while (bits);
  return { quot, rem };
#endif  
}

/// @brief Return type for udivHiQi2
struct udiv8_t {
  uint8_t quot; 
  uint8_t rem;
};

/**
 * @brief Optimised division: uint16_t/uint8_t => uint8_t quotient + uint8_t remainder
 * 
 * Optimised division of unsigned 16-bit by unsigned 8-bit when it is known
 * that the result fits into unsigned 8-bit.
 * 
 * ~60% quicker than raw 16/16 => 16 division on ATMega
 * 
 * @note Bad things will likely happen if the result doesn't fit into 8-bits.
 * @note Copied from https://stackoverflow.com/a/66593564
 * 
 * @param dividend The dividend (numerator)
 * @param divisor The divisor (denominator)
 * @return udiv8_t 
 */
static inline udiv8_t udivHiQi2(uint16_t dividend, uint8_t divisor)
{
    if (divisor==0U) { return { .quot = UINT8_MAX, .rem = 0U }; }

    #define INDEX_REG "r16"

    asm(
        "    ldi " INDEX_REG ", 8 ; bits = 8\n\t"
        "0:\n\t"
        "    lsl  %A0      ; shift rem:quot\n\t"
        "    rol  %B0      ;  left by 1\n\t"
        "    brcs 1f       ; if carry out, rem > divisor\n\t"
        "    cpc  %B0, %A1 ; is rem less than divisor?\n\t"
        "    brcs 2f       ; yes, when carry out\n\t"
        "1:\n\t"
        "    sub  %B0, %A1 ; compute rem -= divisor\n\t"
        "    ori  %A0, 1   ; record quotient bit as 1\n\t"
        "2:\n\t"
        "    dec  " INDEX_REG "     ; bits--\n\t"
        "    brne 0b        ; until bits == 0"
        : "=d" (dividend) 
        : "d" (divisor) , "0" (dividend) 
        : INDEX_REG
    );

  // Lower word contains the quotient, upper word contains the remainder.
  return { .quot = (uint8_t)(dividend & 0x00FFU), .rem = (uint8_t)(dividend >> 8U) };
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

static inline uint8_t fast_div(uint8_t udividend, uint8_t udivisor) {
  return udividend / udivisor;
}

static inline uint16_t fast_div(uint16_t udividend, uint8_t udivisor) {
  // Use u16/u8=>u8 if possible
  if (optimized_div_impl::udivResultFitsInDivisor(udividend, udivisor)) {
    return optimized_div_impl::udivHiQi2(udividend, udivisor).quot;
  } 
  return udividend / udivisor;
}

static inline uint16_t fast_div(uint16_t udividend, uint16_t udivisor) {
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
    return optimized_div_impl::udivSiHi2(udividend, udivisor).quot;
  }
  return udividend / udivisor;
}

static inline uint32_t fast_div(uint32_t udividend, uint32_t udivisor) {
  // Shrink to u32/u16=>u32 if possible
  if (udivisor<(uint32_t)UINT16_MAX) {
    return fast_div(udividend, (uint16_t)udivisor);
  }
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
#define fast_div(a, b) ((a)/(b))
#endif
/// @}