module;

#pragma once
#include <filesystem>

#include "Renderer/MaterialHelpers.h"

export module standard_material;
import vertex;
import texture_resource;
import resource_ref;
import uniform_buffer;
import uniform_definitions;
export import material;
import dynamic_uniform_buffer;

export class StandardMaterial : public Material
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
    StrongResourceRef BaseColorTextureView;
    StrongResourceRef NormalTextureView;
};
