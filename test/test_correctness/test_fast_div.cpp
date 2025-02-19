#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "avr-fast-div.h"

namespace afd_type_traits {
  
  template <typename _Tp>
   struct min_value; 
  template <>
   struct min_value<int8_t> { static constexpr int8_t value = INT8_MIN; };
  template <>
   struct min_value<uint8_t> { static constexpr uint8_t value = 0; };
  template <>
   struct min_value<int16_t> { static constexpr int16_t value = INT16_MIN; };
  template <>
   struct min_value<uint16_t> { static constexpr uint16_t value = 0; };
  template <>
   struct min_value<int32_t> { static constexpr int32_t value = INT32_MIN; };
  template <>
   struct min_value<uint32_t> { static constexpr uint32_t value = 0; }; 

  template <typename _Tp>
   struct max_value;
   template <>
    struct max_value<int8_t> { static constexpr int8_t value = INT8_MAX; };
   template <>
    struct max_value<uint8_t> { static constexpr uint8_t value = UINT8_MAX; };
   template <>
    struct max_value<int16_t> { static constexpr int16_t value = INT16_MAX; };
   template <>
    struct max_value<uint16_t> { static constexpr uint16_t value = UINT16_MAX; };
   template <>
    struct max_value<int32_t> { static constexpr int32_t value = INT32_MAX; };
   template <>
    struct max_value<uint32_t> { static constexpr uint32_t value = UINT32_MAX; };
}

// Wrap up the assertion that a/b==fast_div(a,b)
// and provide pretty error messages
template <typename TDividend, typename TDivisor>
static void assert_fastdiv(TDividend dividend, TDivisor divisor, bool is_signed) {
  TDividend expected = 0;
  if (divisor!=0) { // Division by zero on Teensy generates an exception
    expected = (TDividend)(dividend/divisor);
  }
  TDividend actual = fast_div(dividend, divisor);
  
  char msgBuffer[256];
  if (is_signed) {
    sprintf(msgBuffer, "%s: %" PRId32 ", %" PRId32, __PRETTY_FUNCTION__, (int32_t)dividend, (int32_t)divisor);
    TEST_ASSERT_EQUAL_INT32_MESSAGE(expected, actual, msgBuffer);
  } else {
    sprintf(msgBuffer, "%s: %" PRIu32 ", %" PRIu32, __PRETTY_FUNCTION__, (uint32_t)dividend, (uint32_t)divisor);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, actual, msgBuffer);
  }
}

template <typename T, typename R = afd_type_traits::make_unsigned_t<T>>
static inline R absDelta(const T &min, const T &max) {
  if (min<0) {
    return (R)((R)max + (R)-min);
  }
  return (R)((R)max - (R)min);
}


template <typename TDividend, typename TDivisor>
static void test_fastdiv_range(TDividend divMin, TDividend divMax, TDivisor divisorMin, TDivisor divisorMax, bool is_signed) {

#if defined(DETAILED_MESSAGES)
  TEST_MESSAGE(__PRETTY_FUNCTION__);
#endif

  // To avoid overflow calculating the range for signed values.
  // Since abs(INT_MIN) overflows.
  if (is_signed) {
    if (divMin==afd_type_traits::min_value<TDividend>::value) {
      ++divMin;
    }
    if (divisorMin==afd_type_traits::min_value<TDivisor>::value) {
      ++divisorMin;
    }
  }
#if defined(DETAILED_MESSAGES)
  if (is_signed) {
    TEST_PRINTF("%ld, %ld, %ld, %ld", (int32_t)divMin, (int32_t)divMax, (int32_t)divisorMin, (int32_t)divisorMax);
  } else {
    TEST_PRINTF("%lu, %lu, %lu, %lu", (uint32_t)divMin, (uint32_t)divMax, (uint32_t)divisorMin, (uint32_t)divisorMax);
  }
#endif

  // Test the corners
#if defined(DETAILED_MESSAGES)
  TEST_MESSAGE("Testing range corners");
#endif
  assert_fastdiv(divMax,     divMax, is_signed);
  assert_fastdiv(divMax,     divMin, is_signed);
  assert_fastdiv(divMax,     divisorMin, is_signed);
  assert_fastdiv(divMax,     divisorMax, is_signed);
  assert_fastdiv(divMin,     divMax, is_signed);
  assert_fastdiv(divMin,     divMin, is_signed);
  assert_fastdiv(divMin,     divisorMin, is_signed);
  assert_fastdiv(divMin,     divisorMax, is_signed);
  // These are not valid
  // assert_fastdiv(divisorMax, divMax, is_signed);
  // assert_fastdiv(divisorMax, divMin, is_signed);
  // assert_fastdiv(divisorMax, divisorMin, is_signed);
  // assert_fastdiv(divisorMax, divisorMax, is_signed);
  // assert_fastdiv(divisorMin, divMax, is_signed);
  // assert_fastdiv(divisorMin, divMin, is_signed);
  // assert_fastdiv(divisorMin, divisorMin, is_signed);
  // assert_fastdiv(divisorMin, divisorMax, is_signed);

#if defined(EXTENDED_TEST_LEVEL) && (EXTENDED_TEST_LEVEL>0)
  using udividend_t = typename afd_type_traits::make_unsigned_t<TDividend>;
  udividend_t dividendRange = absDelta(divMin, divMax);
  TEST_ASSERT_GREATER_THAN_UINT32(0, dividendRange);

  using udivisor_t = typename afd_type_traits::make_unsigned_t<TDivisor>;
  udivisor_t divisorRange = absDelta(divisorMin, divisorMax);
  TEST_ASSERT_GREATER_THAN_UINT32(0, divisorRange);

  udividend_t dividendStep = (udividend_t)(dividendRange >  (udividend_t)EXTENDED_TEST_LEVEL ? (dividendRange/(udividend_t)EXTENDED_TEST_LEVEL) : 2);
  TEST_ASSERT_GREATER_THAN_UINT32(0, dividendStep);
  TEST_ASSERT_LESS_THAN_UINT32(divMax, dividendStep);
  TEST_ASSERT_LESS_THAN_UINT32(dividendRange, dividendStep);
  udivisor_t divisorStep = (udivisor_t)(divisorRange >  (udivisor_t)EXTENDED_TEST_LEVEL ? (divisorRange/(udivisor_t)EXTENDED_TEST_LEVEL) : 2);
  TEST_ASSERT_GREATER_THAN_UINT32(0, divisorStep);

  TEST_ASSERT_LESS_THAN_UINT32(divisorMax, divisorStep);
  TEST_ASSERT_LESS_THAN_UINT32(divisorRange, divisorStep);

#if defined(DETAILED_MESSAGES)
  TEST_PRINTF("Steps %lu, %lu", (uint32_t)dividendStep, (uint32_t)divisorStep);
#endif

  for (uint32_t outerIndex=0; outerIndex<EXTENDED_TEST_LEVEL; ++outerIndex) {
    TDividend dividend = (TDividend)(divMin + (dividendStep*outerIndex));
    for (uint32_t innerIndex=0; innerIndex<EXTENDED_TEST_LEVEL; ++innerIndex) {
      TDivisor divisor = (TDivisor)(divisorMin + (divisorStep*innerIndex));
      UNITY_OUTPUT_CHAR('.');
      assert_fastdiv(dividend, divisor, is_signed);
    }
    UNITY_OUTPUT_CHAR('\n');
  }
#endif
}

