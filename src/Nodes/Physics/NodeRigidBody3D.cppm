//
// Created by Vinnie on 21/03/2025.
//

module;
#include "ReflectionMacros.h"

export module node_rigid_body_3d;
import node_collision_object_3d;
import jolt;
import physics.layers;

export class NodeRigidBody3D : public NodeCollisionObject3D
{
    CRAB_CLASS(NodeRigidBody3D, NodeCollisionObject3D)
    CLASS_FLAG(EditorVisible)

    virtual JPH::EMotionType GetMotionType() override;
    virtual JPH::ObjectLayer GetObjectLayer() override;
};

JPH::EMotionType NodeRigidBody3D::GetMotionType() {
    return JPH::EMotionType::Dynamic;
}

JPH::ObjectLayer NodeRigidBody3D::GetObjectLayer() {
    return Layers::MOVING;
}
