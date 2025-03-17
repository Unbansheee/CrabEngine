//
// Created by Vinnie on 22/02/2025.
//
module;

#pragma once

#include "ReflectionMacros.h"

export module node_mesh_instance_3d;
export import node_3d;
import material;
import texture_resource;
import resource_ref;
import mesh_resource;
import render_visitor;

export class NodeMeshInstance3D : public Node3D {
public:
    CRAB_CLASS(NodeMeshInstance3D, Node3D)
    CLASS_FLAG(EditorVisible)
    
    BEGIN_PROPERTIES
        ADD_PROPERTY("Mesh", Mesh)
        ADD_PROPERTY("Material", material);
    END_PROPERTIES

    NodeMeshInstance3D();
    
    void SetMesh(const std::shared_ptr<MeshResource>& newMesh);
    std::shared_ptr<MeshResource> GetMesh() const {return Mesh.Get<MeshResource>();}

    void SetMaterial(const std::shared_ptr<MaterialResource>& newMaterial) { material = newMaterial; }
    std::shared_ptr<MaterialResource> GetMaterial() const
    {
        return material.Get<MaterialResource>();
    }

    virtual void Render(RenderVisitor& Visitor) override;
    
protected:
    StrongResourceRef material;
    StrongResourceRef Mesh;
};
