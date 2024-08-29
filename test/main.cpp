#include <Arduino.h>
#include <unity.h>

extern void test_implementation_details(void);
extern void test_implementation_performance(void);
extern void test_fast_div(void);
extern void test_fast_div_performance(void);

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN(); 
    test_implementation_details();
    test_implementation_performance();
    test_fast_div();
    test_fast_div_performance();
    UNITY_END(); 
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}