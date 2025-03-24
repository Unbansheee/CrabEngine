
module;
#pragma once

#include "ReflectionMacros.h"

//#include "Node3D.cppm"

export module Engine.Node.Camera3D;
export import Engine.Node.Node3D;
import Engine.GFX.View;
import Engine.Types;

export class NodeCamera3D : public Node3D
{
public:
    CRAB_CLASS(NodeCamera3D, Node3D)
    CLASS_FLAG(EditorVisible)
    BEGIN_PROPERTIES
        ADD_PROPERTY("FOV", FOV)
        ADD_PROPERTY("Near Clipping Plane", NearClippingPlane)
        ADD_PROPERTY("Far Clipping Plane", FarClippingPlane)
    END_PROPERTIES

    void EnterTree() override;
    void ExitTree() override;

    virtual Matrix4 GetViewMatrix() const;
    
    float FOV = 45.f;
    float NearClippingPlane = 0.01f;
    float FarClippingPlane = 1000.f;
    
};
