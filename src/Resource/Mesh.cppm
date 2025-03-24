//
// Created by Vinnie on 22/02/2025.
//
module;

#pragma once
#include <optional>

#include "ReflectionMacros.h"
//#include "webgpu/webgpu.hpp"

export module Engine.Resource.Mesh;
import Engine.Reflection.ClassDB;
export import Engine.Resource;
import Engine.WGPU;

export class MeshResource : public Resource {
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


