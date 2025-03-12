#pragma once
#include <optional>
#include <glm/glm.hpp>
#include <memory>

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef glm::mat4 Matrix4;
typedef glm::mat3 Matrix3;
typedef glm::mat2 Matrix2;
typedef glm::quat Quat;
template <typename T> using optional_ref = std::optional<std::reference_wrapper<T>>;

constexpr float PI = 3.14159265358979323846f;

template<typename T>
using SharedRef = std::shared_ptr<T>;

template<typename T, typename... Args>
SharedRef<T> MakeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

