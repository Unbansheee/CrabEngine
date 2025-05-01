module;
#pragma once
#include <optional>

export module Engine.Types;
export import Engine.GLM;
import std;

export typedef glm::vec2 Vector2;
export typedef glm::uvec2 Vector2U;
export typedef glm::vec3 Vector3;
export typedef glm::vec4 Vector4;
export typedef glm::mat4 Matrix4;
export typedef glm::mat3 Matrix3;
export typedef glm::mat2 Matrix2;
export typedef glm::quat Quat;
export template <typename T> using optional_ref = std::optional<std::reference_wrapper<T>>;

export struct RawVec2U {
    unsigned int x = 0;
    unsigned int y = 0;

    RawVec2U() = default;
    RawVec2U(glm::uvec2 vec) {
        x = vec.x;
        y = vec.y;
    }
};

export constexpr float PI = 3.14159265358979323846f;

export template<typename T>
using SharedRef = std::shared_ptr<T>;

export template<typename T, typename... Args>
SharedRef<T>
MakeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

