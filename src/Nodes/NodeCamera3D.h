#pragma once
#include "Renderer/Camera.h"
#include "Node3D.h"



class NodeCamera3D : public Node3D
{
public:
    CRAB_CLASS(NodeCamera3D, Node)
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
