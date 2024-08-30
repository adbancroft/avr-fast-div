#pragma once

#include "avr_fast_div.h"

template <typename T, typename TRange = type_traits::make_unsigned_t<T>>
static inline constexpr TRange safe_range(const T &min, const T &max) {
    return (min > max) ? (TRange)((TRange)min - (TRange)max) : (TRange)((TRange)max - (TRange)min);
}  

template <typename T>
class index_range_generator {
    using TStepSize = type_traits::make_unsigned_t<T>;

public:
    constexpr index_range_generator(const T &begin, const T &end, const uint16_t &num_step) 
        : _begin(begin)
        , _num_step(num_step <= safe_range(begin, end) ? num_step : 0)
        , _step_size((TStepSize)(safe_range(begin, end) / _num_step))
    {
    }

    constexpr T generate(const uint16_t &step) const {
        return (T)(_begin + (_step_size * step));
    }

    constexpr T num_steps(void) const {
        return _num_step;
    }
    
private:
    T _begin;
    uint16_t _num_step;  
    TStepSize _step_size;
};