#pragma once

#include "timer.hpp"

template <typename TLoop, typename TParam>
void measure_executiontime(uint16_t iterations, TLoop from, TLoop to, TLoop step, simple_timer_t &measure, TParam param, void (*pTestFun)(TLoop, TParam)) {
    measure.start();
    for (uint16_t loop=0; loop<iterations; ++loop)
    {
      for (TLoop a = from; a < to; a = (TLoop)(a + step))
      {
        pTestFun(a, param);
      }
    }
    measure.stop();
}

template <typename TParam>
struct execution_time {
    TParam result;
    simple_timer_t timer;
};

template <typename TParam>
struct comparative_execution_times {
    execution_time<TParam> timeA;
    execution_time<TParam> timeB;
};


template <typename TLoop, typename TParam>
comparative_execution_times<TParam> compare_executiontime(uint16_t iterations, TLoop from, TLoop to, TLoop step, void (*pTestFunA)(TLoop, TParam&), void (*pTestFunB)(TLoop, TParam&)) {

    simple_timer_t timerA;
    TParam paramA = 0;
    measure_executiontime<TLoop, TParam&>(iterations, from, to, step, timerA, paramA, pTestFunA);

    simple_timer_t timerB;
    TParam paramB = 0;
    measure_executiontime<TLoop, TParam&>(iterations, from, to, step, timerB, paramB, pTestFunB);

    return comparative_execution_times<TParam> {
        .timeA = execution_time<TParam> { .result = paramA, .timer = timerA },
        .timeB = execution_time<TParam> { .result = paramB, .timer = timerB }
    };
}

template <typename TLoop, typename TParam>
comparative_execution_times<TParam> compare_executiontime(TLoop from, TLoop to, TLoop step, void (*pTestFunA)(TLoop, TParam&), void (*pTestFunB)(TLoop, TParam&)) {

    simple_timer_t timerA;
    TParam paramA = 0;
    measure_executiontime<TLoop, TParam&>(1U, from, to, step, timerA, paramA, pTestFunA);

    simple_timer_t timerB;
    TParam paramB = 0;
    measure_executiontime<TLoop, TParam&>(1U, from, to, step, timerB, paramB, pTestFunB);

    return comparative_execution_times<TParam> {
        .timeA = execution_time<TParam> { .result = paramA, .timer = timerA },
        .timeB = execution_time<TParam> { .result = paramB, .timer = timerB }
    };
}