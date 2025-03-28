//
// Created by Vinnie on 22/02/2025.
//

module Engine.Node.MeshInstance3D;
import Engine.Application;
import Engine.Resource.Material.Standard;
import Engine.WGPU;
import Engine.Types;
import Engine.Resource.Material;
import Engine.Resource.ResourceManager;
import Engine.Resource.ShaderFile;
import Engine.GFX.Renderer;

NodeMeshInstance3D::NodeMeshInstance3D()
{
    material.SetTypeFilter(MaterialResource::GetStaticClass());
    Mesh.SetTypeFilter(MeshResource::GetStaticClass());
}

void NodeMeshInstance3D::SetMesh(const std::shared_ptr<MeshResource> &newMesh) {
    this->Mesh = newMesh;
}

void NodeMeshInstance3D::Render(Renderer& renderer)
{
    Node3D::Render(renderer);

    std::shared_ptr<MeshResource> meshRef = Mesh.Get<MeshResource>();
    renderer.DrawMesh(meshRef, material.Get<MaterialResource>(), transform.GetWorldModelMatrix());
}


