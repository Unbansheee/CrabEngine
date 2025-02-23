//
// Created by Vinnie on 22/02/2025.
//

#pragma once
#include <optional>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <webgpu/webgpu.hpp>

#include "IDrawable.h"

struct VertexData {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

class Mesh : public IDrawable {
public:
    WGPUBuffer vertexBuffer;
    WGPUBuffer indexBuffer;
    uint32_t indexCount;
    uint32_t vertexCount;

    Mesh(WGPUDevice device, const std::vector<VertexData>& vertices, optional_ref<const std::vector<uint16_t>> indices)
        : indexCount(static_cast<uint32_t>(indices.has_value() ? indices.value().get().size() : 0)),
        vertexCount(static_cast<uint32_t>(vertices.size()))
    {
        // Create vertex buffer
        WGPUBufferDescriptor vertexBufferDesc = {
            .nextInChain = nullptr,
            .label = "Vertex Buffer",
            .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
            .size = vertices.size() * sizeof(VertexData),
            .mappedAtCreation = false
        };
        vertexBuffer = wgpuDeviceCreateBuffer(device, &vertexBufferDesc);
        wgpuQueueWriteBuffer(wgpuDeviceGetQueue(device), vertexBuffer, 0, vertices.data(), vertexBufferDesc.size);

        if (indexCount > 0) {
            // Create index buffer
            uint64_t indexBufferSize = indexCount * sizeof(uint16_t);
            // ensure size is a multiple of 4
            indexBufferSize = (indexBufferSize + 3) & ~3;
            WGPUBufferDescriptor indexBufferDesc = {
                .nextInChain = nullptr,
                .label = "Index Buffer",
                .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index,
                .size = indexBufferSize,
                .mappedAtCreation = false
            };
            indexBuffer = wgpuDeviceCreateBuffer(device, &indexBufferDesc);
            wgpuQueueWriteBuffer(wgpuDeviceGetQueue(device), indexBuffer, 0, indices.value().get().data(), indexBufferDesc.size);
        }
    }

    virtual void GatherDrawCommands(std::vector<DrawCommand> &Commands) const override;
    
    ~Mesh() override
    {
        wgpuBufferDestroy(vertexBuffer);
        wgpuBufferRelease(vertexBuffer);
        if (indexCount > 0) {
            wgpuBufferDestroy(indexBuffer);
            wgpuBufferRelease(indexBuffer);
        }

    }
};


