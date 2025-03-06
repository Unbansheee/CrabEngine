#include "Renderer.h"

#include "MathUtils.h"
#include "Gfx/Materials/StandardMaterial.h"
#include "GLFW/glfw3.h"

void Renderer::Initialize(wgpu::Device device)
{
    m_device = device;
    m_queue = device.getQueue();
    m_globalUniformBuffer.Initialize(device);
    m_cameraUniformBuffer.Initialize(device);
    m_lightingUniformBuffer.Initialize(device);
    m_objectUniformBuffer.Initialize(device, 512);
    
    // Create bind group for model buffer (created once during init)
    //m_modelBindGroup = CreateModelBindGroup(m_dynamicModelBuffer);
}


void Renderer::RenderNodeTree(Node* rootNode, const Camera& camera, wgpu::TextureView& colorAttachment,
                              wgpu::TextureView& depthAttachment)
{
    CreateBindGroups();
    Uniforms::UCameraData cameraData;
    cameraData.cameraPosition = camera.Position;
    cameraData.projectionMatrix = camera.ProjectionMatrix;
    cameraData.viewMatrix = camera.ViewMatrix;
    m_cameraUniformBuffer.SetData(cameraData);

    Uniforms::ULightingData lightingData;
    lightingData.LightColors = {
        Vector4{ 1.0f, 0.9f, 0.6f, 1.0f },
        Vector4{ 0.6f, 0.9f, 1.0f, 1.0f }
    };
    lightingData.LightDirections = {
        Vector4{0.5f, -0.9f, 0.1f, 0.0f},
        Vector4{0.2f, 0.4f, 0.3f, 0.0f}
    };
    m_lightingUniformBuffer.SetData(lightingData);
    
    if (rootNode)
    {
        RenderVisitor visitor(m_device, m_objectUniformBuffer);
        rootNode->Render(visitor);
        // 3. Build and sort batches
        auto batches = visitor.BuildBatches();
        SortBatches(batches);

        m_objectUniformBuffer.Upload(m_queue);
        
        // 4. Execute rendering
        ExecuteBatches(batches, colorAttachment, depthAttachment);
    }
}

void Renderer::CreateBindGroups()
{
    auto device = Application::Get().GetDevice();
    if (!m_globalBindGroup)
    {
        MaterialHelpers::BindGroupCreator<Uniforms::GlobalUniformsLayout> BindCreator(device);
        m_globalBindGroup = BindCreator
            .Set<0, WGPUBuffer>(m_globalUniformBuffer.GetBuffer())
            .Build();
    }

    if (!m_objectUniformBindGroup)
    {
        MaterialHelpers::BindGroupCreator<Uniforms::PerObjectUniformsLayout> BindCreator(device);
        m_objectUniformBindGroup = BindCreator
            .SetDynamicBuffer<0>(m_objectUniformBuffer.GetInternalBuffer(), m_objectUniformBuffer.GetBindingSize())
            .Build();
    }

    if (!m_rendererUniformBindGroup)
    {
        MaterialHelpers::BindGroupCreator<Uniforms::RendererUniformsLayout> BindCreator(device);
        m_rendererUniformBindGroup = BindCreator
            .Set<0, WGPUBuffer>(m_cameraUniformBuffer.GetBuffer())
            .Set<1, WGPUBuffer>(m_lightingUniformBuffer.GetBuffer())
            .Build();
    }
}

void Renderer::UpdateUniforms()
{
    Uniforms::UGlobalData globalData;
    globalData.Time = (float)glfwGetTime();
    m_globalUniformBuffer.SetData(globalData);
}

void Renderer::Flush()
{
    // 2. Collect draw commands
    m_queue.submit(m_additionalPasses.size(), m_additionalPasses.data());
    // Execute additional commands
    for (auto command : m_additionalPasses)
    {
        command.release();
    }
    m_additionalPasses.clear();
}


void Renderer::SortBatches(std::vector<DrawBatch>& batches)
{
    // Sort criteria:
    // 1. Render pass priority (e.g., shadows first)
    // 2. Pipeline state
    // 3. Material properties
    /*
        std::sort(batches.begin(), batches.end(), [](const DrawBatch& a, const DrawBatch& b) {
            return std::tie(a.pass, a.pipeline, a.materialHash) < 
                   std::tie(b.pass, b.pipeline, b.materialHash);
        });
        */
}

void Renderer::ExecuteBatches(const std::vector<DrawBatch>& batches, wgpu::TextureView& colorAttachmentView, wgpu::TextureView& depthAttachmentView)
{
    // 1. Begin frame
    // Color attachments
    wgpu::RenderPassColorAttachment colorAttachment;
    colorAttachment.view = colorAttachmentView;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.5f, 0.5f, 0.5f, 1.0f};
    colorAttachment.resolveTarget = nullptr;
    
    wgpu::RenderPassDepthStencilAttachment depthAttachment;
    depthAttachment.view = depthAttachmentView;
    depthAttachment.depthLoadOp = wgpu::LoadOp::Clear; 
    depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
    depthAttachment.stencilStoreOp = wgpu::StoreOp::Store;
    depthAttachment.depthClearValue = 1.0f;
        
    // Assemble render pass
    wgpu::RenderPassDescriptor desc;
    desc.colorAttachmentCount = 1;
    desc.colorAttachments = &colorAttachment;
    desc.depthStencilAttachment = &depthAttachment;
    
    wgpu::CommandEncoder encoder = m_device.createCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.beginRenderPass(desc);
        
    // 2. Track current state
    Material* currentMaterial = nullptr;
        
    for (const auto& batch : batches) {
        // Pipeline state change

        // Switch pipeline and Bind material bind groups
        if (batch.material != currentMaterial) {
            currentMaterial = batch.material;
            currentMaterial->Apply(pass);
        }

        // Bind global bind groups
        pass.setBindGroup(Material::ENamedBindGroup::GLOBAL, m_globalBindGroup, 0, nullptr);
        pass.setBindGroup(Material::ENamedBindGroup::RENDERER, m_rendererUniformBindGroup, 0, nullptr);
        
        // Execute draw calls
        for (const auto& item : batch.drawItems) {
            uint32_t dynamicOffset = item.dynamicOffset;

            pass.setBindGroup(Material::ENamedBindGroup::OBJECT, m_objectUniformBindGroup, 1, &dynamicOffset);
            pass.setVertexBuffer(0, item.vertexBuffer, 0, WGPU_WHOLE_SIZE);
            
            // Model bind group
            //pass.setBindGroup(0, m_modelBindGroup, 1, &dynamicOffset);
                
            if (item.indexCount > 0)
            {
                pass.setIndexBuffer(item.indexBuffer, wgpu::IndexFormat::Uint16, 0, WGPU_WHOLE_SIZE);
                pass.drawIndexed(
                    item.indexCount,
                    1,  // instanceCount
                    0,  // firstIndex
                    0,  // baseVertex
                    0   // firstInstance
                );
            }
            else
            {
                pass.draw(
                    item.vertexCount,
                    1, // instanceCount
                    0, // firstVertex
                    0); // firstInstance
            }
        }
    }
        
    // 3. End frame
    pass.end();
    wgpu::CommandBuffer commands = encoder.finish();
    m_queue.submit(1, &commands);

    pass.release();
    commands.release();
    encoder.release();
}
