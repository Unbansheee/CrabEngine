#pragma once
#include <webgpu/webgpu.hpp>
#include <vector>

#include "DynamicOffsetUniformBuffer.h"
#include "RenderVisitor.h"
#include "UniformBuffer.h"
#include "UniformDefinitions.h"

struct Camera
{
    Vector3 Position = {0,0,0};
    Matrix4 ViewMatrix = Matrix4(1.0f);
    Matrix4 ProjectionMatrix = Matrix4(1.0f);
};

class Renderer : public observable_dtor {
public:
    Renderer() = default;

    // Initialization
    void Initialize(wgpu::Device device);
    
    void Flush();

    void RenderNodeTree(Node* rootNode, const Camera& camera, wgpu::TextureView& colorAttachment, wgpu::TextureView& depthAttachment);

    
    void ECSRenderBegin(Camera& cam);
    void ECSRender(DrawCommand command);
    void ECSRenderEnd();
    
    void AddCommand(wgpu::CommandBuffer command)
    {
        m_additionalPasses.push_back(command);
    }

    virtual void CreateBindGroups();
    virtual void UpdateUniforms();

    std::unique_ptr<RenderVisitor> rv;
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
