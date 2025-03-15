module;

#pragma once

#include <string>
#include <variant>

//#include "Transform.h"
//#include "Resource/ResourceHandle.h"

export module variant;
import resource_ref;
import crab_types;
import transform;

// Supported native types + common engine types
export using ValueVariant = std::variant<
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