module;

#pragma once

#include <cassert>
#include <filesystem>
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
    wgpu::RenderPipeline m_pipeline = nullptr;
    wgpu::raii::ShaderModule m_shaderModule ;
    wgpu::Device m_device = nullptr;
    wgpu::BindGroupLayout m_bindGroupLayouts = nullptr;
    std::vector<MaterialBindGroup> m_bindGroups = {};
    wgpu::TextureFormat TargetTextureFormat = WGPUTextureFormat::WGPUTextureFormat_BGRA8UnormSrgb;
    wgpu::TextureFormat DepthTextureFormat = wgpu::TextureFormat::Depth24Plus;
    MaterialSettings m_settings;
    bool bBindGroupsDirty = false;
    StrongResourceRef shader_file;


    // NEW
    ShaderMetadata m_metadata;
    wgpu::BindGroup bindGroup = nullptr;
    wgpu::Buffer uniformBuffer = nullptr;
    wgpu::BindGroupLayout bindGroupLayout = nullptr;
    std::vector<wgpu::BindGroupEntry> m_bindGroupEntries;
    std::vector<uint8_t> m_uniformData;
    std::unordered_map<std::string, PropertyLayoutInfo> m_layoutInfo;
    std::vector<wgpu::TextureView> m_textureViews;

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
        assert(TargetTextureFormat != wgpu::TextureFormat::Undefined);
        if (!m_device) m_device = Application::Get().GetDevice();
        m_pipeline = CreateRenderPipeline();
        m_bindGroups = CreateMaterialBindGroups();
    }

    void MarkBindGroupsDirty() { bBindGroupsDirty = true;}

    virtual void OnPropertySet(Property& prop) override;

    void InitializeProperties();

    wgpu::RenderPipeline GetPipeline() const { return m_pipeline; }
    virtual wgpu::RenderPipeline CreateRenderPipeline();
    virtual std::vector<wgpu::BindGroupLayout> CreateMaterialBindGroupLayouts() = 0;
    virtual std::vector<MaterialBindGroup> CreateMaterialBindGroups() = 0;
    virtual void CreateVertexBufferLayouts(std::vector<Vertex::VertexBufferLayout>& Layouts) = 0;
    virtual void UpdateUniforms() = 0;
    virtual void Apply(wgpu::RenderPassEncoder renderPass);

    uint32_t GetTextureIndex(const std::string& name) const;
    uint32_t GetTextureBindingIndex(uint32_t textureIndex) const;

    template<typename T>
    void SetProperty(const std::string& name, const T& value) {
        auto device = Application::Get().GetDevice();
        auto queue = Application::Get().GetQueue();

        auto propIt = m_metadata.Properties.find(name);
        if (propIt == m_metadata.Properties.end()) {
            throw std::runtime_error("Property not found: " + name);
        }

        const MaterialProperty& prop = propIt->second;
        if (!ValidateType<T>(prop.Type)) {
            throw std::runtime_error("Type mismatch for property: " + name);
        }

        if (prop.Type == MaterialPropertyType::Texture2D) {
            // Handle texture updates
            const size_t textureIndex = GetTextureIndex(name);
            m_textureViews[textureIndex] = value; // Assuming value is TextureView

            // Recreate bind group with new texture
            std::vector<wgpu::BindGroupEntry>& entries = m_bindGroupEntries;
            entries[GetTextureBindingIndex(textureIndex)] = WGPUBindGroupEntry{
                .binding = GetTextureBindingIndex(textureIndex),
                .textureView = m_textureViews[textureIndex]
            };

            bindGroup = device.createBindGroup(WGPUBindGroupDescriptor{
                .layout = bindGroupLayout,
                .entries = entries.data(),
                .entryCount = entries.size()
            });
        } else {
            // Handle uniform updates
            const auto& layout = m_layoutInfo.at(name);
            memcpy(m_uniformData.data() + layout.Offset, &value, sizeof(T));

            // Partial buffer update
            queue.writeBuffer(
                uniformBuffer,
                layout.Offset,
                m_uniformData.data() + layout.Offset,
                layout.Size
            );
        }
    }

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
};

