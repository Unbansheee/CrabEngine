module;

#pragma once
//#include <webgpu/webgpu.h>
#include "ReflectionMacros.h"


export module Engine.Resource.ArrayMesh;
export import Engine.Resource.Mesh;
import Engine.GFX.MeshVertex;
import Engine.Application;
import Engine.Types;

export class ArrayMeshResource : public MeshResource
{
public:
    CRAB_CLASS(ArrayMeshResource, MeshResource)
    BEGIN_PROPERTIES
    END_PROPERTIES

    ArrayMeshResource() : MeshResource(){};
    
    void Serialize(nlohmann::json& archive) override;
    void Deserialize(nlohmann::json& archive) override;
    void LoadData() override;

    bool bIsRuntime = false;
    std::vector<MeshVertex> vertices{};
    std::vector<uint16_t> indices{};
    
    void LoadFromMeshData(const std::vector<MeshVertex>& _vertices, optional_ref<const std::vector<uint16_t>> _indices)
    {
        if (vertexBuffer){
            wgpuBufferDestroy(vertexBuffer);
            wgpuBufferRelease(vertexBuffer);
        }
        if (indexBuffer) {
            wgpuBufferDestroy(indexBuffer);
            wgpuBufferRelease(indexBuffer);
        }
        
        auto device = Application::Get().GetDevice();
        indexCount = static_cast<uint32_t>(_indices.has_value() ? _indices.value().get().size() : 0);
        vertexCount = static_cast<uint32_t>(_vertices.size());

        // Create vertex buffer
        WGPUBufferDescriptor vertexBufferDesc = {
            .nextInChain = nullptr,
            .label = "Vertex Buffer",
            .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
            .size = _vertices.size() * sizeof(MeshVertex),
            .mappedAtCreation = false
        };

        vertexBuffer = wgpuDeviceCreateBuffer(device, &vertexBufferDesc);
        wgpuQueueWriteBuffer(wgpuDeviceGetQueue(device), vertexBuffer, 0, _vertices.data(), vertexBufferDesc.size);

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
            wgpuQueueWriteBuffer(wgpuDeviceGetQueue(device), indexBuffer, 0, _indices.value().get().data(), indexBufferDesc.size);
        }
    }
};
