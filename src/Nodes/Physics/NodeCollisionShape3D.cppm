//
// Created by Vinnie on 20/03/2025.
//
module;
#include "ReflectionMacros.h"

export module Engine.Node.CollisionShape3D;
import Engine.Node.Node3D;
import Engine.Physics.Jolt;
import Engine.Types;
import Engine.Resource.Material;
import Engine.Resource.ResourceManager;

export class NodeCollisionShape3D : public Node3D {
    CRAB_ABSTRACT_CLASS(NodeCollisionShape3D, Node3D)
    CLASS_FLAG(Abstract)
    BEGIN_PROPERTIES
        ADD_PROPERTY("Density", Density);
    END_PROPERTIES

    float Density = 1000.f;
public:
    JPH::ShapeRefC GetShapeTree();
    virtual JPH::ShapeRefC GetShape() const = 0;

    static std::tuple<std::shared_ptr<MaterialResource>, std::shared_ptr<MaterialResource>> GetDebugMaterial() {

        MaterialSettings s;
        s.DepthCompare = wgpu::CompareFunction::LessEqual;
        s.bDepthWrite = true;
        s.bUseBlending = true;
        s.PrimitiveTopology = wgpu::PrimitiveTopology::TriangleList;

        MaterialSettings swf;
        swf.DepthCompare = WGPUCompareFunction_LessEqual;
        swf.bDepthWrite = true;
        swf.bUseBlending = true;
        swf.PrimitiveTopology = wgpu::PrimitiveTopology::LineStrip;

        static std::shared_ptr<MaterialResource> s_debugMaterial = std::make_shared<MaterialResource>("collision_debug_shader", s);
        static std::shared_ptr<MaterialResource> s_debugMaterialWf = std::make_shared<MaterialResource>("collision_debug_shader", swf);

        auto col = glm::vec4(1, 0, 0, 0.1);
        s_debugMaterial->SetUniform("Colour", col);

        auto colWF = glm::vec4(1, 0, 0, 1);
        s_debugMaterialWf->SetUniform("Colour", colWF);

        return {s_debugMaterial, s_debugMaterialWf};
    }
};

export class NodeBoxShape3D : public NodeCollisionShape3D {
    CRAB_CLASS(NodeBoxShape3D, NodeCollisionShape3D)
    CLASS_FLAG(EditorVisible);

    BEGIN_PROPERTIES
        ADD_PROPERTY("Dimensions", Dimensions);
    END_PROPERTIES

    Vector3 Dimensions = {1.f, 1.f, 1.f};

    void Render(Renderer &renderer) override;

    JPH::ShapeRefC GetShape() const override;

};

export class NodeSphereShape3D : public NodeCollisionShape3D {
    CRAB_CLASS(NodeSphereShape3D, NodeCollisionShape3D)
    CLASS_FLAG(EditorVisible);

    BEGIN_PROPERTIES
        ADD_PROPERTY("Radius", Radius);
    END_PROPERTIES

    float Radius = 1.f;

    void Render(Renderer &renderer) override;

    JPH::ShapeRefC GetShape() const override;
};

export class NodeCapsuleShape3D : public NodeCollisionShape3D {
    CRAB_CLASS(NodeCapsuleShape3D, NodeCollisionShape3D)
    CLASS_FLAG(EditorVisible);

    BEGIN_PROPERTIES
        ADD_PROPERTY("Radius", Radius);
        ADD_PROPERTY("HalfHeight", HalfHeight);
    END_PROPERTIES

    float Radius = 1.f;
    float HalfHeight = 1.f;

    void Render(Renderer &renderer) override;

    JPH::ShapeRefC GetShape() const override;
};

