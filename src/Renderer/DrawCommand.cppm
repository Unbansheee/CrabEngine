#pragma once
//#include <webgpu/webgpu.hpp>

export module Engine.GFX.DrawCommand;
import Engine.Resource.Material;
import Engine.Types;
import Engine.WGPU;

export struct DrawCommand
{
    MaterialResource* material;
    wgpu::Buffer vertexBuffer = nullptr;
    uint32_t vertexCount = 0;
    wgpu::Buffer indexBuffer = nullptr;
    uint32_t indexCount = 0;
    Matrix4 modelMatrix = Matrix4(1.0f);
    uint32_t dynamicOffset = 0;
};
