#include <Arduino.h>
#include <unity.h>
#include <avr/sleep.h>

extern void test_implementation_details(void);
extern void test_implementation_performance(void);
extern void test_fast_div(void);
extern void test_fast_div_performance(void);

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
#if !defined(SIMULATOR)
    delay(2000);
#endif

#if defined(CORE_TEENSY)
    // Without this, Teensy 3.5 produces a linker error:
    //  "undefined reference to `_write'""
    Serial.println("");
#endif

    UNITY_BEGIN(); 
    Serial.println("Testing Implementation");
    Serial.println("----------------------");
    test_implementation_performance();
    Serial.println("Testing Public API");
    Serial.println("------------------");
    test_fast_div_performance();
    UNITY_END(); 
    
    // Tell SimAVR we are done
    cli();
    sleep_enable();
    sleep_cpu();
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}

#include "avr-fast-div.cpp"