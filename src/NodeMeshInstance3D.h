//
// Created by Vinnie on 22/02/2025.
//

#pragma once
#include "Mesh.h"
#include "Node3D.h"


class NodeMeshInstance3D : public Node3D {

public:
    void SetMesh(const std::shared_ptr<Mesh>& mesh);
    virtual void GatherDrawCommands(std::vector<DrawCommand> &Commands) const override;
protected:
    std::shared_ptr<Mesh> mesh;
};
