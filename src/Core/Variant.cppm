module;

#pragma once

#include <string>
#include <variant>

//#include "Transform.h"
//#include "Resource/ResourceHandle.h"

export module Engine.Variant;
import Engine.Resource.Ref;
import Engine.Types;
import Engine.Transform;
import Engine.Object.Ref;

//export class Object;

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
    StrongResourceRef,
    ObjectRef<Object>
>;