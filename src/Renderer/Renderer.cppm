module;

#pragma once
//#include <webgpu/webgpu.hpp>
#include <vector>
#include <rocket/rocket.hpp>

export module renderer;
import node;
import render_visitor;
import uniform_buffer;
import dynamic_offset_uniform_buffer;
import uniform_definitions;
import view;
import wgpu;
import observable_dtor;

export class Renderer : public observable_dtor {
public:
    Renderer() = default;

    // Initialization
    void Initialize(wgpu::Device device);
    
    void Flush();

    void RenderNodeTree(Node* rootNode, View& view, wgpu::TextureView& colorAttachment, wgpu::TextureView& depthAttachment);
    
    void AddCommand(wgpu::CommandBuffer command)
    {
        m_additionalPasses.push_back(command);
    }

    virtual void CreateBindGroups();
    virtual void UpdateUniforms();
    
    rocket::signal<void(Vector2)> OnResized;
private:
    // Core WebGPU objects
    wgpu::Device m_device = nullptr;
    wgpu::Queue m_queue = nullptr;
    std::vector<wgpu::CommandBuffer> m_additionalPasses;
    
    // Rendering state
    wgpu::BindGroup m_globalBindGroup = nullptr;
    wgpu::BindGroup m_objectUniformBindGroup = nullptr;
    wgpu::BindGroup m_rendererUniformBindGroup = nullptr;

    UniformBuffer<Uniforms::UGlobalData> m_globalUniformBuffer;
    UniformBuffer<Uniforms::ULightingData> m_lightingUniformBuffer;
    DynamicOffsetUniformBuffer<Uniforms::UObjectData> m_objectUniformBuffer;
    UniformBuffer<Uniforms::UCameraData> m_cameraUniformBuffer;
    
    //wgpu::BindGroup m_modelBindGroup = nullptr;
    

    void SortBatches(std::vector<DrawBatch>& batches);
    void ExecuteBatches(const std::vector<DrawBatch>& batches, wgpu::TextureView& colorAttachmentView, wgpu::TextureView& depthAttachmentView);
};
