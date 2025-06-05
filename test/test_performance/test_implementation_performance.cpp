#include <Arduino.h>
#include <unity.h>
#include "avr-fast-div.h"
#include "../lambda_timer.hpp"
#include "../unity_print_timers.hpp"
#include "../test_utils.h"
#include "performance_test.h"
#include "afd_implementation.hpp"

// The performance tests here focus on the internal optimized division functions.
// The tests use highly constrained number ranges to avoid overflow.
//
// This also showcases the maximum performance improvement.
#if defined(__AVR__)

// Given an input divisor range, create a dividend range that will comply with our
// optimization restriction: the result of a/b must fit in the type of b.
// The ranges will not overlap
template <typename TDivisor, typename TDividend>
static constexpr index_range_generator<TDividend> create_optimal_dividend_range(const index_range_generator<TDivisor> &in_range) {
  return index_range_generator<TDividend>((TDividend)in_range.rangeMax()*(TDividend)in_range.rangeMin(), 
                                          (TDividend)in_range.rangeMax()*(TDividend)in_range.rangeMax(), 
                                          in_range.num_steps()); 
}


// divide -> u32/u16 => u18
static void test_divide_optimised_u32u16_vs_u32u32_perf(void)
{
  static constexpr index_range_generator<uint16_t> divisorGen(2U, UINT16_MAX, 333U);
  static constexpr index_range_generator<uint32_t> dividendGen = create_optimal_dividend_range<uint16_t, uint32_t>(divisorGen); 

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += avr_fast_div_impl::divide(dividendGen.generate(index), divisorGen.generate(index)) & 0x0000FFFFU;
  };
#if AFD_INLINE==AFD_INLINE_FORCE_OFF || AFD_INLINE==AFD_INLINE_NO
  constexpr uint16_t percentExpected = 55;
#else
  constexpr uint16_t percentExpected = 30;
#endif    
  performance_test(8U, dividendGen, divisorGen, nativeTest, optimizedTest, percentExpected); 
}

// udivHiQi2 -> u16/u8 => u8
static void test_divide_optimisedu16u8_vs_u16u16_perf(void)
{
  static constexpr index_range_generator<uint8_t> divisorGen(2U, UINT8_MAX, (UINT8_MAX/2U)-1U);
  static constexpr index_range_generator<uint16_t> dividendGen = create_optimal_dividend_range<uint8_t, uint16_t>(divisorGen); 

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += avr_fast_div_impl::divide(dividendGen.generate(index), divisorGen.generate(index)) & 0x00FFU;
  };
#if AFD_INLINE==AFD_INLINE_FORCE_OFF || AFD_INLINE==AFD_INLINE_NO
  constexpr uint16_t percentExpected = 70;
#else
  constexpr uint16_t percentExpected = 35;
#endif    
  performance_test(64U, dividendGen, divisorGen, nativeTest, optimizedTest, percentExpected); 
}

static void test_divide_optimised16u8_vs_divide_optimisedu32u16_perf(void)
{
  constexpr uint16_t steps = (UINT8_MAX/2U)-1U;
  static constexpr index_range_generator<uint8_t> divisorGen(2U, UINT8_MAX, steps);
  static constexpr index_range_generator<uint16_t> dividendGen = create_optimal_dividend_range<uint8_t, uint16_t>(divisorGen); 

  static auto u3216Test = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += avr_fast_div_impl::divide((uint32_t)dividendGen.generate(index), (uint16_t)divisorGen.generate(index)) & 0x0000FFFFU;
  };
  static auto u16u8Test = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += avr_fast_div_impl::divide(dividendGen.generate(index), divisorGen.generate(index)) & 0x00FFU;
  };
#if AFD_INLINE==AFD_INLINE_FORCE_OFF || AFD_INLINE==AFD_INLINE_NO
  constexpr uint16_t percentExpected = 60;
#else
  constexpr uint16_t percentExpected = 40;
#endif    
  performance_test(96U, dividendGen, divisorGen, u3216Test, u16u8Test, percentExpected); 
}

static void test_divide_large_divisor16u16_vs_u16u16_perf(void)
{
  static constexpr index_range_generator<uint16_t> divisorGen((uint16_t)UINT8_MAX+2U, UINT16_MAX/4, 399);
  static constexpr index_range_generator<uint16_t> dividendGen(divisorGen.rangeMax(), UINT16_MAX, divisorGen.num_steps()); 

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += avr_fast_div_impl::divide_large_divisor<uint16_t>(dividendGen.generate(index), divisorGen.generate(index));
  };
#if AFD_INLINE==AFD_INLINE_FORCE_OFF || AFD_INLINE==AFD_INLINE_NO
  constexpr uint16_t percentExpected = 65;
#else
  constexpr uint16_t percentExpected = 45;
#endif  
  performance_test(32U, dividendGen, divisorGen, nativeTest, optimizedTest, percentExpected); 
}

static void test_divide_large_divisor32u32_vs_u32u32_perf(void)
{
  static constexpr index_range_generator<uint32_t> divisorGen(UINT32_MAX/30, UINT32_MAX/11UL, 399U);
  static constexpr index_range_generator<uint32_t> dividendGen(UINT32_MAX/20, UINT32_MAX/5UL, divisorGen.num_steps()); 

  static auto nativeTest = [] (uint16_t index, uint32_t &checkSum) { 
    checkSum += dividendGen.generate(index) / divisorGen.generate(index);
  };
  static auto optimizedTest = [] (uint16_t index, uint32_t &checkSum) {
    checkSum += avr_fast_div_impl::divide_large_divisor<uint32_t>(dividendGen.generate(index), divisorGen.generate(index));
  };
#if AFD_INLINE==AFD_INLINE_FORCE_OFF || AFD_INLINE==AFD_INLINE_NO
  constexpr uint16_t percentExpected = 50;
#else
  constexpr uint16_t percentExpected = 20;
#endif  
  performance_test(16U, dividendGen, divisorGen, nativeTest, optimizedTest, percentExpected); 
}


#endif

void test_implementation_performance(void) {
#if defined(__AVR__)
   SET_UNITY_FILENAME() {
        RUN_TEST(test_divide_optimised_u32u16_vs_u32u32_perf);
        RUN_TEST(test_divide_optimisedu16u8_vs_u16u16_perf);
        RUN_TEST(test_divide_optimised16u8_vs_divide_optimisedu32u16_perf);
        RUN_TEST(test_divide_large_divisor16u16_vs_u16u16_perf);
        RUN_TEST(test_divide_large_divisor32u32_vs_u32u32_perf);
   }
#endif
}