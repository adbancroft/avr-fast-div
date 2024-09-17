#pragma once

#include "../index_range_generator.hpp"

#if defined(DETAILED_MESSAGES)
template <typename T>
void dump(const index_range_generator<T> &gen) {
  if (type_traits::is_signed<T>::value) {
    TEST_PRINTF("%ld, %ld, %lu, %ld", (int32_t)gen.rangeMin(), (int32_t)gen.rangeMax(), (uint32_t)gen.step_size(), (uint32_t)gen.num_steps());
  } else {
    TEST_PRINTF("%lu, %lu, %lu, %ld", (uint32_t)gen.rangeMin(), (uint32_t)gen.rangeMax(), (uint32_t)gen.step_size(), (uint32_t)gen.num_steps());
  }
}
#endif

template <typename T, typename U>
static inline void performance_test(uint16_t iters, 
  const index_range_generator<T> &dividendRange, 
#if defined(DETAILED_MESSAGES)
  const index_range_generator<U> &divisorRange, 
#else
  const index_range_generator<U> &, 
#endif
  void (*pTestFunA)(uint16_t, uint32_t&), 
  void (*pTestFunB)(uint16_t, uint32_t&),
  uint8_t percentExpected) {
SET_UNITY_FILENAME() {    

  #if defined(DETAILED_MESSAGES)
    TEST_MESSAGE("dividendRange");
    dump(dividendRange);
    TEST_MESSAGE("divisorRange");
    dump(divisorRange);
  #endif 

    auto comparison = compare_executiontime<uint16_t, uint32_t>(iters, 0U, dividendRange.num_steps(), 1U, pTestFunA, pTestFunB);
    
    MESSAGE_TIMERS(comparison.timeA.timer, comparison.timeB.timer);
    TEST_ASSERT_EQUAL(comparison.timeA.result, comparison.timeB.result);

  #if (__AVR__)
    auto expectedTime = (comparison.timeA.timer.duration_micros()/100U)*percentExpected;

    TEST_ASSERT_LESS_THAN(expectedTime, comparison.timeB.timer.duration_micros());
  #endif
  }
}