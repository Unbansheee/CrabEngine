//
// Created by Vinnie on 20/03/2025.
//
module;
#include "ReflectionMacros.h"

export module Engine.Node.CollisionShape3D;
import Engine.Node.Node3D;
import Engine.Physics.Jolt;
import Engine.Types;

export class NodeCollisionShape3D : public Node3D {
    CRAB_ABSTRACT_CLASS(NodeCollisionShape3D, Node3D)
    CLASS_FLAG(Abstract)

public:
    JPH::ShapeRefC GetShapeTree();
    virtual JPH::ShapeRefC GetShape() const = 0;
};

export class NodeBoxShape3D : public NodeCollisionShape3D {
    CRAB_CLASS(NodeBoxShape3D, NodeCollisionShape3D)
    CLASS_FLAG(EditorVisible);

    BEGIN_PROPERTIES
        ADD_PROPERTY("Dimensions", Dimensions);
    END_PROPERTIES

    Vector3 Dimensions = {1.f, 1.f, 1.f};

    JPH::ShapeRefC GetShape() const override;

};

export class NodeSphereShape3D : public NodeCollisionShape3D {
    CRAB_CLASS(NodeSphereShape3D, NodeCollisionShape3D)
    CLASS_FLAG(EditorVisible);

    BEGIN_PROPERTIES
        ADD_PROPERTY("Radius", Radius);
    END_PROPERTIES

    float Radius = 1.f;

    float GetScaledRadius() const;
    JPH::ShapeRefC GetShape() const override;
};
