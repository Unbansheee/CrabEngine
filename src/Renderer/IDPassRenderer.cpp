#include <webgpu/webgpu.h>

#include "MaterialHelpers.h"
#include "GLFW/glfw3.h"

module Engine.GFX.IDPassRenderer;
import Engine.Math;
import Engine.Application;
import Engine.Node;
import Engine.Resource.Material;
import Engine.GFX.DrawCommand;
import Engine.Resource.Mesh;
import Engine.Resource.Material.Standard;
import Engine.Resource.ShaderFile;
import Engine.Resource.ResourceManager;


void IDPassRenderer::Initialize(wgpu::Device device)
{
    m_device = device;
    m_queue = device.getQueue();
    m_globalUniformBuffer.Initialize(device);
    m_cameraUniformBuffer.Initialize(device);
    m_objectUniformBuffer.Initialize(device, 512);

    IDPassMat = MakeShared<IDPassMaterial>();
    IDPassMat->LoadData();
}


void IDPassRenderer::RenderNodeTree(Node* rootNode, View& view, wgpu::TextureView& colorAttachment,
                              wgpu::TextureView& depthAttachment)
{
    drawnNodes.clear();
    // index 0 is bad
    drawnNodes.emplace_back(ObjectRef<Node>());
    CreateBindGroups();
    GatherDrawCommands(rootNode);
    
    // 3. Build and sort batches
    auto batches = BuildBatches(drawCommandBuffer);
    SortBatches(batches);

    m_objectUniformBuffer.Upload(m_queue);

    Uniforms::UCameraData cameraData;
    cameraData.cameraPosition = view.Position;
    cameraData.projectionMatrix = view.ProjectionMatrix;
    cameraData.viewMatrix = view.ViewMatrix;
    m_cameraUniformBuffer.SetData(cameraData);
    
    // 4. Execute rendering
    ExecuteBatches(batches, colorAttachment, depthAttachment);
}

void IDPassRenderer::DrawMesh(const std::shared_ptr<MeshResource>& mesh, const std::shared_ptr<MaterialResource>& material,
    const Matrix4& transform, Node* node)
{
    drawCommandBuffer.push_back(DrawCommand{
        material.get(),
        mesh->vertexBuffer,
        mesh->vertexCount,
    mesh->indexBuffer,
        mesh->indexCount,
    transform});

    drawnNodes.emplace_back(node);
}

std::vector<DrawBatch> IDPassRenderer::BuildBatches(const std::vector<DrawCommand> commands)
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

        Uniforms::UIDPassObjectData data;
        data.ModelMatrix = cmd.modelMatrix;
        data.DrawID = m_objectUniformBuffer.GetCurrentCount() + 1;
        uint32_t currentOffset = m_objectUniformBuffer.Write(data);
        currentBatch->drawItems.push_back({
            cmd.vertexBuffer,
            cmd.indexBuffer,
            cmd.indexCount,
            cmd.vertexCount,
            currentOffset,
        });
    }
    return batches;
}

void IDPassRenderer::GatherDrawCommands(Node* rootNode)
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

void IDPassRenderer::CreateBindGroups()
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
        MaterialHelpers::BindGroupCreator<Uniforms::IDRendererUniformsLayout> BindCreator(device);
        m_rendererUniformBindGroup = BindCreator
            .Set<0, WGPUBuffer>(m_cameraUniformBuffer.GetBuffer())
            .Build();
    }
}

void IDPassRenderer::UpdateUniforms()
{
    Uniforms::UGlobalData globalData;
    globalData.Time = (float)glfwGetTime();
    m_globalUniformBuffer.SetData(globalData);
}

ObjectRef<Node> IDPassRenderer::GetNode(uint32_t id)
{
    if (drawnNodes.empty()) return ObjectRef<Node>();
    if (id >= drawnNodes.size()) return ObjectRef<Node>();

    return ObjectRef<Node>(drawnNodes.at(id));
}

void IDPassRenderer::Flush()
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


void IDPassRenderer::SortBatches(std::vector<DrawBatch>& batches)
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

void IDPassRenderer::ExecuteBatches(const std::vector<DrawBatch>& batches, wgpu::TextureView& colorAttachmentView, wgpu::TextureView& depthAttachmentView)
{
    // 1. Begin frame
    // Color attachments
    wgpu::RenderPassColorAttachment colorAttachment;
    colorAttachment.view = colorAttachmentView;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
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
        
    IDPassMat->Apply(pass);
        
    for (const auto& batch : batches) {
        // Bind global bind groups
        pass.setBindGroup(MaterialResource::ENamedBindGroup::GLOBAL, m_globalBindGroup, 0, nullptr);
        pass.setBindGroup(MaterialResource::ENamedBindGroup::RENDERER, m_rendererUniformBindGroup, 0, nullptr);
        
        // Execute draw calls
        for (const auto& item : batch.drawItems) {
            uint32_t dynamicOffset = item.dynamicOffset;

            pass.setBindGroup(MaterialResource::ENamedBindGroup::OBJECT, m_objectUniformBindGroup, 1, &dynamicOffset);
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

    drawCommandBuffer.clear();
}
