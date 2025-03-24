module;

#pragma once
#include <vector>
//#include "webgpu/webgpu.hpp"

export module Engine.Renderer.Visitor;
import Engine.GFX.DrawCommand;
import Engine.GFX.DynamicOffsetUniformBuffer;
import Engine.GFX.DynamicUniformBuffer;
import Engine.GFX.UniformDefinitions;
import Engine.GFX.View;
import Engine.Resource.Material;
import Engine.WGPU;

export class NodeMeshInstance3D;

export struct DrawItem {
    wgpu::Buffer vertexBuffer;  // Mesh vertex data
    wgpu::Buffer indexBuffer;   // Mesh index data
    uint32_t indexCount;        // Number of indices to draw
    uint32_t vertexCount;       // Number of vertices to draw
    uint32_t dynamicOffset;     // Offset into a dynamic uniform buffer (e.g., model matrix)
};

export struct DrawBatch {
    MaterialResource* material = nullptr;
    std::vector<DrawItem> drawItems = {};    // Per-object data (buffers, offsets)
};

export class RenderVisitor
{
public:
    RenderVisitor(wgpu::Device device, DynamicOffsetUniformBuffer<Uniforms::UObjectData>& dynamic_uniform_buffer) : dynamicUniforms(dynamic_uniform_buffer)
    {
    }
    
    void Visit(const NodeMeshInstance3D& node);

    std::vector<DrawBatch> BuildBatches() {
        // Sort commands by pipeline and bind groups
        /*
        std::sort(commands.begin(), commands.end(), [](const auto& a, const auto& b) {
            return std::tie(a.pipeline, a.bindGroups) < 
                   std::tie(b.pipeline, b.bindGroups);
        });
        */

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
            currentBatch->drawItems.push_back({
                cmd.vertexBuffer,
                cmd.indexBuffer,
                cmd.indexCount,
                cmd.vertexCount,
                cmd.dynamicOffset,
            });
        }
        return batches;
    }

private:
    std::vector<DrawCommand> commands;
    DynamicOffsetUniformBuffer<Uniforms::UObjectData>& dynamicUniforms;
};

