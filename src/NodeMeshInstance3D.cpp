//
// Created by Vinnie on 22/02/2025.
//

#include "NodeMeshInstance3D.h"

#include "RenderVisitor.h"


void NodeMeshInstance3D::SetMesh(const std::shared_ptr<Mesh> &newMesh) {
    this->mesh = newMesh;
}

void NodeMeshInstance3D::Render(RenderVisitor& Visitor)
{
    Visitor.Visit(*this);
    Node3D::Render(Visitor);
}



