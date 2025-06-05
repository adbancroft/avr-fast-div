[![Build](https://github.com/adbancroft/avr-fast-div/actions/workflows/build.yml/badge.svg)](https://github.com/adbancroft/avr-fast-div/actions/workflows/build.yml)
[![Unit Tests](https://github.com/adbancroft/avr-fast-div/actions/workflows/unit-tests.yml/badge.svg)](https://github.com/adbancroft/avr-fast-div/actions/workflows/unit-tests.yml)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=adbancroft_avr-fast-div&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=adbancroft_avr-fast-div)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=adbancroft_avr-fast-div&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=adbancroft_avr-fast-div)

# avr-fast-div: optimized integer division for AVR hardware

This library provides *up to* 60% improvement in run time division speed on AVR hardware. Exact speedup varies depending on data types & number ranges - see below for details (also see the [unit tests](https://github.com/adbancroft/avr-fast-div/actions/workflows/unit-tests.yml)).

As a general guideline, avr-fast-div is applicable to these operations:

```
uint32_t/uint16_t
uint32_t/uint8_t
int32_t/int16_t
int32_t/int8_t
uint16_t/uint8_t
int16_t/int8_t
````

(or other divison operators where the dividend & divisor values fall within the ranges of the types above).

**Best practice:** Use the smallest data type that can hold the required integer range & prefer unsigned where possible.   

## Constraints

 1. division using a signed type and unsigned type is not supported. E.g. `int16_t/uint16_t` (it's also a recipe for confusion, since C++ converts the signed integer to an unsigned one before doing the division).
 2. There is no 64-bit support

## Using the library

### Installation

The library is available in both the [Arduino Library](https://www.arduino.cc/reference/en/libraries/avr-fast-div/) and [PlatformIO Library](https://registry.platformio.org/libraries/adbancroft/avr-fast-div) registries. 

The library can also be cloned & included locally or included directly from GitHub (if your tooling supports it). 

### Code

 1. `#include <avr-fast-div.h>`
 2. Replace divide operations with a call to fast_div. I.e.
     * `a / b` -> `fast_div(a, b)`

The code base is compatible with all platforms: non-AVR builds compile down to the standard division operator.

**Note:** if the divisor (`b`) is a [compile time constant greater than 8-bits](https://stackoverflow.com/questions/47994933/why-doesnt-gcc-or-clang-on-arm-use-division-by-invariant-integers-using-multip), you probably want to use [libdivide](https://libdivide.com/) instead.

You can control function inlining aggressiveness via the `AFD_INLINE` pre-processor flag (useful for flash constrained environments):

|Value|Effect     |Description|
|-----|-----------|-----------|
|`0`  | Forced Off|Functions are declared `noinline`|
|`1`  | Off       |Functions are *not* declared inline; compiler may choose to inline anyway|
|`2`  | On        |Functions *are* declared inline; compiler may not choose to inline|
|`3`  | Forced On|Functions are declared `always_inline`|

## Details

Since the AVR architecture has no hardware divider, all run time division is done in software by the compiler emitting a call to one of the division functions (E.g. [__udivmodsi4](https://github.com/gcc-mirror/gcc/blob/cdd5dd2125ca850aa8599f76bed02509590541ef/libgcc/config/avr/lib1funcs.S#L1615)) contained in a [runtime support library](https://gcc.gnu.org/wiki/avr-gcc#Exceptions_to_the_Calling_Convention).

By neccesity, the division functions are optimised for the general case. Combined with integer type promotion, this can result in sub-optimal division speed. E.g.

```
    uint16_t divisor = 355;    // Note: greater than UINT8_MAX
    uint32_t dividend = 85123; // Note: greater than UINT16_MAX
    
    uint32_t result = dividend / divisor;  // 239U
    // 1. Divisor is promoted to uint32_t
    // (following C/C++ integer promotion rules)
    // 2. __udivmodsi4() is called to divide (32/32=>32 division)
```

If the program is using a limited range of `[u]int32_t` or `[u]int16_t`, this can be sped up a lot. 

Specifically, **if the divisor can be contained in a smaller type than the dividend *and* the result will fit into the smaller divisor type then we can halve the time of the division operation.**

Where possible, avr-fast-div will route division operations through functions optimized for the following operations:
1. `uint32_t/uint16_t => uint16_t`
2. `uint16_t/uint8_t => uint8_t`

As a result, the optimizations are most effective when the number ranges are constrained to a range smaller than the full integral type min & max values. 

Example

  * An `unsigned long` storing time in milliseconds (the Arduino `millis()` function return type) has a range of 0 to ~1193 **hours**
  * If the code base only tracks time for 1 hour,  the variable is artificially constrained to `[0, 3600000]`
  * Division operations on it can be optimised when the divisor is greater than 64 (since `36000000/65<UINT16_MAX`)
