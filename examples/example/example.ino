/*
 avr-fast-div example
*/
#include <Arduino.h>
#include <stdint.h>
#include "avr-fast-div.h"

void setup() {
  Serial.begin(9600);   //send and receive at 9600 baud  
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Beginning test...");

  const uint32_t divisor = random(2U, UINT16_MAX/2U);
  const uint32_t dividend = random((uint32_t)UINT16_MAX+1U, (uint32_t)UINT16_MAX*24U);

  constexpr uint32_t iterations = 5000;

  // Built in "/" operator
  uint32_t divCheckSum = 0UL;
  uint32_t divStartTime = micros();
  for (uint32_t i = 0U; i < iterations; i++) {
      // We need the +i & -i to prevent the optimiser making this loop a no-op
      divCheckSum += (dividend+i) / (divisor+i);
  }
  uint32_t divEndTime = micros();
  uint32_t divDuration = divEndTime-divStartTime;

  // fast_div()
  uint32_t fastdivCheckSum = 0UL;
  uint32_t fastdivStartTime = micros();
  for (uint32_t i = 0U; i < iterations; i++) {
      fastdivCheckSum += fast_div((dividend+i), (divisor+i));
  }
  uint32_t fastdivEndTime = micros();
  uint32_t fastdivDuration = fastdivEndTime-fastdivStartTime;

  char msg[128];
  sprintf(msg, "Dividend: %" PRIu32 ", Divisor: %" PRIu32, dividend, divisor);
  Serial.println(msg);
  sprintf(msg, "Div Checksum: %" PRIu32 ", FastDiv Checksum: %" PRIu32, divCheckSum, fastdivCheckSum);
  Serial.println(msg);
  sprintf(msg, "Div Duration: %" PRIu32 ", FastDiv Duration: %" PRIu32, divDuration, fastdivDuration);
  Serial.println(msg);
  uint16_t percentDelta = fastdivDuration * 100U / divDuration;
  sprintf(msg, "fast_div() took %" PRIu16 "%% less time than the division operator", 100-percentDelta);
  Serial.println(msg);
}

void loop() {

    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}

