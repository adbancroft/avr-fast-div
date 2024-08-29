#include <Arduino.h>
#include <unity.h>
#include "avr_fast_div.h"
#include "lambda_timer.hpp"
#include "unity_print_timers.hpp"
#include "test_utils.h"

#if defined(__AVR__)

static void test_udivSiHi2_vs_u32u32_perf(void)
{
  constexpr uint16_t step = UINT16_MAX/3333U;
  constexpr uint16_t start_index = 3U;
  constexpr uint16_t end_index = UINT16_MAX - step;

  static uint32_t coreTestCount = 0U;
  static uint32_t divTestCount = 0U;

  coreTestCount = 0U;
  divTestCount = 0U;
  static auto coreTest = [] (uint16_t index, uint32_t(*pDiv)(uint32_t, uint16_t), uint32_t &checkSum) { 
    uint32_t dividend = generateDividendFromDivisorU<uint16_t, uint32_t>(index, start_index, end_index, 24U);
    ++coreTestCount;
    // We need to protect udivSiHi2 from spurious input.
    if (optimized_div_impl::udivResultFitsInDivisor(dividend, index)) {
      checkSum += pDiv(dividend, index); 
      ++divTestCount;
    }
  };
  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    static auto divFunc = [](uint32_t dividend, uint16_t divisor) -> uint32_t {
      return dividend / (uint32_t)divisor;
    };
    coreTest(index, divFunc, checkSum);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    static auto divFunc = [](uint32_t dividend, uint16_t divisor) -> uint32_t {
      return optimized_div_impl::udivSiHi2(dividend, divisor).quot;
    };
    coreTest(index, divFunc, checkSum);
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(start_index, end_index, step, nativeTest, optimizedTest);

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
  // Make sure that 90% of the tests actually were within limits.
  // I.e. udivResultFitsInDivisor in coreTest returned true
  TEST_ASSERT_INT32_WITHIN(coreTestCount/10U, coreTestCount, divTestCount);
}

static void test_udivHiQi2_vs_u16u16_perf(void)
{
  constexpr uint16_t iters = 32;
  constexpr uint8_t step = 1;
  constexpr uint8_t start_index = 1;
  constexpr uint8_t end_index = UINT8_MAX - step;

  static uint32_t coreTestCount = 0U;
  static uint32_t divTestCount = 0U;

  coreTestCount = 0U;
  divTestCount = 0U;
  static auto coreTest = [] (uint8_t index, uint16_t(*pDiv)(uint16_t, uint8_t), uint32_t &checkSum) { 
    uint16_t dividend = generateDividendFromDivisorU<uint8_t, uint16_t>(index, start_index, end_index, 12U);
    ++coreTestCount;
    // We need to protect udivHiQi2 from spurious input.
    if (optimized_div_impl::udivResultFitsInDivisor(dividend, index)) {
      checkSum += pDiv(dividend, index); 
      ++divTestCount;
    }
  };

  auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) { 
    static auto divFunc = [](uint16_t dividend, uint8_t divisor) -> uint16_t {
      return optimized_div_impl::udivHiQi2(dividend, divisor).quot;
    };
    coreTest(index, divFunc, checkSum);    
  };

  auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { 
    static auto divFunc = [](uint16_t dividend, uint8_t divisor) -> uint16_t {
      return dividend / (uint16_t)divisor;
    };
    coreTest(index, divFunc, checkSum);
  };
  auto comparison = compare_executiontime<uint8_t, uint32_t>(iters, start_index, end_index, step, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
  // Make sure that 90% of the tests actually were within limits.
  // I.e. udivResultFitsInDivisor in coreTest returned true
  TEST_ASSERT_INT32_WITHIN(coreTestCount/10U, coreTestCount, divTestCount);
}

static void test_udivHiQi2_vs_udivSiHi2_perf(void)
{
  constexpr uint16_t iters = 32;
  constexpr uint8_t step = 1;
  constexpr uint8_t start_index = 1;
  constexpr uint8_t end_index = UINT8_MAX - step;

  static uint32_t coreTestCount = 0U;
  static uint32_t divTestCount = 0U;

  coreTestCount = 0U;
  divTestCount = 0U;
  static auto coreTest = [] (uint8_t index, uint16_t(*pDiv)(uint16_t, uint8_t), uint32_t &checkSum) { 
    uint16_t dividend = generateDividendFromDivisorU<uint8_t, uint16_t>(index, start_index, end_index, 12U);
    ++coreTestCount;
    // We need to protect udivHiQi2 from spurious input.
    if (optimized_div_impl::udivResultFitsInDivisor(dividend, index)) {
      checkSum += pDiv(dividend, index); 
      ++divTestCount;
    }
  };

  auto optimizedTest = [] (uint8_t index, uint32_t &checkSum) { 
    static auto divFunc = [](uint16_t dividend, uint8_t divisor) -> uint16_t {
      return optimized_div_impl::udivHiQi2(dividend, divisor).quot;
    };
    coreTest(index, divFunc, checkSum);    
  };

  auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { 
    static auto divFunc = [](uint16_t dividend, uint8_t divisor) -> uint16_t {
      return optimized_div_impl::udivSiHi2(dividend, divisor).quot;
    };
    coreTest(index, divFunc, checkSum);
  };    
  auto comparison = compare_executiontime<uint8_t, uint32_t>(iters, start_index, end_index, step, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);
  // Make sure that 90% of the tests actually were within limits.
  // I.e. udivResultFitsInDivisor in coreTest returned true
  TEST_ASSERT_INT32_WITHIN(coreTestCount/10U, coreTestCount, divTestCount);
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