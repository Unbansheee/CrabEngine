//
// Created by Vinnie on 22/02/2025.
//

module node_mesh_instance_3d;
import application;
import render_visitor;
import standard_material;
import wgpu;
import crab_types;
import material;
import resource_manager;
import shader_file_resource;

NodeMeshInstance3D::NodeMeshInstance3D()
{
    material.SetTypeFilter(MaterialResource::GetStaticClass());
    Mesh.SetTypeFilter(MeshResource::GetStaticClass());
}

void NodeMeshInstance3D::SetMesh(const std::shared_ptr<MeshResource> &newMesh) {
    this->Mesh = newMesh;
}

void NodeMeshInstance3D::Render(RenderVisitor& Visitor)
{
    if (!material.Get<Resource>())
    {
        auto m = MakeShared<StandardMaterial>();
        m->shader_file = ResourceManager::Load<ShaderFileResource>(ENGINE_RESOURCE_DIR"/standard_material.wgsl");
        m->LoadData();
        material = m;

    }
    
    if (Mesh.Get<Resource>())
    {
        Visitor.Visit(*this);
    }
    
    Super::Render(Visitor);
    
}



