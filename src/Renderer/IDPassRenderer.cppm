module;

#pragma once
//#include <webgpu/webgpu.hpp>
#include <vector>
#include <rocket/rocket.hpp>

export module Engine.GFX.IDPassRenderer;
import Engine.Node;
import Engine.GFX.UniformBuffer;
import Engine.GFX.DynamicOffsetUniformBuffer;
import Engine.GFX.UniformDefinitions;
import Engine.GFX.View;
import Engine.WGPU;
import Engine.Object.ObservableDtor;
import Engine.GFX.IDPassMaterial;
export import Engine.GFX.DrawCommand;
import Engine.Object.Ref;

export class MaterialResource;
export class MeshResource;

export class IDPassRenderer : public observable_dtor {
public:
    IDPassRenderer() = default;

    // Initialization
    void Initialize(wgpu::Device device);
    void Flush();
    void RenderNodeTree(Node* rootNode, View& view, wgpu::TextureView& colorAttachment, wgpu::TextureView& depthAttachment);
    void AddCommand(wgpu::CommandBuffer command)
    {
        m_additionalPasses.push_back(command);
    }

    void DrawMesh(const std::shared_ptr<MeshResource>& mesh, const std::shared_ptr<MaterialResource>& material, const Matrix4& transform, Node* node);
    
    std::vector<DrawBatch> BuildBatches(const std::vector<DrawCommand> commands);
    void GatherDrawCommands(Node* rootNode);
    
    virtual void CreateBindGroups();
    virtual void UpdateUniforms();

    ObjectRef<Node> GetNode(uint32_t id);
    
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
    DynamicOffsetUniformBuffer<Uniforms::UIDPassObjectData> m_objectUniformBuffer;
    UniformBuffer<Uniforms::UCameraData> m_cameraUniformBuffer;
    
    //wgpu::BindGroup m_modelBindGroup = nullptr;
    std::vector<DrawCommand> drawCommandBuffer;

    std::shared_ptr<IDPassMaterial> IDPassMat;

    std::vector<ObjectRef<Node>> drawnNodes;
    
    void SortBatches(std::vector<DrawBatch>& batches);
    void ExecuteBatches(const std::vector<DrawBatch>& batches, wgpu::TextureView& colorAttachmentView, wgpu::TextureView& depthAttachmentView);
};
