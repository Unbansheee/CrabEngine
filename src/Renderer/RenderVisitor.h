#pragma once
#include <algorithm>
#include <vector>

#include "DrawCommand.h"
#include "DynamicUniformBuffer.h"
#include "NodeMeshInstance3D.h"

class Node;

struct DrawItem {
    wgpu::Buffer vertexBuffer;  // Mesh vertex data
    wgpu::Buffer indexBuffer;   // Mesh index data
    uint32_t indexCount;        // Number of indices to draw
    uint32_t vertexCount;       // Number of vertices to draw
    uint32_t dynamicOffset;     // Offset into a dynamic uniform buffer (e.g., model matrix)
};

struct DrawBatch {
    Material* material = nullptr;
    std::vector<DrawItem> drawItems = {};    // Per-object data (buffers, offsets)
};

class RenderVisitor
{
public:
    RenderVisitor(wgpu::Device device, DynamicUniformBuffer& dynamic_uniform_buffer) : dynamicUniforms(dynamic_uniform_buffer)
    {
    }
    
    void Visit(const NodeMeshInstance3D& node) {
        if (node.GetMaterial() == nullptr) return;
        
        // Get model matrix offset in the dynamic buffer
        //uint32_t offset = dynamicUniforms.Allocate(1);
        //glm::mat4 t = node.GetTransform().GetWorldModelMatrix();
        //dynamicUniforms.Write(offset, &t, 1);
        
        // Create a DrawCommand with dynamic offset
        DrawCommand cmd = {
            .material = node.GetMaterial().get(),
            .vertexBuffer = node.GetMesh()->vertexBuffer,
            .vertexCount = node.GetMesh()->vertexCount,
            .indexBuffer = node.GetMesh()->indexBuffer,
            .indexCount = node.GetMesh()->indexCount,
            .dynamicOffset = 0
        };
        commands.push_back(cmd);
    }

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
    DynamicUniformBuffer& dynamicUniforms;
};
