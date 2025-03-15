module;

#pragma once

#include <cassert>
#include <filesystem>
//#include <webgpu/webgpu.hpp>

export module material;
import resource_manager;
export import vertex;
import wgpu;


export class Material
{
public:
    virtual ~Material() = default;

    enum ENamedBindGroup
    {
        GLOBAL = 0,
        RENDERER = 1,
        OBJECT = 2,
        MATERIAL = 3,
        Count
    };
    
protected:
    struct MaterialBindGroup
    {
        uint8_t BindGroupIndex = 0;
        wgpu::BindGroup BindGroup;
    };

    struct MaterialSettings
    {
        wgpu::PrimitiveTopology PrimitiveTopology = wgpu::PrimitiveTopology::TriangleList;
        wgpu::FrontFace FrontFace = wgpu::FrontFace::CCW;
        wgpu::CullMode CullMode = wgpu::CullMode::None;
    };

    enum RenderPass
    {
        OPAQUE,
        UI
    };
    
public:
    wgpu::RenderPipeline m_pipeline = nullptr;
    wgpu::ShaderModule m_shaderModule = nullptr;
    wgpu::Device m_device = nullptr;
    wgpu::BindGroupLayout m_bindGroupLayouts = nullptr;
    std::vector<MaterialBindGroup> m_bindGroups = {};
    wgpu::TextureFormat TargetTextureFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat DepthTextureFormat = wgpu::TextureFormat::Depth24Plus;
    MaterialSettings m_settings;
    bool bBindGroupsDirty = false;

    
    Material(wgpu::Device device, const std::filesystem::path& shaderPath, MaterialSettings settings = MaterialSettings()) : m_device(device), m_settings(settings)
    {
        m_shaderModule = ResourceManager::loadShaderModule(shaderPath, device);
    }

    virtual void Initialize()
    {
        assert(TargetTextureFormat != wgpu::TextureFormat::Undefined);
        m_pipeline = CreateRenderPipeline();
        m_bindGroups = CreateMaterialBindGroups();
    }

    void MarkBindGroupsDirty() { bBindGroupsDirty = true;}

    
    wgpu::RenderPipeline GetPipeline() const { return m_pipeline; }
    virtual wgpu::RenderPipeline CreateRenderPipeline();
    virtual std::vector<wgpu::BindGroupLayout> CreateMaterialBindGroupLayouts() = 0;
    virtual std::vector<MaterialBindGroup> CreateMaterialBindGroups() = 0;
    virtual void CreateVertexBufferLayouts(std::vector<Vertex::VertexBufferLayout>& Layouts) = 0;
    virtual void UpdateUniforms() = 0;
    virtual void Apply(wgpu::RenderPassEncoder renderPass);
};
