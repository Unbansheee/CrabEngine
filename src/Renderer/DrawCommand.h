#pragma once
#include <webgpu/webgpu.hpp>

#include "CrabTypes.h"

class Material;

struct DrawCommand
{
    Material* material;
    wgpu::Buffer vertexBuffer = nullptr;
    uint32_t vertexCount = 0;
    wgpu::Buffer indexBuffer = nullptr;
    uint32_t indexCount = 0;
    Matrix4 modelMatrix = Matrix4(1.0f);
    uint32_t dynamicOffset = 0;
};
