//
// Created by Vinnie on 21/03/2025.
//
module;
#include "ReflectionMacros.h"

export module node_static_body_3d;
import node_collision_object_3d;
import jolt;
import physics.layers;

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
