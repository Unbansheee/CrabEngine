module;
#include "MaterialHelpers.h"

module Engine.Resource.Material.Standard;
import Engine.Resource.ImageTexture;
import Engine.Resource.ResourceManager;
import Engine.GFX.MeshVertex;
import Engine.WGPU;
import Engine.Resource.ShaderFile;

using namespace Uniforms;
using namespace MaterialHelpers;

StandardMaterial::StandardMaterial() : MaterialResource()
{
    shader_file = ResourceManager::Load<ShaderFileResource>(ENGINE_RESOURCE_DIR"/standard_material.wgsl");
}


void StandardMaterial::Initialize()
{
    MaterialParameters.Initialize(m_device);
    
    using namespace wgpu;
    SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = AddressMode::Repeat;
    samplerDesc.addressModeV = AddressMode::Repeat;
    samplerDesc.addressModeW = AddressMode::Repeat;
    samplerDesc.magFilter = FilterMode::Linear;
    samplerDesc.minFilter = FilterMode::Linear;
    samplerDesc.mipmapFilter = MipmapFilterMode::Nearest;
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = 8.0f;
    samplerDesc.compare = CompareFunction::Undefined;
    samplerDesc.maxAnisotropy = 1;
    TextureSampler = m_device.createSampler(samplerDesc);

    WGPUTextureViewDescriptor d;

    
    if (!BaseColorTextureView.Get<TextureResource>())
    {
        BaseColorTextureView = ResourceManager::Load<ImageTextureResource>(ENGINE_RESOURCE_DIR"/null_texture_black.png");
    }
    if (!NormalTextureView.Get<TextureResource>())
    {
        NormalTextureView = ResourceManager::Load<ImageTextureResource>(ENGINE_RESOURCE_DIR"/null_texture_black.png");
    }

    
    MaterialResource::Initialize();
}

std::vector<wgpu::BindGroupLayout> StandardMaterial::CreateMaterialBindGroupLayouts()
{
    return {
        GlobalUniformsLayout::Create(m_device), //0
        RendererUniformsLayout::Create(m_device), // 1
        PerObjectUniformsLayout::Create(m_device), // 2
        StandardMaterialUniformsLayout::Create(m_device) // 3 
    };   
}

std::vector<MaterialResource::MaterialBindGroup> StandardMaterial::CreateMaterialBindGroups()
{
    BindGroupCreator<StandardMaterialUniformsLayout> materialBindsCreator(m_device);
    
    auto matGroup = materialBindsCreator
        .Set<0, WGPUBuffer>(MaterialParameters.GetBuffer())
        .Set<1, WGPUSampler>(TextureSampler)
        .Set<2, WGPUTextureView>(BaseColorTextureView.Get<TextureResource>()->GetInternalTextureView())
        .Set<3, WGPUTextureView>(NormalTextureView.Get<TextureResource>()->GetInternalTextureView())
        .Build();
    
    return {{MATERIAL, matGroup}};
}

void StandardMaterial::CreateVertexBufferLayouts(std::vector<Vertex::VertexBufferLayout>& Layouts)
{
    auto& a = Layouts.emplace_back();
    Vertex::CreateVertexBufferLayout<MeshVertex>(a);
}

void StandardMaterial::UpdateUniforms()
{
}

void StandardMaterial::OnPropertySet(Property &prop) {
    MaterialResource::OnPropertySet(prop);

    if (prop.flags & PropertyFlags::MaterialProperty) {
        MarkBindGroupsDirty();
    }
}
