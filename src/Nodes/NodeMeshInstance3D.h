//
// Created by Vinnie on 22/02/2025.
//

#pragma once
#include "Resource/Mesh.h"
#include "Node3D.h"
#include "Renderer/Material.h"


class NodeMeshInstance3D : public Node3D {
public:

    BEGIN_PROPERTIES(Node3D)
        ADD_PROPERTY(NodeMeshInstance3D, "Funny Number", FunnyNumber)
    END_PROPERTIES

    void SetMesh(const std::shared_ptr<Mesh>& newMesh);
    const std::shared_ptr<Mesh>& GetMesh() const {return mesh;}

    void SetMaterial(const std::shared_ptr<Material>& newMaterial) {material = newMaterial;}
    const std::shared_ptr<Material>& GetMaterial() const {return material;}

    virtual void Render(RenderVisitor& Visitor) override;
protected:
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    int FunnyNumber = 0;
};
