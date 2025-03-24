//
// Created by Vinnie on 21/03/2025.
//

module;
#include "ReflectionMacros.h"

export module Engine.Node.RigidBody3D;
import Engine.Node.CollisionObject3D;
import Engine.Physics.Jolt;
import Engine.Physics.Layers;

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
