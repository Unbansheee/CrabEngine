//
// Created by Vinnie on 22/02/2025.
//

#include "Transform.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>

Vector3 Transform::LocalToWorldPosition(Vector3 in) const
{
    return Vector3(GetWorldModelMatrix() * Vector4(in, 1.0f));
}

Vector3 Transform::LocalToWorldDirection(Vector3 in) const
{
    Matrix3 normalMatrix = transpose(inverse(Matrix3(GetWorldModelMatrix())));
    return glm::normalize(normalMatrix * in);
}

Vector3 Transform::LocalToWorldScale(Vector3 in) const
{
    auto m = GetWorldModelMatrix();

    // TODO: this use to be getcol(0). might need to be row based?
    Vector3 x = Vector3(m[0]);
    Vector3 y = Vector3(m[1]);
    Vector3 z = Vector3(m[2]);

    Vector3 worldScale;
    worldScale.x = (length(x) * Scale.x);
    worldScale.y = (length(y) * Scale.y);
    worldScale.z = (length(z) * Scale.z);

    return worldScale;
}

Vector3 Transform::WorldToLocalScale(Vector3 in) const
{
    auto m = GetWorldModelMatrix();

    // TODO: this use to be getcol(0). might need to be row based?
    Vector3 x = Vector3(m[0]);
    Vector3 y = Vector3(m[1]);
    Vector3 z = Vector3(m[2]);

    Vector3 localScale;
    localScale.x = (length(x) / Scale.x);
    localScale.y = (length(y) / Scale.y);
    localScale.z = (length(z) / Scale.z);

    return localScale;
}

Vector3 Transform::WorldToLocalPosition(Vector3 in) const
{
    return Vector3(inverse(GetWorldModelMatrix()) * Vector4(in, 1.f));
}

Vector3 Transform::WorldToLocalDirection(Vector3 in) const
{
    Matrix3 normalMatrix = transpose(inverse(Matrix3(GetWorldModelMatrix())));
    return glm::normalize(Vector3(inverse(normalMatrix) * in));
}

Quat Transform::LocalToWorldRotation(const Quat& in) const
{
    return normalize(Quat(Matrix3(GetWorldModelMatrix()))* in);
}

Quat Transform::WorldToLocalRotation(const Quat& in) const
{
    return normalize(Quat(glm::inverse(Matrix3(GetWorldModelMatrix()))) * in);
}