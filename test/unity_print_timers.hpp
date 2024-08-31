#pragma once
#include <unity.h>
#include "timer.hpp"

static inline void MESSAGE_TIMERS(const simple_timer_t &timerA, const simple_timer_t &timerB) {
    auto aTime = timerA.duration_micros();
    auto bTime = timerB.duration_micros();
    auto percent = bTime * 100U / aTime;

    char buffer[128];
    sprintf(buffer, "Timing: %" PRIu32 ", %" PRIu32 ", %"  PRIu32 "%%", aTime, bTime, percent);
    TEST_MESSAGE(buffer);
}
