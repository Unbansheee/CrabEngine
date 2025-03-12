//
// Created by Vinnie on 22/02/2025.
//

#pragma once
#include <optional>

#include "Resource.h"
#include "webgpu/webgpu.hpp"
#include "Core/ClassDB.h"

class MeshResource : public Resource {
public:
    CRAB_CLASS(MeshResource, Resource)
    
    BEGIN_PROPERTIES
    END_PROPERTIES
    
public:
    WGPUBuffer vertexBuffer = nullptr;
    WGPUBuffer indexBuffer = nullptr;
    uint32_t indexCount = 0;
    uint32_t vertexCount = 0;

    MeshResource() : Resource(){};
    
    ~MeshResource() override
    {
        if (vertexBuffer)
        {
            wgpuBufferDestroy(vertexBuffer);
            wgpuBufferRelease(vertexBuffer);
        }

        if (indexBuffer) {
            wgpuBufferDestroy(indexBuffer);
            wgpuBufferRelease(indexBuffer);
        }
    }
};


