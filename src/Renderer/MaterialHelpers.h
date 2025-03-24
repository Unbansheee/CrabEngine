#pragma once
import Engine.WGPU;

enum EShaderStageVisibility : uint8_t
{
    NONE = wgpu::ShaderStage::None,
    VERTEX = wgpu::ShaderStage::Vertex,
    FRAGMENT = wgpu::ShaderStage::Fragment,
    COMPUTE = wgpu::ShaderStage::Compute,
};

namespace MaterialHelpers
{
    #define BEGIN_LAYOUT(name) struct name##Layout {
    #define UNIFORM_BUFFER(binding, visibility) MaterialHelpers::UniformBufferEntry<binding, visibility> v##binding;
    #define TEXTURE(binding, visibility) TextureEntry<binding, visibility> v##binding;
    #define END_LAYOUT(...) using Type = MaterialHelpers::BindGroupLayoutBuilder<__VA_ARGS__>; };
    
    template <uint32_t Binding, WGPUShaderStageFlags Visibility>
    struct UniformBufferEntry {
        static wgpu::BindGroupLayoutEntry LayoutEntry() {
            wgpu::BufferBindingLayout buf = wgpu::Default;
            buf.type = wgpu::BufferBindingType::Uniform;
            buf.hasDynamicOffset = false;
            wgpu::BindGroupLayoutEntry entry = wgpu::Default;
            entry.visibility = Visibility;
            entry.binding = Binding;
            entry.buffer = buf;
            return entry;
        }
    };

    template <uint32_t Binding, WGPUShaderStageFlags Visibility>
    struct DynamicOffsetUniformBufferEntry {
        static wgpu::BindGroupLayoutEntry LayoutEntry() {
            wgpu::BufferBindingLayout buf = wgpu::Default;
            buf.type = wgpu::BufferBindingType::Uniform;
            buf.hasDynamicOffset = true;
            wgpu::BindGroupLayoutEntry entry = wgpu::Default;
            entry.visibility = Visibility;
            entry.binding = Binding;
            entry.buffer = buf;
            return entry;
        }
    };
    
    template <uint32_t Binding, uint8_t Visibility>
    struct TextureEntry {
        static wgpu::BindGroupLayoutEntry LayoutEntry() {
            wgpu::TextureBindingLayout tex;
            tex.sampleType = wgpu::TextureSampleType::Float;
            tex.viewDimension = wgpu::TextureViewDimension::_2D;
            wgpu::BindGroupLayoutEntry entry;
            entry.visibility = Visibility;
            entry.binding = Binding;
            entry.texture = tex;
            return entry;
        }
    };

    template <uint32_t Binding, uint8_t Visibility>
    struct SamplerEntry {
            static wgpu::BindGroupLayoutEntry LayoutEntry() {
                wgpu::SamplerBindingLayout samp = wgpu::Default;
                samp.type = wgpu::SamplerBindingType::Filtering;
                wgpu::BindGroupLayoutEntry entry;
                entry.visibility = Visibility;
                entry.binding = Binding;
                entry.sampler = samp;
                return entry;
            }
        };
    

    template <typename... Entries>
    class BindGroupLayoutBuilder {
    public:
        static constexpr size_t EntryCount = sizeof...(Entries);

        static wgpu::BindGroupLayout Create(wgpu::Device device) {
            const std::array<wgpu::BindGroupLayoutEntry, sizeof...(Entries)> entries = {
                Entries::LayoutEntry()...
            };
        
            wgpu::BindGroupLayoutDescriptor desc = wgpu::Default;
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            return device.createBindGroupLayout(desc);
        }
    };



    template <typename Layout>
    class BindGroupCreator {
    public:
        BindGroupCreator(wgpu::Device device) : m_layout(Layout::Create(device)), m_device(device) {}

        template <uint32_t Binding, typename T>
        BindGroupCreator& Set(const T& resource) {
            if constexpr (std::is_same_v<T, WGPUTextureView>) {
                wgpu::BindGroupEntry entry = wgpu::Default;
                entry.binding = Binding;
                entry.textureView = resource;
                m_entries[Binding] = entry;
            }
            else if constexpr (std::is_same_v<T, WGPUBuffer>)
            {
                wgpu::BindGroupEntry entry = wgpu::Default;
                entry.binding = Binding;
                entry.buffer = resource;
                entry.size = wgpuBufferGetSize(resource);
                m_entries[Binding] = entry;
            }
            else if constexpr (std::is_same_v<T, WGPUSampler>)
            {
                wgpu::BindGroupEntry entry = wgpu::Default;
                entry.binding = Binding;
                entry.sampler = resource;
                m_entries[Binding] = entry;
            }
            
            return *this;
        }

        
        template <uint32_t Binding>
        BindGroupCreator& SetDynamicBuffer(const WGPUBuffer& resource, uint32_t bindingSize)
        {
            wgpu::BindGroupEntry entry = wgpu::Default;
            entry.binding = Binding;
            entry.buffer = resource;
            entry.size = bindingSize;
            m_entries[Binding] = entry;

            return *this;
        }

        wgpu::BindGroup Build() {
            wgpu::BindGroupDescriptor desc;
            desc.layout = m_layout;
            desc.entryCount = m_entries.size();
            desc.entries = m_entries.data();
            
            return m_device.createBindGroup(desc);
        }

    private:
        wgpu::Device m_device;
        wgpu::BindGroupLayout m_layout;
        std::array<wgpu::BindGroupEntry, Layout::EntryCount> m_entries{};
    };
}