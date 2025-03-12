#pragma once
#include <string>
#include <variant>

#include "CrabTypes.h"
#include "Transform.h"
#include "Resource/ResourceHandle.h"

// Supported native types + common engine types
using ValueVariant = std::variant<
    int,
    float,
    bool, 
    std::string,
    Vector3,
    Vector2,
    Quat,
    Vector4,
    Transform,
    ResourceRef,
    StrongResourceRef
>;