#pragma once

#include <stdint.h>
#include <limits.h>

// A poor, but adequate, replacement for type traits templates in the
// C++ standard library (since AVR-GCC doesn't ship with a standard library implementation).
namespace afd_type_traits {

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
