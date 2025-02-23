//
// Created by Vinnie on 22/02/2025.
//

#include "NodeMeshInstance3D.h"


void NodeMeshInstance3D::SetMesh(const std::shared_ptr<Mesh> &mesh) {
    this->mesh = mesh;
}

void NodeMeshInstance3D::GatherDrawCommands(std::vector<DrawCommand> &Commands) const {
    Node3D::GatherDrawCommands(Commands);

    if (mesh) {
        std::vector<DrawCommand> meshCommands;
        mesh->GatherDrawCommands(meshCommands);
        for (auto& command : meshCommands) {
            command.modelMatrix = transform.GetWorldModelMatrix();
            Commands.push_back(command);
        }
    }
}
