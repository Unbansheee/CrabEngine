//
// Created by Vinnie on 22/02/2025.
//

module node_mesh_instance_3d;
import application;
import render_visitor;
import standard_material;
import wgpu;
import crab_types;
//#include "webgpu/webgpu.hpp"

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

    if (Mesh.Get<Resource>())
    {
        Visitor.Visit(*this);
    }
    
    Super::Render(Visitor);
}



