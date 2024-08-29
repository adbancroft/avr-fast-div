#pragma once

#include <stdint.h>

#if defined(__AVR__)
#include <Arduino.h>
#else
#include <sys/time.h>
#endif

#if !defined(MILLIS_PER_SEC)
#define MILLIS_PER_SEC 1000ULL
#endif
#if !defined(MICROS_PER_SEC)
#define MICROS_PER_SEC (MILLIS_PER_SEC*1000)
#endif
#if !defined(NANOS_PER_SEC)
#define NANOS_PER_SEC (MICROS_PER_SEC*1000)
#endif

class simple_timer_t {
private:
#if defined(__AVR__)
    uint32_t start_time;
    uint32_t end_time;
#else
    struct timeval start_time;
    struct timeval end_time;
#endif

public:

    void start() {
#if defined(__AVR__)
        start_time = micros();
#else 
        gettimeofday(&start_time, NULL);
#endif
    }

    void stop() {
#if defined(__AVR__)
        end_time = micros();
#else 
        gettimeofday(&end_time, NULL);
#endif        
    }

    uint32_t duration_micros() const {
#if defined(__AVR__)
        return end_time-start_time;
#else 
        return (uint32_t)(((end_time.tv_sec - start_time.tv_sec) * MICROS_PER_SEC) + (end_time.tv_usec - start_time.tv_usec));
#endif 
    }
};
