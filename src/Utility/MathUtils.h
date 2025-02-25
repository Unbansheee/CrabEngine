#pragma once
#include <cstdint>

namespace MathUtils
{
    constexpr inline uint32_t Align(uint32_t value, uint32_t step) {
        uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
        return step * divide_and_ceil;
    }
}
