module;
#pragma once
#include <cstdint>

export module Engine.Math;
export import Engine.Types;

namespace MathUtils
{
    export constexpr inline uint32_t Align(uint32_t value, uint32_t step) {
        uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
        return step * divide_and_ceil;
    }

    export bool DecomposeTransform(const Matrix4& transform, Vector3& outTranslation, Quat& outRotation, Vector3& outScale);
}