template <typename TDividend, typename TDivisor>
static void test_type_ranges(void) {
  test_fastdiv_range(afd_type_traits::min_value<TDividend>::value, afd_type_traits::max_value<TDividend>::value, 
                     afd_type_traits::min_value<TDivisor>::value, afd_type_traits::max_value<TDivisor>::value,
                     afd_type_traits::is_signed<TDividend>::value);
}

template <typename T>
static void test_type_ranges(void) {

  test_fastdiv_range(afd_type_traits::min_value<T>::value, afd_type_traits::max_value<T>::value, 
                    // Adjust the divisor range, else division will always be 1
                     (T)((afd_type_traits::min_value<T>::value/5)*4), (T)((afd_type_traits::max_value<T>::value/5)*4),
                     afd_type_traits::is_signed<T>::value);
}

static void test_fast_div_u8(void) {
  test_type_ranges<uint8_t>();
}
static void test_fast_div_s8(void) {
  test_type_ranges<int8_t>();
}

static void test_fast_div_8(void) {
  RUN_TEST(test_fast_div_u8);
  RUN_TEST(test_fast_div_s8);
}

static void test_fast_div_u16_u16(void) {
  test_type_ranges<uint16_t>();
}

static void test_fast_div_s16_s16(void) {
  test_type_ranges<int16_t>();
}

static void test_fast_div_u16_u8(void) {
  // Test fast_div()
  test_type_ranges<uint16_t, uint8_t>();

  // Test fast_div16_8() whih has a much more limited range of inputs
  TEST_ASSERT_EQUAL_UINT8(0, fast_div16_8(0, 0));
  TEST_ASSERT_EQUAL_UINT8(1, fast_div16_8(1, 1));
  TEST_ASSERT_EQUAL_UINT8(UINT8_MAX, fast_div16_8((uint16_t)UINT8_MAX*UINT8_MAX, UINT8_MAX));
}

static void test_fast_div_s16_s8(void) {
  test_type_ranges<int16_t, int8_t>();
}

static void test_fast_div_16(void) {
  RUN_TEST(test_fast_div_u16_u16);
  RUN_TEST(test_fast_div_s16_s16);
  RUN_TEST(test_fast_div_u16_u8);
  RUN_TEST(test_fast_div_s16_s8);
}

static void test_fast_div_32_32(void) {
  test_type_ranges<uint32_t>();
}
static void test_fast_div_32_16(void) {
  // Test fast_div()
  test_type_ranges<uint32_t, uint16_t>();

  // Test fast_div32_16() whih has a much more limited range of inputs
  TEST_ASSERT_EQUAL_UINT16(0, fast_div32_16(0, 0));
  TEST_ASSERT_EQUAL_UINT16(1, fast_div32_16(1, 1));
  TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, fast_div32_16((uint32_t)UINT16_MAX*UINT16_MAX, UINT16_MAX));
}
static void test_fast_div_32_8(void) {
  test_type_ranges<uint32_t, uint8_t>();
}
static void test_fast_div_s32_s32(void) {
  test_type_ranges<int32_t>();
}
static void test_fast_div_s32_s16(void) {
  test_type_ranges<int32_t, int16_t>();
}
static void test_fast_div_s32_s8(void) {
  test_type_ranges<int32_t, int8_t>();
}

static void test_fast_div_32(void) {
  RUN_TEST(test_fast_div_32_32);
  RUN_TEST(test_fast_div_32_16);
  RUN_TEST(test_fast_div_32_8);
  RUN_TEST(test_fast_div_s32_s32);
  RUN_TEST(test_fast_div_s32_s16);
  RUN_TEST(test_fast_div_s32_s8); 
}

void test_fast_div(void) {
    SET_UNITY_FILENAME() {
        test_fast_div_8();
        test_fast_div_16();
        test_fast_div_32();
    }
}