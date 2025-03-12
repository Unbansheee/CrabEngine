//
// Created by Vinnie on 22/02/2025.
//

#include "NodeMeshInstance3D.h"

#include "Application.h"
#include "RenderVisitor.h"
#include "Gfx/Materials/StandardMaterial.h"


void NodeMeshInstance3D::SetMesh(const std::shared_ptr<MeshResource> &newMesh) {
    this->Mesh = newMesh;
}

void NodeMeshInstance3D::Render(RenderVisitor& Visitor)
{
    if (!material)
    {
        material = MakeShared<StandardMaterial>(Application::Get().GetDevice(), ENGINE_RESOURCE_DIR"/standard_material.wgsl");
        material->TargetTextureFormat = WGPUTextureFormat_BGRA8UnormSrgb;
        //material->NormalTextureView = norm;
        //material->BaseColorTextureView = albedo;
        material->Initialize();
    }

    if (Mesh)
    {
        auto inst = Mesh.Get<MeshResource>();
        if (!inst->IsLoaded())
        {
            inst->LoadData();
        }
        Visitor.Visit(*this);
    }
    
    Super::Render(Visitor);
}



