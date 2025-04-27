module;

#pragma once

export module Engine.Variant;
import Engine.Types;
import Engine.Transform;
import Engine.Object.Ref;
import Engine.UID;
import std;

export class Object;
export class Resource;

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
    UID,
    Transform,
    std::shared_ptr<Resource>,
    ObjectRef<Object>
>;