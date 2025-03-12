#pragma once

#include <json.hpp>
#include <glm/detail/type_quat.hpp>

#include "CrabTypes.h"
#include "Gfx/MeshVertex.h"

namespace glm
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2, x, y)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3, x, y, z)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector4, x, y, z, w);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Quat, w, x, y, z);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MeshVertex, position, normal, color, uv, tangent, bitangent);
