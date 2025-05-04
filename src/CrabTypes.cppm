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

export struct InteropVector2U {
    unsigned int x, y = 0;

    InteropVector2U() = default;
    InteropVector2U(glm::uvec2 vec) {
        x = vec.x;
        y = vec.y;
    }
    operator glm::uvec2() {
        return {x, y};
    }
};

export struct InteropVector3 {
    float x, y, z = 0.f;

    InteropVector3() = default;
    InteropVector3(glm::vec3 vec) {
        x = vec.x;
        y = vec.y;
        z = vec.z;
    }
    operator glm::vec3() {
        return {x, y, z};
    }
};

export struct InteropVector2 {
    float x, y = 0.f;

    InteropVector2() = default;
    InteropVector2(glm::vec3 vec) {
        x = vec.x;
        y = vec.y;
    }
    operator glm::vec2() {
        return {x, y};
    }
};

export struct InteropQuat {
    float w = 1.f;
    float x, y, z = 0.f;

    InteropQuat() = default;
    InteropQuat(glm::quat vec) {
        w = vec.w;
        x = vec.x;
        y = vec.y;
        z = vec.z;
    }
    operator glm::quat() const {
        return glm::quat(w, x, y, z);
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

