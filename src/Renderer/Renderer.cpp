﻿module;

#include "MaterialHelpers.h"
#include "GLFW/glfw3.h"

module Engine.GFX.Renderer;
import Engine.Math;
import Engine.Application;
import Engine.Node;
import Engine.Resource.Material;
import Engine.GFX.DrawCommand;
import Engine.Resource.Mesh;
import Engine.Resource.ResourceManager;


void Renderer::Initialize(wgpu::Device device)
{
    m_device = device;
    m_queue = device.getQueue();
    m_globalUniformBuffer.Initialize(device);
    m_cameraUniformBuffer.Initialize(device);
    m_lightingUniformBuffer.Initialize(device);
    m_objectUniformBuffer.Initialize(device);

    m_fallbackMaterial = MakeShared<MaterialResource>();
    m_fallbackMaterial->LoadData();
}


std::vector<Node*> Renderer::RenderNodeTree(Node* rootNode, View& view, wgpu::TextureView& colorAttachment,
                              wgpu::TextureView& depthAttachment, const std::shared_ptr<TextureResource>& idTexture)
{
    std::vector<Node*> DrawnNodes;
    DrawnNodes.push_back(nullptr); // Index 0 is INVALID
    
    Uniforms::ULightingData lightingData;
    lightingData.DirectionalLights = {
        Uniforms::ULightingData::DirectionalLight{Vector4{0.5f, -0.9f, 0.1f, 0.0f}, Vector4{ 1.0f, 0.9f, 0.6f, 1.0f }},
        Uniforms::ULightingData::DirectionalLight{Vector4{0.2f, 0.4f, 0.3f, 0.0f}, {Vector4{ 0.6f, 0.9f, 1.0f, 1.0f }}}
    };
    lightingData.DirectionalLightCount = 2;
    m_lightingUniformBuffer.SetData(lightingData);
    
    GatherDrawCommands(rootNode);
    
    // 3. Build and sort batches
    auto batches = BuildBatches(drawCommandBuffer, DrawnNodes);

    Uniforms::UCameraData cameraData;
    cameraData.cameraPosition = view.Position;
    cameraData.projectionMatrix = view.ProjectionMatrix;
    cameraData.viewMatrix = view.ViewMatrix;
    m_cameraUniformBuffer.SetData(cameraData);
    
    // 4. Execute rendering
    ExecuteBatches(batches, colorAttachment, depthAttachment, idTexture);

    return DrawnNodes;
}

void Renderer::DrawMesh(const std::shared_ptr<MeshResource>& mesh, const std::shared_ptr<MaterialResource>& material,
    const Matrix4& transform, Node* sender, int priority)
{
    mesh->LoadIfRequired();
    material->LoadIfRequired();

    drawCommandBuffer.push_back(DrawCommand{
        material ? material.get() : m_fallbackMaterial.get(),
        mesh->vertexBuffer,
        mesh->vertexCount,
    mesh->indexBuffer,
        mesh->indexCount,
    transform,
        sender
        });

    
}

std::vector<DrawBatch> Renderer::BuildBatches(const std::vector<DrawCommand> commands, std::vector<Node*>& drawnNodes)
{
    // Group into batches
    std::vector<DrawBatch> batches;
    DrawBatch* currentBatch = nullptr;
    
    for (const auto& cmd : commands) {
        if (!currentBatch || 
            currentBatch->material != cmd.material)
            {
                batches.push_back({
                    .material = cmd.material,
                    .drawItems = {},
                });
                currentBatch = &batches.back();
            }

        drawnNodes.push_back(cmd.sender);
        uint32_t drawID = static_cast<uint32_t>(drawnNodes.size()) - 1;

        currentBatch->drawItems.push_back({
            cmd.vertexBuffer,
            cmd.indexBuffer,
            cmd.indexCount,
            cmd.vertexCount,
            cmd.modelMatrix,
            drawID,
        });
    }
    return batches;
}

void Renderer::GatherDrawCommands(Node* rootNode)
{
    std::queue<Node*> RenderQueue;
    RenderQueue.push(rootNode);
    while (!RenderQueue.empty())
    {
        Node* current = RenderQueue.front();
        RenderQueue.pop();
        if (current->IsHidden()) continue;
        
        current->Render(*this);
        for (auto& child : current->GetChildren())
        {
            RenderQueue.push(child);
        }
    }
}

