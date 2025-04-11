module;

#pragma once

#include "ReflectionMacros.h"
//#include <webgpu/webgpu.hpp>

export module Engine.Resource.Material;
import Engine.Resource.ResourceManager;
export import Engine.GFX.Vertex;
import Engine.WGPU;
import Engine.Object;
import Engine.Resource;
import Engine.Resource.Ref;
import Engine.Application;
import Engine.Assert;
import Engine.MaterialProperties;
import Engine.ShaderCompiler.Types;


// Storage for resources
struct BufferBinding {
    WGPUBuffer buffer;
    uint32_t size;
    bool isDynamic;
};

struct TextureBinding {
    std::shared_ptr<TextureResource> texture;
    //WGPUSampler sampler;
};

export class MaterialResource : public Resource
{
    CRAB_ABSTRACT_CLASS(MaterialResource, Resource)
    BEGIN_PROPERTIES
        ADD_PROPERTY("ShaderFile", shader_file)
    END_PROPERTIES

    ~MaterialResource() override = default;

    enum ENamedBindGroup
    {
        OBJECT = 0,
        GLOBAL = 1,
        RENDERER = 2,
        MATERIAL = 3,
        Count
    };
    
protected:
    struct MaterialBindGroup
    {
        uint8_t BindGroupIndex = 0;
        wgpu::BindGroup BindGroup;
        wgpu::BindGroupLayout Layout;
    };

    struct MaterialSettings
    {
        wgpu::PrimitiveTopology PrimitiveTopology = wgpu::PrimitiveTopology::TriangleList;
        wgpu::FrontFace FrontFace = wgpu::FrontFace::CCW;
        wgpu::CullMode CullMode = wgpu::CullMode::None;
        bool bUseBlending = true;
    };

    enum RenderPass
    {
        OPAQUE,
        UI
    };
    
public:
    wgpu::Device m_device = nullptr;
    wgpu::RenderPipeline m_pipeline = nullptr;
    wgpu::raii::ShaderModule m_shaderModule ;

    // TODO: Replace
    wgpu::TextureFormat TargetTextureFormat = WGPUTextureFormat::WGPUTextureFormat_BGRA8UnormSrgb;
    wgpu::TextureFormat DepthTextureFormat = wgpu::TextureFormat::Depth24Plus;

    MaterialSettings m_settings;
    StrongResourceRef shader_file;

    std::unordered_map<std::string, BufferBinding> m_buffers;
    std::unordered_map<std::string, TextureBinding> m_textures;

    std::unordered_map<std::string, UniformMetadata> m_uniformMetadata;
    std::unordered_map<uint32_t, WGPUBindGroup> m_bindGroups;
    std::unordered_map<uint32_t, bool> m_dirtyGroups;
    std::unordered_map<uint32_t, wgpu::raii::BindGroupLayout> m_bindGroupLayouts;
    void UpdateBindGroups();

    // NEW
    template<typename T>
    void SetUniform(const std::string& uniformName, T& data) {
        Assert::Check(m_buffers.contains(uniformName), "Buffers.contains(uniformName)", "Parameter does not exist");
        auto buff = m_buffers.at(uniformName);
        Application::Get().GetQueue().writeBuffer(buff.buffer, 0, &data, sizeof(T));
    };
    void SetUniform(const std::string& uniformName, void* data, uint32_t size);
    void SetTexture(const std::string& uniformName, const std::shared_ptr<TextureResource>& texture);

    wgpu::raii::PipelineLayout m_pipelineLayout;

    void LoadData() override;
    MaterialResource() : Resource() {};
    
    MaterialResource(wgpu::Device device, const std::filesystem::path& shaderPath, MaterialSettings settings = MaterialSettings()) : m_device(device), m_settings(settings)
    {
        LoadFromShaderPath(device, shaderPath, settings);
    }

    void LoadFromShaderPath(wgpu::Device device, const std::filesystem::path& shaderPath, MaterialSettings settings = MaterialSettings());

    virtual void Initialize()
    {
        //assert(TargetTextureFormat != wgpu::TextureFormat::Undefined);
        if (!m_device) m_device = Application::Get().GetDevice();
        m_pipeline = CreateRenderPipeline();
    }

    virtual void OnPropertySet(Property& prop) override;

    void InitializeProperties();

    wgpu::RenderPipeline GetPipeline() const { return m_pipeline; }
    virtual wgpu::RenderPipeline CreateRenderPipeline();

    virtual void Apply(wgpu::RenderPassEncoder renderPass);



    /*
    template<typename T>
    bool ValidateType(MaterialPropertyType type) {
        switch (type) {
            case MaterialPropertyType::Float:
                Assert::Check(std::is_same_v<T, float>, "T == float", "Invalid type for T");
                break;
            case MaterialPropertyType::Int:
                Assert::Check(std::is_same_v<T, int>, "T == int", "Invalid type for T");
                break;
            case MaterialPropertyType::UInt:
                Assert::Check(std::is_same_v<T, uint32_t>, "T == uint32_t", "Invalid type for T");
                break;
            case MaterialPropertyType::Vector2:
                Assert::Check(std::is_same_v<T, glm::vec2>, "T == Vector2", "Invalid type for T");
                break;
            case MaterialPropertyType::Vector3:
                Assert::Check(std::is_same_v<T, glm::vec3>, "T == Vector3", "Invalid type for T");
                break;
            case MaterialPropertyType::Vector4:
                Assert::Check(std::is_same_v<T, glm::vec4>, "T == Vector4", "Invalid type for T");
                break;
            case MaterialPropertyType::Matrix4x4:
                Assert::Check(std::is_same_v<T, glm::mat4x4>, "T == Matrix4", "Invalid type for T");
                break;
            case MaterialPropertyType::Texture2D:
                Assert::Check(std::is_same_v<T, wgpu::TextureView>, "T == wgpu::TextureView", "Invalid type for T");
                break;
            case MaterialPropertyType::TextureCube:
                Assert::Check(std::is_same_v<T, wgpu::TextureView>, "T == wgpu::TextureView", "Invalid type for T");
                break;
        }

        return false;
    }
    */
};

