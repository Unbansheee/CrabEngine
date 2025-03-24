//
// Created by Vinnie on 21/03/2025.
//
module;
#include "ReflectionMacros.h"

export module Engine.Node.StaticBody3D;
import Engine.Node.CollisionObject3D;
import Engine.Physics.Jolt;
import Engine.Physics.Layers;

export class NodeStaticBody3D : public NodeCollisionObject3D
{
    CRAB_CLASS(NodeStaticBody3D, NodeCollisionObject3D)
    CLASS_FLAG(EditorVisible)

    virtual JPH::EMotionType GetMotionType() override;
    virtual JPH::ObjectLayer GetObjectLayer() override;
};

JPH::EMotionType NodeStaticBody3D::GetMotionType() {
    return JPH::EMotionType::Static;
}

JPH::ObjectLayer NodeStaticBody3D::GetObjectLayer() {
    return Layers::NON_MOVING;
}
