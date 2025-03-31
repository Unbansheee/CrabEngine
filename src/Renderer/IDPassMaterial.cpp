module;
#include "MaterialHelpers.h"

module Engine.GFX.IDPassMaterial;
import Engine.Resource.ImageTexture;
import Engine.Resource.ResourceManager;
import Engine.GFX.MeshVertex;
import Engine.WGPU;
import Engine.Resource.ShaderFile;
import Engine.GFX.UniformDefinitions;

using namespace Uniforms;
using namespace MaterialHelpers;

IDPassMaterial::IDPassMaterial() : MaterialResource()
{
    shader_file = ResourceManager::Load<ShaderFileResource>(ENGINE_RESOURCE_DIR"/id_pass.wgsl");
}

void IDPassMaterial::Initialize()
{
    TargetTextureFormat = wgpu::TextureFormat::R32Uint;
    m_settings.bUseBlending = false;
    MaterialResource::Initialize();
}

std::vector<wgpu::BindGroupLayout> IDPassMaterial::CreateMaterialBindGroupLayouts()
{
    return {
        GlobalUniformsLayout::Create(m_device), //0
        IDRendererUniformsLayout::Create(m_device), // 1
        PerObjectUniformsLayout::Create(m_device), // 2
    };   
}

std::vector<MaterialResource::MaterialBindGroup> IDPassMaterial::CreateMaterialBindGroups()
{
    return {};
}

void IDPassMaterial::CreateVertexBufferLayouts(std::vector<Vertex::VertexBufferLayout>& Layouts)
{
    auto& a = Layouts.emplace_back();
    Vertex::CreateVertexBufferLayout<MeshVertex>(a);
}

void IDPassMaterial::UpdateUniforms()
{
}
