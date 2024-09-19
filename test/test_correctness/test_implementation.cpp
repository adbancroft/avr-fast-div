#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "avr-fast-div.h"

#if defined(__AVR__)

static constexpr uint32_t MICROS_PER_SEC = 1000000;
static constexpr uint32_t MICROS_PER_MIN = MICROS_PER_SEC*60U;
static constexpr uint32_t MICROS_PER_HOUR = MICROS_PER_MIN*60U;

static void assert_divide_u32u16(uint32_t dividend, uint16_t divisor) {
    auto native = ldiv(dividend, divisor);
    auto optimised = avr_fast_div_impl::divide<uint32_t, uint16_t>(dividend, divisor);
    char msgBuffer[128];
    sprintf(msgBuffer, "%" PRIu32 ", %" PRIu16, dividend, divisor);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(native.quot, optimised & 0x0000FFFFU, msgBuffer);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(native.rem, (uint16_t)(optimised >> 16U), msgBuffer);
}

static void test_divide_u32u16(void)
{
  assert_divide_u32u16(1, 1);
  // assert_divide_u32u16(UINT32_MAX, 1U); // quotient will not it into U16 
  assert_divide_u32u16(UINT32_MAX/2UL, UINT16_MAX);
  // assert_divide_u32u16(UINT32_MAX, UINT16_MAX); // quotient will not it into U16 
  assert_divide_u32u16((uint32_t)UINT16_MAX+1UL, UINT16_MAX);
  assert_divide_u32u16((uint32_t)UINT16_MAX-1UL, UINT16_MAX);
  assert_divide_u32u16((uint32_t)UINT16_MAX*3U, (UINT16_MAX/4U)*3U);
  assert_divide_u32u16(MICROS_PER_MIN, 60000); // 1000 RPM
  assert_divide_u32u16(MICROS_PER_MIN, 54005); // 1111 RPM
  assert_divide_u32u16(MICROS_PER_MIN, 7590);  // 7905 RPM
  assert_divide_u32u16(MICROS_PER_MIN, 7715);  // 7777 RPM  
  assert_divide_u32u16(MICROS_PER_MIN, 3333);  // 18000 RPM  
}

static void assert_divide_u16u8(uint16_t dividend, uint8_t divisor) {
  auto native = div(dividend, divisor);
  auto optimised = avr_fast_div_impl::divide<uint16_t, uint8_t>(dividend, divisor);
  char msgBuffer[128];
  sprintf(msgBuffer, "%" PRIu16 ", %" PRIu8, dividend, divisor);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(native.quot, optimised & 0x00FFU, msgBuffer);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(native.rem, (uint16_t)(optimised >> 8U), msgBuffer);
}

static void test_divide_u16u8(void)
{
  assert_divide_u32u16(1, 1);
  assert_divide_u32u16(UINT16_MAX, 1);
  assert_divide_u32u16(UINT16_MAX, UINT16_MAX);
  assert_divide_u32u16(UINT16_MAX, UINT8_MAX);
  assert_divide_u16u8(UINT8_MAX+1, UINT8_MAX);
  assert_divide_u16u8(UINT8_MAX-1, UINT8_MAX);

  // Below are from an idle target table in a real tune
  assert_divide_u16u8(150, 30); 
  assert_divide_u16u8(70, 14);
  assert_divide_u16u8(60, 25);
  assert_divide_u16u8(40, 9);

  // Artificial
  assert_divide_u16u8(UINT8_MAX*7-1, 7); 
}

template <typename T>
static void assert_divide_large_divisor(T dividend, T divisor) {
  auto native = dividend / divisor;
  auto optimised = avr_fast_div_impl::divide_large_divisor(dividend, divisor);
  char msgBuffer[128];
  sprintf(msgBuffer, "%" PRIu32", %" PRIu32, (uint32_t)dividend, (uint32_t)divisor);
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(native, optimised, msgBuffer);
}


static void test_divide_large_divisor_u32u32(void) {
  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint32_t>(UINT32_MAX, 1), 0);
  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint32_t>(UINT32_MAX, UINT8_MAX), 0);
  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint32_t>(UINT32_MAX, UINT16_MAX), 0);
  assert_divide_large_divisor<uint32_t>(UINT32_MAX, UINT16_MAX+1UL);
  assert_divide_large_divisor<uint32_t>(UINT32_MAX, UINT32_MAX/2U);
  assert_divide_large_divisor<uint32_t>(UINT32_MAX, UINT32_MAX);

  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint32_t>(UINT16_MAX, 1), 0);
  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint32_t>(UINT16_MAX, UINT16_MAX-1U), 0);
  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint32_t>(UINT16_MAX, UINT16_MAX), 0);
}

static void test_divide_large_divisor_u16u16(void) {
  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint16_t>(UINT16_MAX, 1), 0);
  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint16_t>(UINT16_MAX, UINT8_MAX), 0);
  assert_divide_large_divisor<uint16_t>(UINT16_MAX, UINT8_MAX+1UL);
  assert_divide_large_divisor<uint16_t>(UINT16_MAX, UINT16_MAX/2);
  assert_divide_large_divisor<uint16_t>(UINT16_MAX, UINT16_MAX);

  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint16_t>(UINT8_MAX, 1), 0);
  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint16_t>(UINT8_MAX, UINT8_MAX-1U), 0);
  TEST_ASSERT_EQUAL_UINT32(avr_fast_div_impl::divide_large_divisor<uint16_t>(UINT8_MAX, UINT8_MAX), 0);
}
#endif

void test_implementation_details(void) {
 #if defined(__AVR__)
   SET_UNITY_FILENAME() {
        RUN_TEST(test_divide_u32u16);
        RUN_TEST(test_divide_u16u8);
        RUN_TEST(test_divide_large_divisor_u32u32);
        RUN_TEST(test_divide_large_divisor_u16u16);
    }
#endif
}