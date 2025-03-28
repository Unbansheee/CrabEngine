//
// Created by Vinnie on 22/02/2025.
//
module;

#pragma once

#include "ReflectionMacros.h"

export module Engine.Node.MeshInstance3D;
export import Engine.Node.Node3D;
import Engine.Resource.Material;
import Engine.Resource.Texture;
import Engine.Resource.Ref;
import Engine.Resource.Mesh;

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

    void Render(Renderer& renderer) override;
    
protected:
    StrongResourceRef material;
    StrongResourceRef Mesh;
};
