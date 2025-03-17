module;

#pragma once

#include <cassert>
#include <filesystem>
#include "ReflectionMacros.h"
//#include <webgpu/webgpu.hpp>

export module material;
import resource_manager;
export import vertex;
import wgpu;
import object;
import resource;
import resource_ref;


export class MaterialResource : public Resource
{
    CRAB_ABSTRACT_CLASS(MaterialResource, Resource)
    BEGIN_PROPERTIES
        ADD_PROPERTY("ShaderFile", shader_file)
    END_PROPERTIES

    ~MaterialResource() override = default;

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
    wgpu::TextureFormat TargetTextureFormat = WGPUTextureFormat::WGPUTextureFormat_BGRA8UnormSrgb;
    wgpu::TextureFormat DepthTextureFormat = wgpu::TextureFormat::Depth24Plus;
    MaterialSettings m_settings;
    bool bBindGroupsDirty = false;
    StrongResourceRef shader_file;

    void LoadData() override;
    MaterialResource() : Resource() {};
    
    MaterialResource(wgpu::Device device, const std::filesystem::path& shaderPath, MaterialSettings settings = MaterialSettings()) : m_device(device), m_settings(settings)
    {
        LoadFromShaderPath(device, shaderPath, settings);
    }

    void LoadFromShaderPath(wgpu::Device device, const std::filesystem::path& shaderPath, MaterialSettings settings = MaterialSettings());

    virtual void Initialize()
    {
        assert(TargetTextureFormat != wgpu::TextureFormat::Undefined);
        m_pipeline = CreateRenderPipeline();
        m_bindGroups = CreateMaterialBindGroups();
    }

    void MarkBindGroupsDirty() { bBindGroupsDirty = true;}

    virtual void OnPropertySet(Property& prop) override;
    
    wgpu::RenderPipeline GetPipeline() const { return m_pipeline; }
    virtual wgpu::RenderPipeline CreateRenderPipeline();
    virtual std::vector<wgpu::BindGroupLayout> CreateMaterialBindGroupLayouts() = 0;
    virtual std::vector<MaterialBindGroup> CreateMaterialBindGroups() = 0;
    virtual void CreateVertexBufferLayouts(std::vector<Vertex::VertexBufferLayout>& Layouts) = 0;
    virtual void UpdateUniforms() = 0;
    virtual void Apply(wgpu::RenderPassEncoder renderPass);
};
