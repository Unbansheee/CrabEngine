module;

#pragma once

#include "ReflectionMacros.h"

export module Engine.Resource.Material;
import Engine.Resource.ResourceManager;
export import Engine.GFX.Vertex;
import Engine.WGPU;
import Engine.Object;
import Engine.Resource;
import Engine.Application;
import Engine.Assert;
import Engine.MaterialProperties;
import Engine.ShaderCompiler.Types;
import std;

// Storage for resources
struct BufferBinding {
    wgpu::raii::Buffer buffer;
    uint32_t size;
    bool isDynamic;
};

struct CPUBuffer {
    std::vector<uint8_t> data; // Raw bytes matching GPU buffer layout
    bool isDirty = false;      // Tracks if changes need uploading
    uint32_t size;             // Total buffer size (bytes)
    UniformMetadata uniformMetadata; // From shader reflection
};

struct TextureBinding {
    std::shared_ptr<TextureResource> texture;
};

struct SamplerBinding {
    wgpu::raii::Sampler sampler;
};

export struct MaterialSettings
{
    wgpu::PrimitiveTopology PrimitiveTopology = wgpu::PrimitiveTopology::TriangleList;
    wgpu::FrontFace FrontFace = wgpu::FrontFace::CW;
    wgpu::CullMode CullMode = wgpu::CullMode::Back;
    wgpu::CompareFunction DepthCompare = WGPUCompareFunction_Less;
    bool bUseBlending = true;
    bool bDepthWrite = true;
};

export class MaterialResource : public Resource
{
    CRAB_CLASS(MaterialResource, Resource)
    CLASS_FLAG(EditorVisible);
    BEGIN_PROPERTIES
        ADD_PROPERTY("ShaderModuleName", ShaderModuleName);
    END_PROPERTIES

    ~MaterialResource() override = default;
    MaterialResource(const std::string& moduleName, const MaterialSettings& settings = MaterialSettings());

    enum ENamedBindGroup
    {
        OBJECT = 0,
        GLOBAL = 1,
        RENDERER = 2,
        MATERIAL = 3,
        Count
    };

    std::string ShaderModuleName = "default_shader";

protected:
    struct MaterialBindGroup
    {
        uint8_t BindGroupIndex = 0;
        wgpu::BindGroup BindGroup;
        wgpu::BindGroupLayout Layout;
    };



    enum RenderPass
    {
        OPAQUE,
        UI
    };
    
public:
    void RecompileShader();

    wgpu::Device m_device = nullptr;
    wgpu::raii::RenderPipeline m_pipeline{};
    wgpu::raii::ShaderModule m_shaderModule;

    // TODO: Replace
    wgpu::TextureFormat TargetTextureFormat = WGPUTextureFormat::WGPUTextureFormat_BGRA8UnormSrgb;
    wgpu::TextureFormat DepthTextureFormat = wgpu::TextureFormat::Depth24Plus;

    MaterialSettings m_settings;

    std::unordered_map<std::string, CPUBuffer> m_cpuBuffers; // Mirrors m_buffers
    std::unordered_map<std::string, BufferBinding> m_buffers;

    std::unordered_map<std::string, TextureBinding> m_textures;
    std::unordered_map<std::string, SamplerBinding> m_samplers;

    std::unordered_map<std::string, UniformMetadata> m_uniformMetadata;
    std::unordered_map<uint32_t, wgpu::raii::BindGroup> m_bindGroups;
    std::unordered_map<uint32_t, bool> m_dirtyGroups;
    std::unordered_map<uint32_t, wgpu::raii::BindGroupLayout> m_bindGroupLayouts;
    void UpdateBindGroups();

    wgpu::raii::TextureView GetThumbnail() override;
    // NEW

    bool HasUniform(const std::string& parameter) {return m_uniformMetadata.contains(parameter);}
    std::vector<uint8_t> GetUniformData(const std::string& name);

    template<typename T>
    void SetUniform(const std::string& uniformName, T& data) {
        if constexpr(std::is_base_of_v<T, std::shared_ptr<TextureResource>>) {
            SetTexture(uniformName, data);
        }
        else if constexpr (std::is_base_of_v<T, wgpu::raii::Sampler>) {
            SetSampler(uniformName, data);
        }
        else {
            SetUniformData(uniformName, (void*)&data, sizeof(T));
        }
    };


    void SetUniformData(const std::string& uniformName, void* data, uint32_t size, uint32_t offset = 0);
    void SetTexture(const std::string& uniformName, const std::shared_ptr<TextureResource>& texture);
    void SetSampler(const std::string& uniformName, wgpu::raii::Sampler sampler);

    std::shared_ptr<TextureResource> GetTexture(const std::string& uniformName);

    std::vector<uint8_t> ReadBufferData(const wgpu::raii::Buffer& buffer, uint32_t size);

    wgpu::raii::PipelineLayout m_pipelineLayout;

    void CreateBuffer(const std::string& name, uint32_t size, bool isDynamic);

    void LoadData() override;
    MaterialResource() : Resource() {};
    
    MaterialResource(wgpu::Device device, const std::string& moduleName, MaterialSettings settings = MaterialSettings()) : m_device(device), m_settings(settings)
    {
        LoadFromShaderPath(device, moduleName, false, settings);
    }

    void LoadFromShaderPath(wgpu::Device device, const std::string& moduleName, bool bForceRecompile, MaterialSettings settings = MaterialSettings());

    virtual void Initialize()
    {
        if (!m_device) m_device = Application::Get().GetDevice();
        m_pipeline = CreateRenderPipeline();
    }

    virtual void OnPropertySet(Property& prop) override;

    void InitializeProperties();

    wgpu::raii::RenderPipeline GetPipeline() const { return m_pipeline; }
    virtual wgpu::raii::RenderPipeline CreateRenderPipeline();

    virtual void Apply(wgpu::RenderPassEncoder renderPass);

    void Serialize(nlohmann::json &archive) override;
    void Deserialize(nlohmann::json &archive) override;

    static inline std::shared_ptr<TextureResource> MaterialResourceThumbnail = nullptr;
};

