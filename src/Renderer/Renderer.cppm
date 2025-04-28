module;

#pragma once
//#include <webgpu/webgpu.hpp>
#include <vector>
#include <rocket/rocket.hpp>

export module Engine.GFX.Renderer;
import Engine.Node;
import Engine.GFX.UniformBuffer;
import Engine.GFX.DynamicOffsetUniformBuffer;
import Engine.GFX.UniformDefinitions;
import Engine.GFX.View;
import Engine.WGPU;
import Engine.Object.ObservableDtor;
export import Engine.GFX.DrawCommand;
import Engine.Resource.Texture;

export class MaterialResource;
export class MeshResource;

export class Renderer : public observable_dtor {
public:
    Renderer() = default;

    // Initialization
    void Initialize(wgpu::Device device);
    void Flush();
    std::vector<Node*> RenderNodeTree(Node* rootNode, View& view, wgpu::TextureView& colorAttachment, wgpu::TextureView& depthAttachment, const std::shared_ptr<TextureResource>& idTexture);
    void AddCommand(wgpu::raii::CommandBuffer command)
    {
        m_additionalPasses.push_back(command);
    }

    void DrawMesh(const std::shared_ptr<MeshResource>& mesh, const std::shared_ptr<MaterialResource>& material, const Matrix4& transform, Node* sender = nullptr, int priority = 0);
    
    std::vector<DrawBatch> BuildBatches(const std::vector<DrawCommand> commands, std::vector<Node*>& drawnNodes);
    void GatherDrawCommands(Node* rootNode);
    
    virtual void CreateBindGroups(wgpu::TextureView& idPassTex);
    virtual void UpdateUniforms();

    void SetClearColour(Vector4 colour) {m_clearColour = colour;}

    rocket::signal<void(Vector2)> OnResized;

    bool bDebugDrawEnabled = false;
private:
    glm::vec4 m_clearColour = {0,0,0,1};

    // Core WebGPU objects
    wgpu::Device m_device = nullptr;
    wgpu::Queue m_queue = nullptr;
    std::vector<wgpu::raii::CommandBuffer> m_additionalPasses;
    
    // Rendering state
    wgpu::BindGroup m_globalBindGroup = nullptr;
    wgpu::BindGroup m_objectUniformBindGroup = nullptr;
    wgpu::BindGroup m_rendererUniformBindGroup = nullptr;

    UniformBuffer<Uniforms::UGlobalData> m_globalUniformBuffer;
    UniformBuffer<Uniforms::ULightingData> m_lightingUniformBuffer;
    UniformBuffer<Uniforms::UObjectData> m_objectUniformBuffer;

    UniformBuffer<Uniforms::UCameraData> m_cameraUniformBuffer;

    std::shared_ptr<MaterialResource> m_fallbackMaterial;
    
    std::vector<DrawCommand> drawCommandBuffer;

    void ExecuteBatches(const std::vector<DrawBatch>& batches, wgpu::TextureView& colorAttachmentView, wgpu::TextureView& depthAttachmentView, const std::shared_ptr<TextureResource>& idTexture);
};
