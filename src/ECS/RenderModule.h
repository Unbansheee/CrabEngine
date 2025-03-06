#pragma once
#include "CrabTypes.h"
#include "Renderer/Material.h"
#include "Mesh.h"
#include "Systems.h"
#include "WindowModule.h"
#include "webgpu/webgpu.hpp"



struct MeshComponent
{
    MeshComponent(const SharedRef<Mesh>& mesh) : Mesh(mesh) {}
    MeshComponent() = default;
    
    SharedRef<Mesh> Mesh;
};

struct MeshRenderComponent
{
    MeshRenderComponent(const SharedRef<Material>& mat) : Material(mat) {}
    MeshRenderComponent() = default;
    
    SharedRef<Material> Material;
};

struct CameraComponent
{
    
};

struct GlobalBindGroup
{
    UniformBuffer<Uniforms::UGlobalData> GlobalUniformBuffer; // Binding 0
    wgpu::BindGroup BindGroup = nullptr; // Set 0
};

struct RendererBindGroup
{
    UniformBuffer<Uniforms::UCameraData> CameraData; // Binding 0
    UniformBuffer<Uniforms::ULightingData> LightingData; // Binding 1
    wgpu::BindGroup BindGroup = nullptr; // Set 1
};

// Dynamic buffer for model matrices (shared across all objects)
struct ModelUniformBuffer {
    DynamicOffsetUniformBuffer<Uniforms::UObjectData> ObjectData; // Binding 0
    wgpu::BindGroup BindGroup = nullptr; // Set 2
};

struct DynamicOffset
{
    uint32_t Offset;
};

struct RenderModule
{
    RenderModule(flecs::world& w)
    {
        w.system<GPU, RenderTargetViewComponent, ModelUniformBuffer, RendererBindGroup, GlobalBindGroup>()
        .kind(flecs::OnUpdate)
        .term_at(0).singleton()
        .each([](flecs::entity e, GPU& gpu, RenderTargetViewComponent& rtv, ModelUniformBuffer& modelBuffer, RendererBindGroup& rbg, GlobalBindGroup& gbg)
        {
            wgpu::TextureView targetView = rtv.ColorView;
            wgpu::CommandEncoder encoder = gpu.Device.createCommandEncoder();

            // Configure the render pass (color + depth)
            wgpu::RenderPassDescriptor renderPassDesc = {};
            wgpu::RenderPassColorAttachment colorAttachment = {};
            colorAttachment.view = targetView;
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = {0.1f, 0.1f, 0.1f, 1.0f}; // Dark gray

            wgpu::RenderPassDepthStencilAttachment depthAttachment = {};
            depthAttachment.view = rtv.DepthView;
            depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
            depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
            depthAttachment.depthClearValue = 1.0f;

            renderPassDesc.colorAttachmentCount = 1;
            renderPassDesc.colorAttachments = &colorAttachment;
            renderPassDesc.depthStencilAttachment = &depthAttachment;

            // Start the render pass
            wgpu::RenderPassEncoder pass = encoder.beginRenderPass(renderPassDesc);

            // Group renderable entities by material to minimize pipeline switches
            std::unordered_map<SharedRef<Material>, std::vector<flecs::entity>> materialBatches;

            // Collect all entities with MeshComponent and MeshRenderComponent
            e.world().query<MeshComponent, MeshRenderComponent>()
                .each([&](flecs::entity ent, MeshComponent&, MeshRenderComponent& render) {
                    materialBatches[render.Material].push_back(ent);
                });

            for (auto& [materialEntity, entities] : materialBatches) {
            // Get the material's pipeline and bind group
            const SharedRef<Material> material = materialEntity;
            if (!material) continue; // Skip invalid materials

            material->Apply(pass);
            pass.setBindGroup(Material::ENamedBindGroup::RENDERER, rbg.BindGroup, 0, nullptr);
            pass.setBindGroup(Material::ENamedBindGroup::GLOBAL, gbg.BindGroup, 0, nullptr);

                
            // Draw all meshes using this material
            for (flecs::entity ent : entities) {
                const MeshComponent* mesh = ent.get<MeshComponent>();
                if (!mesh) continue;

                auto offset = ent.get<DynamicOffset>();
                pass.setBindGroup(Material::ENamedBindGroup::OBJECT, modelBuffer.BindGroup, 1, offset ? &offset->Offset : 0);
                
                // Bind vertex buffers
                pass.setVertexBuffer(0, mesh->Mesh->vertexBuffer, 0, WGPU_WHOLE_SIZE);

                // Draw indexed or non-indexed
                if (mesh->Mesh->indexBuffer) {
                    pass.setIndexBuffer(mesh->Mesh->indexBuffer, wgpu::IndexFormat::Uint16, 0, WGPU_WHOLE_SIZE);
                    pass.drawIndexed(mesh->Mesh->indexCount, 1, 0, 0, 0);
                } else {
                    pass.draw(mesh->Mesh->vertexCount, 1, 0, 0);
                }
            }
        }

            pass.end();
            wgpu::CommandBuffer commands = encoder.finish();
            gpu.Device.getQueue().submit(1, &commands);
        });

        
        w.system<GPU, Transform, ModelUniformBuffer>()
            .term_at(0).singleton()
            .each([](flecs::entity e, GPU& gpu, Transform& t, ModelUniformBuffer& modelBuffer)
            {
                glm::mat4 model = t.ModelMatrix;
                glm::mat4 local = t.GetLocalModelMatrix();
                Uniforms::UObjectData data;
                data.ModelMatrix = model;
                data.LocalMatrix = local;
                uint32_t offset = modelBuffer.ObjectData.Write(data);

                e.set<DynamicOffset>({offset});
            });
    }
};