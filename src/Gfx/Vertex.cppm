module;
#pragma once

#include <vector>
#include <cstdint>

export module Engine.GFX.Vertex;
import Engine.GFX.MeshVertex;
import Engine.GLM;
import Engine.WGPU;

// I dont even know if this is used anymore, its from like 9 weeks ago :(

// Helper template to map C++ types to WebGPU formats
export template <typename T>
struct WebGPUFormat;
template <> struct WebGPUFormat<float>    { static constexpr WGPUVertexFormat format = WGPUVertexFormat_Float32; };
template <> struct WebGPUFormat<glm::vec2> { static constexpr WGPUVertexFormat format = WGPUVertexFormat_Float32x2; };
template <> struct WebGPUFormat<glm::vec3> { static constexpr WGPUVertexFormat format = WGPUVertexFormat_Float32x3; };
template <> struct WebGPUFormat<glm::vec4> { static constexpr WGPUVertexFormat format = WGPUVertexFormat_Float32x4; };

namespace Vertex
{
    export struct VertexAttribute;
}




// Attribute collector
export template <typename T>
struct VertexAttributeLayout {
    static std::vector<Vertex::VertexAttribute> GetAttributes() {
        static_assert(!std::is_same_v<T, T>, 
            "No vertex attribute specialization provided. Please specialize VertexAttributeLayout for your type.");
        return {};
    }
};





#define ATTRIBUTE(VertexType, member, location) \
     {offsetof(VertexType, member), WebGPUFormat<decltype(VertexType{}.member)>::format, location} 

#define DEFINE_VERTEX_LAYOUT(VertexType, ...) \
template <> \
    struct VertexAttributeLayout<VertexType> { \
    static std::vector<VertexAttribute> GetAttributes() { \
    return std::vector<VertexAttribute>{__VA_ARGS__}; \
    } \
};\



namespace Vertex
{
    // Struct to hold attribute data
    struct VertexAttribute {
        size_t offset;
        WGPUVertexFormat format;
        uint32_t shaderLocation;
    };

    export struct VertexBufferLayout
    {
        WGPUVertexBufferLayout Layout;
        std::vector<WGPUVertexAttribute> Attributes;
    };
    
    export template <typename VertexType>
    void CreateVertexBufferLayout(VertexBufferLayout& InLayout) {
            auto attributes = VertexAttributeLayout<VertexType>::GetAttributes();
            
            std::vector<WGPUVertexAttribute>& wgpuAttributes = InLayout.Attributes;
            uint32_t stride = 0;

            for (const auto& attr : attributes) {
                WGPUVertexAttribute wgpuAttr{};
                wgpuAttr.format = attr.format;
                wgpuAttr.offset = static_cast<uint32_t>(attr.offset);
                wgpuAttr.shaderLocation = attr.shaderLocation;

                wgpuAttributes.push_back(wgpuAttr);

                uint32_t stride_offset = 0;
                switch (attr.format)
                {
                case WGPUVertexFormat_Float32: stride_offset = sizeof(float); break;
                case WGPUVertexFormat_Float32x2: stride_offset = sizeof(float) * 2; break;
                case WGPUVertexFormat_Float32x3: stride_offset = sizeof(float) * 3; break;
                case WGPUVertexFormat_Float32x4: stride_offset = sizeof(float) * 4; break;
                default:
                    static_assert(true, "Vertex attribute type not implemented");
                    break;
                }
                stride += stride_offset; // Adjust based on actual type size
            }

            WGPUVertexBufferLayout& layout = InLayout.Layout;
            layout.arrayStride = stride;
            layout.stepMode = WGPUVertexStepMode_Vertex;
            layout.attributeCount = static_cast<uint32_t>(wgpuAttributes.size());
            layout.attributes = wgpuAttributes.data();
        }

}

export template <>
struct VertexAttributeLayout<MeshVertex>
{
    static std::vector<Vertex::VertexAttribute> GetAttributes()
    {
        return std::vector<Vertex::VertexAttribute>{
            {
                ((::size_t)&reinterpret_cast<char const volatile&>((((MeshVertex*)0)->position))),
                WebGPUFormat<decltype(MeshVertex{}.position)>::format, 0
            },
            {
                ((::size_t)&reinterpret_cast<char const volatile&>((((MeshVertex*)0)->normal))),
                WebGPUFormat<decltype(MeshVertex{}.normal)>::format, 1
            },
            {
                ((::size_t)&reinterpret_cast<char const volatile&>((((MeshVertex*)0)->color))),
                WebGPUFormat<decltype(MeshVertex{}.color)>::format, 2
            },
            {
                ((::size_t)&reinterpret_cast<char const volatile&>((((MeshVertex*)0)->uv))),
                WebGPUFormat<decltype(MeshVertex{}.uv)>::format, 3
            },
            {
                ((::size_t)&reinterpret_cast<char const volatile&>((((MeshVertex*)0)->tangent))),
                WebGPUFormat<decltype(MeshVertex{}.tangent)>::format, 4
            },
            {
                ((::size_t)&reinterpret_cast<char const volatile&>((((MeshVertex*)0)->bitangent))),
                WebGPUFormat<decltype(MeshVertex{}.bitangent)>::format, 5
            }
        };
    }
};