void Renderer::CreateBindGroups(wgpu::TextureView& idPassTex)
{
    auto device = Application::Get().GetDevice();
    if (!m_globalBindGroup)
    {
        MaterialHelpers::BindGroupCreator<Uniforms::GlobalUniformsLayout> BindCreator(device);
        m_globalBindGroup = BindCreator
            .Set<0, WGPUBuffer>(*m_globalUniformBuffer.GetBuffer())
            .Build();
    }


    if (!m_objectUniformBindGroup)
    {
        MaterialHelpers::BindGroupCreator<Uniforms::PerObjectUniformsLayout> BindCreator(device);
        m_objectUniformBindGroup = BindCreator
            .Set<0, WGPUBuffer>(*m_objectUniformBuffer.GetBuffer())
            .Build();
    }


    if (m_rendererUniformBindGroup)
    {
        m_rendererUniformBindGroup.release();
        m_rendererUniformBindGroup = nullptr;
    }
    if (!m_rendererUniformBindGroup)
    {
        MaterialHelpers::BindGroupCreator<Uniforms::RendererUniformsLayout> BindCreator(device);
        m_rendererUniformBindGroup = BindCreator
            .Set<0, WGPUBuffer>(*m_cameraUniformBuffer.GetBuffer())
            .Set<1, WGPUBuffer>(*m_lightingUniformBuffer.GetBuffer())
            .Set<2, WGPUTextureView>(idPassTex)
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
    m_queue.submit(m_additionalPasses.size(), (wgpu::CommandBuffer*)m_additionalPasses.data());
    m_additionalPasses.clear();
}

void Renderer::ExecuteBatches(const std::vector<DrawBatch>& batches, wgpu::TextureView& colorAttachmentView, wgpu::TextureView& depthAttachmentView, const std::shared_ptr<TextureResource>& idTexture)
{
    // 1. Begin frame
    // Color attachments
    wgpu::RenderPassColorAttachment colorAttachment;
    colorAttachment.view = colorAttachmentView;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = WGPUColor(m_clearColour.r, m_clearColour.g, m_clearColour.b, m_clearColour.a);
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

    wgpu::raii::CommandEncoder encoder = m_device.createCommandEncoder();
    wgpu::raii::RenderPassEncoder pass = encoder->beginRenderPass(desc);
        
    // 2. Track current state
    MaterialResource* currentMaterial = nullptr;

    for (const auto& batch : batches) {
        // Pipeline state change

        // Switch pipeline and Bind material bind groups
        if (batch.material != currentMaterial) {
            currentMaterial = batch.material;

            currentMaterial->SetUniform("uCameraData", m_cameraUniformBuffer.GetData());
            currentMaterial->SetUniform("uLightingData", m_lightingUniformBuffer.GetData());

            if (idTexture) {
                currentMaterial->SetTexture("idPassTexture", idTexture);
            }
            currentMaterial->Apply(*pass);
        }


        // Execute draw calls
        for (const auto& item : batch.drawItems) {
            Uniforms::UObjectData d;
            d.DrawID = item.drawID;
            d.ModelMatrix = item.modelMatrix;
            pass->setPushConstants(wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment, 0, sizeof(d), &d);
            pass->setVertexBuffer(0, item.vertexBuffer, 0, wgpu::WHOLE_SIZE);


            if (item.indexCount > 0 && currentMaterial->m_settings.PrimitiveTopology == wgpu::PrimitiveTopology::TriangleList)
            {
                pass->setIndexBuffer(item.indexBuffer, wgpu::IndexFormat::Uint16, 0, wgpu::WHOLE_SIZE);
                pass->drawIndexed(
                    item.indexCount,
                    1,  // instanceCount
                    0,  // firstIndex
                    0,  // baseVertex
                    0   // firstInstance
                );
            }
            else
            {
                pass->draw(
                    item.vertexCount,
                    1, // instanceCount
                    0, // firstVertex
                    0); // firstInstance
            }
        }
    }

    // 3. End frame
    pass->end();
    wgpu::raii::CommandBuffer commands = encoder->finish();
    m_queue.submit(1, (wgpu::CommandBuffer*)&commands);

    drawCommandBuffer.clear();
}
