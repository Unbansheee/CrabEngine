#pragma once
#include "Renderer/UniformDefinitions.h"
#include "Gfx/Vertex.h"
#include "Renderer/DynamicUniformBuffer.h"
#include "Renderer/Material.h"
#include "Renderer/UniformBuffer.h"
#include "Resource/TextureResource.h"


class StandardMaterial : public Material
{
public:
    using StandardMaterialUniformsLayout = MaterialHelpers::BindGroupLayoutBuilder<
        MaterialHelpers::UniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>, // UStandardMaterialParameters
        MaterialHelpers::SamplerEntry<1, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>, // TextureSampler
        MaterialHelpers::TextureEntry<2, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>, // BaseColorTexture
        MaterialHelpers::TextureEntry<3, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>>; // NormalTexture

    StandardMaterial(const wgpu::Device& device, const std::filesystem::path& shaderPath, const MaterialSettings& settings = MaterialSettings())
        : Material(device, shaderPath, settings),
          MaterialParameters(device)
    {
    }

    void Initialize() override;
    std::vector<wgpu::BindGroupLayout> CreateMaterialBindGroupLayouts() override;
    std::vector<MaterialBindGroup> CreateMaterialBindGroups() override;
    void CreateVertexBufferLayouts(std::vector<Vertex::VertexBufferLayout>& Layouts) override;
    void UpdateUniforms() override;

public:
    UniformBuffer<Uniforms::UStandardMaterialParameters> MaterialParameters;
    wgpu::Sampler TextureSampler = nullptr;
    SharedRef<TextureResource> BaseColorTextureView = nullptr;
    SharedRef<TextureResource> NormalTextureView = nullptr;
};
