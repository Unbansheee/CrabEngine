module;

#pragma once

#include "ReflectionMacros.h"
#include "Renderer/MaterialHelpers.h"

export module Engine.Resource.Material.Standard;
import Engine.GFX.Vertex;
import Engine.Resource.Texture;
import Engine.Resource.Ref;
import Engine.GFX.UniformBuffer;
import Engine.GFX.UniformDefinitions;
export import Engine.Resource.Material;
import Engine.GFX.DynamicUniformBuffer;
import Engine.Variant;

export class StandardMaterial : public MaterialResource
{
public:

    CRAB_CLASS(StandardMaterial, MaterialResource)
    BEGIN_PROPERTIES
        ADD_PROPERTY_FLAGS("Base Colour", BaseColorTextureView, PropertyFlags::MaterialProperty)
        ADD_PROPERTY_FLAGS("Normal", NormalTextureView, PropertyFlags::MaterialProperty)
        ADD_NESTED_STRUCT(MaterialParameters, UniformBuffer<Uniforms::UStandardMaterialParameters>)
    END_PROPERTIES
    
    using StandardMaterialUniformsLayout = MaterialHelpers::BindGroupLayoutBuilder<
        MaterialHelpers::UniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>, // UStandardMaterialParameters
        MaterialHelpers::SamplerEntry<1, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>, // TextureSampler
        MaterialHelpers::TextureEntry<2, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>, // BaseColorTexture
        MaterialHelpers::TextureEntry<3, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>>; // NormalTexture

    StandardMaterial();
    
    StandardMaterial(const wgpu::Device& device, const std::filesystem::path& shaderPath, const MaterialSettings& settings = MaterialSettings())
        : MaterialResource(device, shaderPath, settings)
          
    {
    }

    void Initialize() override;
    std::vector<wgpu::BindGroupLayout> CreateMaterialBindGroupLayouts() override;
    std::vector<MaterialBindGroup> CreateMaterialBindGroups() override;
    void CreateVertexBufferLayouts(std::vector<Vertex::VertexBufferLayout>& Layouts) override;
    void UpdateUniforms() override;
    void OnPropertySet(Property &prop) override;

public:
    UniformBuffer<Uniforms::UStandardMaterialParameters> MaterialParameters;
    wgpu::Sampler TextureSampler = nullptr;
    StrongResourceRef BaseColorTextureView;
    StrongResourceRef NormalTextureView;
};
