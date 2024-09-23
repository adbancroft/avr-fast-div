#include <Arduino.h>
#include <unity.h>
#include "avr-fast-div.h"
#include "../lambda_timer.hpp"
#include "../unity_print_timers.hpp"
#include "../test_utils.h"
#include "../index_range_generator.hpp"
#include "performance_test.h"

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
static void performance_test(uint16_t iters, const index_range_generator<T> &dividendRange, const index_range_generator<U> &divisorRange, uint8_t percentExpected) {
  // Because we don't have std::function, we have to use function pointers
  // and static variables to pass the test functions to the comparison functions.
  static const index_range_generator<T> *pDividendRange;
  pDividendRange = &dividendRange;
  static const index_range_generator<U> *pDivisorRange;
  pDivisorRange = &divisorRange;

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += pDividendRange->generate(index) / pDivisorRange->generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += fast_div(pDividendRange->generate(index), pDivisorRange->generate(index));
  };
  performance_test(iters, dividendRange, divisorRange, nativeTest, optimizedTest, percentExpected); 
}

static void test_fast_div_perf_u16_u8_optimal(void)
{
  // Tests the optimal scenario: all results of u16/u8 fit into a u8
  static constexpr index_range_generator<uint8_t> divisorGen(2U, UINT8_MAX, (UINT8_MAX/2U)-1U);
  static constexpr index_range_generator<uint16_t> dividendGen = create_optimal_dividend_range<uint8_t, uint16_t>(divisorGen); 

#if defined(UNOPTIMIZED_BUILD)  
  constexpr uint8_t percentExpected = 75;
#else
  constexpr uint8_t percentExpected = 40;
#endif 
  performance_test(64, dividendGen, divisorGen, percentExpected);
}

static void test_fast_div_perf_u16_u8_worst_case(void)
{
  // Tests the worst case scenario: none results of u16/u8 fit into a u8
  index_range_generator<uint8_t> divisorGen(2U, UINT8_MAX-2U, UINT8_MAX-4U);
  index_range_generator<uint16_t> dividendGen(UINT16_MAX/divisorGen.rangeMax()*3, UINT16_MAX, divisorGen.num_steps());

#if defined(UNOPTIMIZED_BUILD)
  constexpr uint8_t percentExpected = 111;
#else
  constexpr uint8_t percentExpected = 110;
#endif 
  performance_test(32, dividendGen, divisorGen, percentExpected);
}

static void test_fast_div_perf_u32_u8(void)
{
  static constexpr index_range_generator<uint8_t> divisorGen(2U, UINT8_MAX-2U, UINT8_MAX-4U);
  static constexpr index_range_generator<uint32_t> dividendGen((uint32_t)divisorGen.rangeMax()*2ULL, ((uint32_t)UINT16_MAX+2)*24ULL, divisorGen.num_steps()); 

#if defined(UNOPTIMIZED_BUILD)
  constexpr uint8_t percentExpected = 60;
#else
  constexpr uint8_t percentExpected = 35;
#endif 
  performance_test(16, dividendGen, divisorGen, percentExpected);
}

static void test_fast_div_perf_u32_u16_optimal(void)
{
  // Tests the optimal scenario: all results of u32/u16 fit into a u16
  static constexpr index_range_generator<uint16_t> divisorGen(2U, UINT16_MAX, 333U);
  static constexpr index_range_generator<uint32_t> dividendGen = create_optimal_dividend_range<uint16_t, uint32_t>(divisorGen); 

#if defined(UNOPTIMIZED_BUILD)
  constexpr uint8_t percentExpected = 60;
#else
  constexpr uint8_t percentExpected = 35;
#endif 
  performance_test(12, dividendGen, divisorGen, percentExpected);
}

