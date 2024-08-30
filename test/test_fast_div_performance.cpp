#include <Arduino.h>
#include <unity.h>
#include "avr_fast_div.h"
#include "lambda_timer.hpp"
#include "unity_print_timers.hpp"
#include "test_utils.h"
#include "index_range_generator.hpp"

static void test_fast_div_u16_u8_perf(void)
{
  constexpr uint16_t steps = UINT8_MAX-1U;
  static constexpr index_range_generator<uint8_t> divisorGen(1U, UINT8_MAX, steps);
  static constexpr index_range_generator<uint16_t> dividendGen(1U, UINT16_MAX, steps);

  static auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint8_t, uint32_t>(32U, 0U, steps, 1U, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // We only expect a speed improvement on AVR
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
#endif
}

static void test_fast_div_u16_u16_perf(void)
{
  constexpr uint16_t steps = 3333U;
  static constexpr index_range_generator<uint16_t> divisorGen(1U, UINT16_MAX/2U, steps);
  static constexpr index_range_generator<uint16_t> dividendGen((UINT16_MAX/2U)+1U, UINT16_MAX, steps);

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(0U, steps, 1U, nativeTest, optimizedTest);
  
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
  constexpr uint16_t steps = 3333U;
  static constexpr index_range_generator<uint16_t> divisorGen(1U, UINT16_MAX, steps);
  static constexpr index_range_generator<uint32_t> dividendGen(1UL, 1UL<<24UL, steps);

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(0U, steps, 1U, nativeTest, optimizedTest);

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // We only expect a speed improvement on AVR
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
#endif
}

static void test_fast_div_u32_u16_perf(void)
{
  constexpr uint16_t steps = 3333U;
  static constexpr index_range_generator<uint16_t> divisorGen(1U, UINT16_MAX, steps);
  static constexpr index_range_generator<uint32_t> dividendGen(1UL, UINT32_MAX, steps);

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(0U, steps, 1U, nativeTest, optimizedTest);

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
  constexpr uint16_t steps = 3333U;
  static constexpr index_range_generator<uint32_t> divisorGen(1UL, UINT32_MAX/2UL, steps);
  static constexpr index_range_generator<uint32_t> dividendGen(UINT32_MAX/2UL+1, UINT32_MAX, steps);

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(0U, steps, 1U, nativeTest, optimizedTest);

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
  constexpr uint16_t steps = 3333U;
  static constexpr index_range_generator<int16_t> divisorGen(INT16_MIN, INT16_MAX, steps);
  static constexpr index_range_generator<int32_t> dividendGen(INT32_MIN, INT32_MAX, steps);

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(0U, steps, 1U, nativeTest, optimizedTest);  

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