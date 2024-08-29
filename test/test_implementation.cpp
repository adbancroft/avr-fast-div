#include <Arduino.h>
#include <unity.h>
#include "test_utils.h"
#include "avr_fast_div.h"

#if defined(__AVR__)

static constexpr uint32_t MICROS_PER_SEC = 1000000;
static constexpr uint32_t MICROS_PER_MIN = MICROS_PER_SEC*60U;
static constexpr uint32_t MICROS_PER_HOUR = MICROS_PER_MIN*60U;

static void assert_udivSiHi2(uint32_t dividend, uint16_t divisor) {
    auto native = ldiv(dividend, divisor);
    auto optimised = optimized_div_impl::udivSiHi2(dividend, divisor);
    TEST_ASSERT_EQUAL_UINT16(native.quot, optimised.quot);
    TEST_ASSERT_EQUAL_UINT16(native.rem, optimised.rem);
}

static void test_udivSiHi2(void)
{
  // Divide by zero
  TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, optimized_div_impl::udivSiHi2(0, 0).quot);

  // Result doesn't fit into 16-bits
  TEST_ASSERT_EQUAL_UINT16((UINT16_MAX/2)+1, optimized_div_impl::udivSiHi2(UINT32_MAX, UINT16_MAX).quot);

  assert_udivSiHi2(1, 1);
  assert_udivSiHi2(UINT16_MAX+1, UINT16_MAX);
  assert_udivSiHi2(UINT16_MAX-1, UINT16_MAX);
  assert_udivSiHi2(MICROS_PER_MIN, 60000); // 1000 RPM
  assert_udivSiHi2(MICROS_PER_MIN, 54005); // 1111 RPM
  assert_udivSiHi2(MICROS_PER_MIN, 7590);  // 7905 RPM
  assert_udivSiHi2(MICROS_PER_MIN, 7715);  // 7777 RPM  
  assert_udivSiHi2(MICROS_PER_MIN, 3333);  // 18000 RPM  
}

static void assert_udivHiQi2(uint16_t dividend, uint8_t divisor) {
  auto native = div(dividend, divisor);
  auto optimised = optimized_div_impl::udivHiQi2(dividend, divisor);
  TEST_ASSERT_EQUAL_UINT16(native.quot, optimised.quot);
  TEST_ASSERT_EQUAL_UINT16(native.rem, optimised.rem);    
}

static void test_udivHiQi2(void)
{
  // Divide by zero
  TEST_ASSERT_EQUAL_UINT16(UINT8_MAX, optimized_div_impl::udivHiQi2(0, 0).quot);

  // Result doesn't fit into 8-bits
  TEST_ASSERT_EQUAL_UINT16((UINT8_MAX/2U)+1U, optimized_div_impl::udivHiQi2(UINT16_MAX, UINT8_MAX).quot);

  assert_udivHiQi2(1, 1);
  assert_udivHiQi2(UINT8_MAX+1, UINT8_MAX);
  assert_udivHiQi2(UINT8_MAX-1, UINT8_MAX);
  // Below are from an idle target table in a real tune
  assert_udivHiQi2(150, 30); 
  assert_udivHiQi2(70, 14);
  assert_udivHiQi2(60, 25);
  assert_udivHiQi2(40, 9);
  // Artificial
  assert_udivHiQi2(UINT8_MAX*7-1, 7); 
}

#endif

void test_implementation_details(void) {
 #if defined(__AVR__)
   SET_UNITY_FILENAME() {
        RUN_TEST(test_udivSiHi2);
        RUN_TEST(test_udivHiQi2);
    }
#endif
}