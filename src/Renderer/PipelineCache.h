#pragma once
#include "webgpu/webgpu.hpp"


class PipelineCache {
public:
    static PipelineCache& Get()
    {
        static PipelineCache s;
        return s;
    };
    
    wgpu::RenderPipeline GetPipeline(wgpu::Device device,
        const wgpu::RenderPipelineDescriptor& desc
    ) {
        size_t hash = HashPipelineDescriptor(desc);
        
        if (auto it = m_cache.find(hash); it != m_cache.end()) {
            return it->second;
        }
        
        wgpu::RenderPipeline pipeline = device.createRenderPipeline(desc);
        m_cache[hash] = pipeline;
        return pipeline;
    }

private:
    static inline std::unordered_map<size_t, WGPURenderPipeline> m_cache;
    
    size_t HashPipelineDescriptor(const wgpu::RenderPipelineDescriptor& desc) {
        // TODO: Implement a robust hash combining all pipeline state:
        // - Shader modules
        // - Vertex state
        // - Primitive state
        // - Depth/stencil state
        // - Multisample state
        // (Simplified example)
        return 0;
        return reinterpret_cast<size_t>(&desc.vertex.module) ^ 
               reinterpret_cast<size_t>(&desc.fragment->module);
    }

private:
    PipelineCache() = default;
    ~PipelineCache() {}
};
