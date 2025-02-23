#pragma once
#include <optional>
#include <glm/glm.hpp>

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef glm::mat4 Matrix4;
typedef glm::mat3 Matrix3;
typedef glm::mat2 Matrix2;
typedef glm::quat Quat;
template <typename T> using optional_ref = std::optional<std::reference_wrapper<T>>;

constexpr float PI = 3.14159265358979323846f;