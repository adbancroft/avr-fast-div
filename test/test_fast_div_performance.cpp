#include <Arduino.h>
#include <unity.h>
#include "avr-fast-div.h"
#include "lambda_timer.hpp"
#include "unity_print_timers.hpp"
#include "test_utils.h"

// Performance test divisors are created by iterating over a fixed integer range with a fixed step.
// E.g. [0, UINT16_MAX], step size 7777U
// The generateDividendFromDivisor family of functions maps the divisor to a dividend by interpolation.
template <typename TDivisor>
static uint16_t generateDividendFromDivisorU12(TDivisor index, TDivisor min, TDivisor max) {
  return generateDividendFromDivisorU<TDivisor, uint16_t>(index, min, max, 12U);
}
template <typename TDivisor>
static uint16_t generateDividendFromDivisorU16(TDivisor index, TDivisor min, TDivisor max) {
  return generateDividendFromDivisorU<TDivisor, uint16_t>(index, min, max, 16U);
}
template <typename TDivisor>
static uint32_t generateDividendFromDivisorU24(TDivisor index, TDivisor min, TDivisor max) {
  return generateDividendFromDivisorU<TDivisor, uint32_t>(index, min, max, 24U);
}
template <typename TDivisor>
static uint32_t generateDividendFromDivisorU32(TDivisor index, TDivisor min, TDivisor max) {
  return generateDividendFromDivisorU<TDivisor, uint32_t>(index, min, max, 32U);
}

template <typename TDivisor>
static int16_t generateDividendFromDivisorI16(TDivisor index, TDivisor min, TDivisor max) {
  return interpolate<TDivisor, int16_t>(index, min, max, INT16_MIN, INT16_MAX);
}
template <typename TDivisor>
static int32_t generateDividendFromDivisorI32(TDivisor index, TDivisor min, TDivisor max) {
  return interpolate<TDivisor, int32_t>(index, min, max, INT32_MIN, INT32_MAX);
}

static void test_fast_div_u16_u8_perf(void)
{
  constexpr uint8_t start_index = 1U;
  constexpr uint8_t end_index = UINT8_MAX;
  constexpr uint8_t step = 1;

  auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += (uint32_t)(generateDividendFromDivisorU16(index, start_index, end_index) / (uint16_t)index); };
  auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) { checkSum += (uint32_t)fast_div(generateDividendFromDivisorU16(index, start_index, end_index), index); };
  auto comparison = compare_executiontime<uint8_t, uint32_t>(20, start_index, end_index, step, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // We only expect a speed improvement on AVR
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
#endif
}

static void test_fast_div_u16_u16_perf(void)
{
  constexpr uint16_t step = UINT16_MAX/7777U;
  constexpr uint16_t start_index = 1U;
  constexpr uint16_t end_index = UINT16_MAX - step;

  auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += (uint32_t)((uint16_t)generateDividendFromDivisorU16(index, start_index, end_index) / (uint16_t)index); };
  auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += (uint32_t)fast_div((uint16_t)generateDividendFromDivisorU16(index, start_index, end_index), index); };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(start_index, end_index, step, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  // The u16/u16 case my be slightly slower than the native operation, since the vast majority of the 
  // divisors will be >UINT8_MAX. It all depends on the number ranges!

  // We'll give a 2% margin
  auto margin = comparison.timeA.timer.duration_micros()/50U;
  TEST_ASSERT_UINT32_WITHIN(margin, comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

static void test_fast_div_u24_u16_perf(void)
{
  constexpr uint16_t step = UINT16_MAX/7777U;
  constexpr uint16_t start_index = 1U;
  constexpr uint16_t end_index = UINT16_MAX - step;

  auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += generateDividendFromDivisorU24(index, start_index, end_index) / (uint32_t)index; };
  auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += fast_div(generateDividendFromDivisorU24(index, start_index, end_index), index); };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(start_index, end_index, step, nativeTest, optimizedTest);

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // We only expect a speed improvement on AVR
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
#endif
}

static void test_fast_div_u32_u16_perf(void)
{
  constexpr uint16_t step = UINT16_MAX/5355U;
  constexpr uint16_t start_index = 1;
  constexpr uint16_t end_index = UINT16_MAX - step;

  auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += generateDividendFromDivisorU32(index, start_index, end_index) / (uint32_t)index; };
  auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) { checkSum += fast_div(generateDividendFromDivisorU32(index, start_index, end_index), index); };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(start_index, end_index, step, nativeTest, optimizedTest);

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  // The u32/u16 case will only be slightly faster than the native operation, since the vast majority of the 
  // results will not fit into uint16_t. It all depends on the number ranges!

  // We'll give a 5% margin
  auto margin = comparison.timeA.timer.duration_micros()/20U;
  TEST_ASSERT_UINT32_WITHIN(margin, comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

static void test_fast_div_u32_u32_perf(void)
{
  constexpr uint32_t step = UINT32_MAX/7777U;
  constexpr uint32_t start_index = 1;
  constexpr uint32_t end_index = UINT32_MAX - step;

  auto nativeTest = [] (uint32_t index, uint32_t &checkSum) { checkSum += generateDividendFromDivisorU32(index, start_index, end_index) / (uint32_t)index; };
  auto optimizedTest = [] (uint32_t index, uint32_t &checkSum) { checkSum += fast_div(generateDividendFromDivisorU32(index, start_index, end_index), index); };
  auto comparison = compare_executiontime<uint32_t, uint32_t>(start_index, end_index, step, nativeTest, optimizedTest);

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  // The u32/u32 case will be about the same speed the native operation, since the vast majority of the 
  // divisors will be >UINT16_MAX. It all depends on the number ranges!

  // We'll give a 2% margin
  auto margin = comparison.timeA.timer.duration_micros()/50U;
  TEST_ASSERT_UINT32_WITHIN(margin, comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

static void test_fast_div_s32_s16_perf(void)
{
  constexpr int16_t step = INT16_MAX/3333;
  constexpr int16_t start_index = INT16_MIN;
  constexpr int16_t end_index = INT16_MAX - step;

  auto nativeTest = [] (int16_t index, int32_t &checkSum) { checkSum += generateDividendFromDivisorI32(index, start_index, end_index) / (int32_t)index; };
  auto optimizedTest = [] (int16_t index, int32_t &checkSum) { checkSum += fast_div(generateDividendFromDivisorI32(index, start_index, end_index), index); };
  auto comparison = compare_executiontime<int16_t, int32_t>(start_index, end_index, step, nativeTest, optimizedTest);

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__)
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
#endif
}

void test_fast_div_performance(void) {
   SET_UNITY_FILENAME() {
        RUN_TEST(test_fast_div_u16_u8_perf);
        RUN_TEST(test_fast_div_u16_u16_perf);
        RUN_TEST(test_fast_div_u24_u16_perf);
        RUN_TEST(test_fast_div_u32_u16_perf);
        RUN_TEST(test_fast_div_u32_u32_perf);
        RUN_TEST(test_fast_div_s32_s16_perf);
    }
}