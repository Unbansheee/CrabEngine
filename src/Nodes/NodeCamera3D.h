#pragma once
#include "Camera.h"
#include "Node3D.h"



class NodeCamera3D : public Node3D
{
public:

    BEGIN_PROPERTIES(Node3D)
        ADD_PROPERTY(NodeCamera3D, "FOV", FOV)
        ADD_PROPERTY(NodeCamera3D, "Near Clipping Plane", NearClippingPlane)
        ADD_PROPERTY(NodeCamera3D, "Far Clipping Plane", FarClippingPlane)
    END_PROPERTIES

    void EnterTree() override;
    void ExitTree() override;

    Matrix4 GetViewMatrix() const;
    
    float FOV = 45.f;
    float NearClippingPlane = 0.01f;
    float FarClippingPlane = 1000.f;
    
};
