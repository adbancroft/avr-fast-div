# avr-fast-div: optimized integer division for avr-gcc
Division is a relatively slow operation on all platforms, compared to addition, subrtraction & multiplication. This library provides *up to* 40% improvement in run time division speed on AVR hardware.

Exact speedup varies depending on data types & number ranges - see below.
## Summary
avr-fast-div contains optimizations for these operations:
1. `uint32_t/uint16_t => uint16_t`
2. `uint16_t/uint8_t => uint8_t`
3. `int32_t/int16_t => int16_t` 
4. `int16_t/int8_t => int8_t`

The optimizations are most effective when your number ranges are constrained to much less than the type maximum values. E.g. time in milliseconds will probably be uint32_t (the Arduino `millis()` function return type, max. value 4294967296), but your codebase only needs to track time for 1 hour (max. value 3600000).
## Using the library

1. `#include "avr_fast_div.h"`
2. Replace all divide operations with a call to fast_div. I.e.
    * `a / b` -> `fast_div(a, b)`

The code base is compatible with all platforms: non-AVR builds compile down to the standard division operator.

**Note:** if the divisor (`b`) is a [compile time constant greater than 8-bits](https://stackoverflow.com/questions/47994933/why-doesnt-gcc-or-clang-on-arm-use-division-by-invariant-integers-using-multip), you probably want to use [libdivide](https://libdivide.com/) instead.
## Background
Since the AVR architecture has no hardware divider, all run time division is done in software by the compiler emitting a call to one of the division functions (E.g. [__udivmodsi4](https://github.com/gcc-mirror/gcc/blob/cdd5dd2125ca850aa8599f76bed02509590541ef/libgcc/config/avr/lib1funcs.S#L1615)) contained in a [runtime support library](https://gcc.gnu.org/wiki/avr-gcc#Exceptions_to_the_Calling_Convention).

By neccesity, the division functions are optimised for the general case. Combined with integer type promotion, this can result in sub-optimal division speed. E.g.

    uint16_t divisor = 355;    // Note: greater than UINT8_MAX
    uint32_t dividend = 85123; // Note: greater than UINT16_MAX
    
    uint32_t result = dividend / divisor;  // 239U
    // 1. Divisor is promoted to uint32_t
    // (following C/C++ integer promotion rules)
    // 2. __udivmodsi4() is called to divide (32/32=>32 division)

If the program is using a limited range of [u]int32_t or [u]int16_t, this can be sped up a lot. 

Specifically, **if the divisor can be contained in a smaller type than the dividend *and* the result will fit into the smaller divisor type then we can halve the time of the division operation.**

