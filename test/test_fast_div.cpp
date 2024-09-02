#include <Arduino.h>
#include <unity.h>
#include "test_utils.h"
#include "avr-fast-div.h"

// Wrap up the assertion that a/b==fast_div(a,b)
// and provide pretty error messages
template <typename TDividend, typename TDivisor>
static void assert_fastdiv(TDividend dividend, TDivisor divisor) {
  if (divisor!=0) { // Division by zero on Teensy generates an exception
    TDividend expected = (TDividend)(dividend/divisor);
    TDividend actual = fast_div(dividend, divisor);
    
    char msgBuffer[256];
    if (dividend>INT32_MAX || divisor>INT32_MAX) {
      sprintf(msgBuffer, "%s: %" PRIu32 ", %" PRIu32, __PRETTY_FUNCTION__, (uint32_t)dividend, (uint32_t)divisor);
    } else {
      sprintf(msgBuffer, "%s: %" PRId32 ", %" PRId32, __PRETTY_FUNCTION__, (int32_t)dividend, (int32_t)divisor);
    }
    TEST_ASSERT_EQUAL_INT32_MESSAGE(expected, actual, msgBuffer);
  }
}

// Test fast div by type & range
template <typename TDividend, typename TDivisor>
static void assert_fastdiv_range(TDividend divMin, TDividend divMax, TDivisor divisorMin, TDivisor divisorMax) {
  // Test the corners
  assert_fastdiv(divMax, divisorMin);
  assert_fastdiv(divMin, divisorMax);
  assert_fastdiv(divMax, divisorMax);
  assert_fastdiv(divMin, divisorMin);

  // Test the middle
  assert_fastdiv<TDividend, TDivisor>((TDividend)(divMin+((divMax-divMin)/2)), 
                                      (TDivisor)(divisorMin+((divisorMax-divisorMin)/2)));
}


static void test_fast_div_8(void) {
  // i8/i8
  assert_fastdiv_range((int8_t)INT8_MIN, (int8_t)INT8_MAX,   (int8_t)INT8_MIN, (int8_t)INT8_MAX);
  // u8/u8
  assert_fastdiv_range((uint8_t)0U,      (uint8_t)UINT8_MAX,   (uint8_t)1U,      (uint8_t)UINT8_MAX);
}

static void test_fast_div_16(void) {
  // i16/i16
  assert_fastdiv_range((int16_t)INT16_MIN, (int16_t)INT16_MAX, (int16_t)INT16_MIN, (int16_t)INT16_MAX);
  // u16/u16
  assert_fastdiv_range((uint16_t)0U, (uint16_t)UINT16_MAX, (uint16_t)1U, (uint16_t)UINT16_MAX);
  // i16/i8
  assert_fastdiv_range((int16_t)INT16_MIN, (int16_t)INT16_MAX, (int8_t)INT8_MIN, (int8_t)INT8_MAX);
  // u16/u8
  assert_fastdiv_range((uint16_t)0U, (uint16_t)UINT16_MAX, (uint8_t)1U, (uint8_t)UINT8_MAX);
}

static void test_fast_div_32(void) {
  // i32/i32
  assert_fastdiv_range((int32_t)INT32_MIN, (int32_t)INT32_MAX, (int32_t)INT32_MIN, (int32_t)INT32_MAX);
  // i32/i16
  assert_fastdiv_range((int32_t)INT32_MIN, (int32_t)INT32_MAX, (int16_t)INT16_MIN, (int16_t)INT16_MAX);
  // i32/i8
  assert_fastdiv_range((int32_t)INT32_MIN, (int32_t)INT32_MAX, (int8_t)INT8_MIN, (int8_t)INT8_MAX);
  // u32/u32
  assert_fastdiv_range((uint32_t)0U, (uint32_t)UINT32_MAX, (uint32_t)1U, (uint32_t)UINT32_MAX);
  // u32/u16
  assert_fastdiv_range((uint32_t)0U, (uint32_t)UINT32_MAX, (uint16_t)1U, (uint16_t)UINT16_MAX);
  // u32/u8
  assert_fastdiv_range((uint32_t)0U, (uint32_t)UINT32_MAX, (uint8_t)1U, (uint8_t)UINT8_MAX);
}

void test_fast_div(void) {
    SET_UNITY_FILENAME() {
        RUN_TEST(test_fast_div_8);
        RUN_TEST(test_fast_div_16);
        RUN_TEST(test_fast_div_32);
    }
}