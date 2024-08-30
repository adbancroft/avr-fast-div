#include <Arduino.h>
#include <unity.h>
#include "avr_fast_div.h"
#include "lambda_timer.hpp"
#include "unity_print_timers.hpp"
#include "test_utils.h"
#include "index_range_generator.hpp"

// The performance tests here focus on the internal optimized division functions.
// The tests use constrained number ranges to avoid overflow.
//
// This also showcases the maximum performance improvement.
#if defined(__AVR__)

static void test_udivSiHi2_vs_u32u32_perf(void)
{
  constexpr uint16_t steps = 333U;
  static constexpr index_range_generator<uint16_t> divisorGen(2U, UINT16_MAX/3U, steps);
  static constexpr index_range_generator<uint32_t> dividendGen(UINT16_MAX+1UL, UINT16_MAX*4UL, steps); 

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += optimized_div_impl::udivSiHi2(dividendGen.generate(index), divisorGen.generate(index)).quot;
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(0U, steps, 1U, nativeTest, optimizedTest);

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

static void test_udivHiQi2_vs_u16u16_perf(void)
{
  constexpr uint16_t steps = UINT8_MAX/2U;
  static constexpr index_range_generator<uint8_t> divisorGen(UINT8_MAX/4U, UINT8_MAX, steps);
  static constexpr index_range_generator<uint16_t> dividendGen(UINT8_MAX*3U, 1U<<12U, steps);

  static auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) {
    checkSum += optimized_div_impl::udivHiQi2(dividendGen.generate(index), divisorGen.generate(index)).quot;
  };
  auto comparison = compare_executiontime<uint8_t, uint32_t>(32U, 0U, steps, 1U, nativeTest, optimizedTest);

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

static void test_udivHiQi2_vs_udivSiHi2_perf(void)
{
  constexpr uint16_t steps = UINT8_MAX/2U;
  static constexpr index_range_generator<uint8_t> divisorGen(UINT8_MAX/4U, UINT8_MAX, steps);
  static constexpr index_range_generator<uint16_t> dividendGen(UINT8_MAX*3U, 1U<<12U, steps);

  static auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { 
    checkSum += optimized_div_impl::udivSiHi2(dividendGen.generate(index), divisorGen.generate(index)).quot;
  };
  static auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) {
    checkSum += optimized_div_impl::udivHiQi2(dividendGen.generate(index), divisorGen.generate(index)).quot;
  };
  auto comparison = compare_executiontime<uint8_t, uint32_t>(32U, 0U, steps, 1U, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

#endif

void test_implementation_performance(void) {
#if defined(__AVR__)
   SET_UNITY_FILENAME() {
        RUN_TEST(test_udivSiHi2_vs_u32u32_perf);
        RUN_TEST(test_udivHiQi2_vs_u16u16_perf);
        RUN_TEST(test_udivHiQi2_vs_udivSiHi2_perf);
   }
#endif
}