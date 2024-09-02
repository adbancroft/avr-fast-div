#include <Arduino.h>
#include <unity.h>
#include "avr-fast-div.h"
#include "lambda_timer.hpp"
#include "unity_print_timers.hpp"
#include "test_utils.h"
#include "index_range_generator.hpp"

// The performance tests here focus on the public fast_div() family of functions.
// The tests either use constrained number ranges to showcase the performance
// improvement or worst case scenarios to show how minimal the overhead is.

template <typename T, typename U>
static constexpr index_range_generator<U> create_optimal_dividend_range(const index_range_generator<T> &in_range) {
  return index_range_generator<U>((U)in_range.rangeMax()*(U)in_range.rangeMin(), 
                                  (U)in_range.rangeMax()*(U)in_range.rangeMax(), 
                                  in_range.num_steps()); 
}

template <typename T, typename U>
static constexpr index_range_generator<U> create_worst_case_dividend_range(const index_range_generator<T> &in_range, U minValue) {
  return index_range_generator<U>(minValue*in_range.rangeMin(),
                                  minValue*in_range.rangeMax(), 
                                  in_range.num_steps()); 
}

static void test_fast_div_perf_u16_u8_optimal(void)
{
  // Tests the optimal scenario: all results of u16/u8 fit into a u8
  static constexpr index_range_generator<uint8_t> divisorGen(2U, UINT8_MAX, (UINT8_MAX/2U)-1U);
  static constexpr index_range_generator<uint16_t> dividendGen = create_optimal_dividend_range<uint8_t, uint16_t>(divisorGen); 

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(32U, 0U, divisorGen.num_steps(), 1U, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // We only expect a speed improvement on AVR
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
#endif
}

#if defined(DEBUG)
template <typename T>
void dump(const index_range_generator<T> &gen) {
  Serial.print(gen.rangeMin());
  Serial.print(", ");
  Serial.print(gen.rangeMax());
  Serial.print(", ");
  Serial.print(gen.step_size());
  Serial.print(", ");
  Serial.println(gen.num_steps());
}
#endif

static void test_fast_div_perf_u16_u8_worst_case(void)
{
  // Tests the worst case scenario: none results of u16/u8 fit into a u8
  static constexpr index_range_generator<uint8_t> divisorGen(2U, UINT8_MAX-2U, UINT8_MAX-4U);
  static constexpr index_range_generator<uint16_t> dividendGen = create_worst_case_dividend_range<uint8_t, uint16_t>(divisorGen, UINT8_MAX+1U);

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(32U, 0U, divisorGen.num_steps(), 1U, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(UNOPTIMIZED_BUILD)  
  // Should be very close to the native speed; use a 11% margin
  auto margin = comparison.timeA.timer.duration_micros()/9U;
#else
  // Should be very close to the native speed; use a 3% margin
  auto margin = comparison.timeA.timer.duration_micros()/33U;
#endif
  TEST_ASSERT_UINT32_WITHIN(margin, comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

static void test_fast_div_perf_u32_u16_optimal(void)
{
  // Tests the optimal scenario: all results of u32/u16 fit into a u16
  static constexpr index_range_generator<uint16_t> divisorGen(2U, UINT16_MAX, 333U);
  static constexpr index_range_generator<uint32_t> dividendGen = create_optimal_dividend_range<uint16_t, uint32_t>(divisorGen); 

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(32U, 0U, divisorGen.num_steps(), 1U, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__) // We only expect a speed improvement on AVR
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
#endif
}

static void test_fast_div_perf_u32_u16_worst_case(void)
{
  // Tests the worst case scenario: none results of u32/u16 fit into a u16
  static constexpr index_range_generator<uint16_t> divisorGen(2U, UINT16_MAX-2U, 333U);
  static constexpr index_range_generator<uint32_t> dividendGen = create_worst_case_dividend_range<uint16_t, uint32_t>(divisorGen, UINT16_MAX+1UL);

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(32U, 0U, divisorGen.num_steps(), 1U, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(UNOPTIMIZED_BUILD)  
  // Should be very close to the native speed; use a 10% margin
  auto margin = comparison.timeA.timer.duration_micros()/10U;
#else
  // Should be very close to the native speed; use a 3% margin
  auto margin = comparison.timeA.timer.duration_micros()/33U;
#endif
  TEST_ASSERT_UINT32_WITHIN(margin, comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

static void test_fast_div_perf_u16_u16(void)
{
  static constexpr index_range_generator<uint16_t> divisorGen(1U, UINT16_MAX/2U, 3333U);
  static constexpr index_range_generator<uint16_t> dividendGen(divisorGen.rangeMax()+1U, UINT16_MAX, divisorGen.num_steps());

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(0U, divisorGen.num_steps(), 1U, nativeTest, optimizedTest);
  
  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  // The u16/u16 case my be slightly slower than the native operation, since none of the vast majority of the 
  // results will not fit in u8.

#if defined(UNOPTIMIZED_BUILD)  
  // Should be very close to the native speed; use a 10% margin
  auto margin = comparison.timeA.timer.duration_micros()/10U;
#else
  // Should be very close to the native speed; use a 3% margin
  auto margin = comparison.timeA.timer.duration_micros()/33U;
#endif
  TEST_ASSERT_UINT32_WITHIN(margin, comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

static void test_fast_div_perf_u32_u32(void)
{
  static constexpr index_range_generator<uint32_t> divisorGen(1UL, UINT32_MAX/2UL, 3333U);
  static constexpr index_range_generator<uint32_t> dividendGen(divisorGen.rangeMin()+1UL, UINT32_MAX, divisorGen.num_steps());

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(0U, divisorGen.num_steps(), 1U, nativeTest, optimizedTest);

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  // The u32/u32 case will be about the same speed the native operation, since the vast majority of the 
  // results will not fit into uint16_t. It all depends on the number ranges!

#if defined(UNOPTIMIZED_BUILD)  
  // Should be very close to the native speed; use a 10% margin
  auto margin = comparison.timeA.timer.duration_micros()/10U;
#else
  // Should be very close to the native speed; use a 3% margin
  auto margin = comparison.timeA.timer.duration_micros()/33U;
#endif
  TEST_ASSERT_UINT32_WITHIN(margin, comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}

static void test_fast_div_perf_s32_s16_optimal(void)
{
  static constexpr index_range_generator<int16_t> divisorGen(INT16_MIN, INT16_MAX, 3333U);
  static constexpr index_range_generator<int32_t> dividendGen(INT16_MIN*2UL, INT16_MAX*2UL, divisorGen.num_steps());

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(0U, divisorGen.num_steps(), 1U, nativeTest, optimizedTest);  

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

#if defined(__AVR__)
  TEST_ASSERT_LESS_THAN(comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
#endif
}

static void test_fast_div_perf_s32_s16_worst_case(void)
{
  static constexpr index_range_generator<int16_t> divisorGen(1, INT16_MAX, 99U);
  static constexpr index_range_generator<int32_t> dividendGen = create_worst_case_dividend_range<int16_t, int32_t>(divisorGen, ((int32_t)INT16_MAX+1)*2);

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(dividendGen.generate(index), divisorGen.generate(index));
  };
  auto comparison = compare_executiontime<uint16_t, uint32_t>(32, 0U, divisorGen.num_steps(), 1U, nativeTest, optimizedTest);  

  MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
  TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  // The s32/s16 case my be slightly slower than the native operation, since none of 
  // results will not fit in u16. 
#if defined(UNOPTIMIZED_BUILD)  
  // Should be very close to the native speed; use a 10% margin
  auto margin = comparison.timeA.timer.duration_micros()/10U;
#else
  // Should be very close to the native speed; use a 3% margin
  auto margin = comparison.timeA.timer.duration_micros()/33U;
#endif
  TEST_ASSERT_UINT32_WITHIN(margin, comparison.timeA.timer.duration_micros(), comparison.timeB.timer.duration_micros());
}


void test_fast_div_performance(void) {
   SET_UNITY_FILENAME() {
      RUN_TEST(test_fast_div_perf_u16_u8_optimal);
      RUN_TEST(test_fast_div_perf_u16_u8_worst_case);
      RUN_TEST(test_fast_div_perf_u16_u16);
      RUN_TEST(test_fast_div_perf_u32_u16_optimal);
      RUN_TEST(test_fast_div_perf_u32_u16_worst_case);
      RUN_TEST(test_fast_div_perf_u32_u32);
      RUN_TEST(test_fast_div_perf_s32_s16_optimal);
      RUN_TEST(test_fast_div_perf_s32_s16_worst_case);
  }
}