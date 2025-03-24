//
// Created by Vinnie on 22/02/2025.
//

module Engine.Node.MeshInstance3D;
import Engine.Application;
import Engine.Renderer.Visitor;
import Engine.Resource.Material.Standard;
import Engine.WGPU;
import Engine.Types;
import Engine.Resource.Material;
import Engine.Resource.ResourceManager;
import Engine.Resource.ShaderFile;

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



