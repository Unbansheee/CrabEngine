module;
#pragma once

#include "ReflectionMacros.h"

export module Engine.Node.Node3D;
export import Engine.Transform;
export import Engine.Object;
import Engine.Types;

import Engine.Node;


// 3D version of a Node
// Contains a Transform
// Can be translated, rotated, scaled

export class Node3D : public Node
{
public:
    CRAB_CLASS(Node3D, Node)
    CLASS_FLAG(EditorVisible)
    
    BEGIN_PROPERTIES
        ADD_PROPERTY("Transform", transform)
    END_PROPERTIES

    BIND_METHOD_PARAMS(void, SetPosition, InteropVector3 position, (position))
    BIND_METHOD_PARAMS(void, SetScale, InteropVector3 scale, (scale))
    BIND_METHOD_PARAMS(void, SetOrientation, InteropQuat orientation, (orientation))
    BIND_METHOD_PARAMS(void, SetGlobalPosition, InteropVector3 position, (position))
    BIND_METHOD_PARAMS(void, SetGlobalScale, InteropVector3 scale, (scale))
    BIND_METHOD_PARAMS(void, SetGlobalOrientation, InteropQuat orientation, (orientation))

    BIND_METHOD(InteropVector3, GetPosition);
    BIND_METHOD(InteropVector3, GetScale);
    BIND_METHOD(InteropQuat, GetOrientation);
    BIND_METHOD(InteropVector3, GetGlobalPosition);
    BIND_METHOD(InteropVector3, GetGlobalScale);
    BIND_METHOD(InteropQuat, GetGlobalOrientation);
    
    virtual void SetPosition(const Vector3& Pos);
    virtual void SetScale(const Vector3& Scale);
    virtual void SetOrientation(const Quat& Orientation);

    Vector3 GetPosition() const;
    Vector3 GetScale() const;
    Quat GetOrientation() const;

    Vector3 GetGlobalPosition() const;
    Vector3 GetGlobalScale() const;
    Quat GetGlobalOrientation() const;

    void SetGlobalPosition(const Vector3& NewPosition);
    void SetGlobalScale(const Vector3& NewScale);
    void SetGlobalOrientation(const Quat& NewOrientation);

    Vector3 GetForwardVector() const;
    Vector3 GetUpVector() const;
    Vector3 GetRightVector() const;

    Transform GetTransform() const override;
    void UpdateTransform() override;

    Transform transform;

};