static void test_fast_div_perf_u32_u16_worst_case(void)
{
  // Tests the worst case scenario: none results of u32/u16 fit into a u16
  static constexpr index_range_generator<uint16_t> divisorGen(2U, UINT16_MAX-2U, 333U);
  static constexpr index_range_generator<uint32_t> dividendGen(divisorGen.rangeMax()*2ULL, UINT32_MAX, divisorGen.num_steps());

#if defined(UNOPTIMIZED_BUILD)
  constexpr uint8_t percentExpected = 105;
#else
  constexpr uint8_t percentExpected = 105;
#endif 
  performance_test(11, dividendGen, divisorGen, percentExpected);
}

static void test_fast_div_perf_u16_u16(void)
{
  static constexpr index_range_generator<uint16_t> divisorGen(1U, UINT16_MAX/2U, 3333U);
  static constexpr index_range_generator<uint16_t> dividendGen(divisorGen.rangeMax()+1U, UINT16_MAX, divisorGen.num_steps());

#if defined(UNOPTIMIZED_BUILD)
#if defined(AFD_SMALL_TEXT)
  constexpr uint8_t percentExpected = 70;
#else
  constexpr uint8_t percentExpected = 70;
#endif
#else
#if defined(AFD_SMALL_TEXT)
  constexpr uint8_t percentExpected = 50;
#else
  constexpr uint8_t percentExpected = 45;
#endif
#endif 
  performance_test(3, dividendGen, divisorGen, percentExpected);
}

static void test_fast_div_perf_u32_u32(void)
{
  static constexpr index_range_generator<uint32_t> divisorGen(UINT16_MAX, UINT32_MAX/33UL, 3333U);
  static constexpr index_range_generator<uint32_t> dividendGen((UINT32_MAX/33UL)*2ULL, UINT32_MAX, divisorGen.num_steps());

#if defined(UNOPTIMIZED_BUILD)
  constexpr uint8_t percentExpected = 80;
#else
  constexpr uint8_t percentExpected = 65;
#endif 
  performance_test(1, dividendGen, divisorGen, percentExpected);
}

static void test_fast_div_perf_s32_s16_optimal(void)
{
  static constexpr index_range_generator<int16_t> divisorGen(INT16_MIN+1L, INT16_MAX, 3333U);
  static constexpr index_range_generator<int32_t> dividendGen(INT16_MIN*2L, INT16_MAX*2L, divisorGen.num_steps());

#if defined(UNOPTIMIZED_BUILD)
  constexpr uint8_t percentExpected = 60;
#else
  constexpr uint8_t percentExpected = 45;
#endif 
  performance_test(1, dividendGen, divisorGen, percentExpected);
}

static void test_fast_div_perf_s32_s16_worst_case(void)
{
  static constexpr index_range_generator<int16_t> divisorGen(1, INT16_MAX, 99U);
  static constexpr index_range_generator<int32_t> dividendGen(divisorGen.rangeMax()*3LL, INT32_MAX/55LL, divisorGen.num_steps());

#if defined(UNOPTIMIZED_BUILD)
#if defined(AFD_SMALL_TEXT)
  constexpr uint8_t percentExpected = 65;
#else
  constexpr uint8_t percentExpected = 60;
#endif
#else
#if defined(AFD_SMALL_TEXT)
  constexpr uint8_t percentExpected = 50;
#else
  constexpr uint8_t percentExpected = 45;
#endif
#endif 
  performance_test(32, dividendGen, divisorGen, percentExpected);
}


void test_fast_div_performance(void) {
   SET_UNITY_FILENAME() {
      RUN_TEST(test_fast_div_perf_u16_u8_optimal);
      RUN_TEST(test_fast_div_perf_u16_u8_worst_case);
      RUN_TEST(test_fast_div_perf_u16_u16);
      RUN_TEST(test_fast_div_perf_u32_u8);
      RUN_TEST(test_fast_div_perf_u32_u16_optimal);
      RUN_TEST(test_fast_div_perf_u32_u16_worst_case);
      RUN_TEST(test_fast_div_perf_u32_u32);
      RUN_TEST(test_fast_div_perf_s32_s16_optimal);
      RUN_TEST(test_fast_div_perf_s32_s16_worst_case);
  }
}