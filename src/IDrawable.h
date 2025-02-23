//
// Created by Vinnie on 22/02/2025.
//

#pragma once
#include <vector>
#include <webgpu/webgpu.hpp>
#include <CrabTypes.h>

class Context;

enum RenderSpace
{
    WORLD,
    SCREEN
};

// Data for executing a DrawCall. Drawable objects must be able to supply this data
struct DrawCommand
{
    DrawCommand() = default;

    // Draw type
    wgpu::PrimitiveTopology primitiveType = wgpu::PrimitiveTopology::TriangleList;

    // Renderspace
    RenderSpace renderSpace = WORLD;

    // Pointer to the first element of the vertexBuffer
    wgpu::Buffer vertexBuffer = nullptr;

    // Number of vertices in vertexbuffer
    uint32_t vertexCount = 0;

    // Pointer to the first element of the indexBuffer array
    wgpu::Buffer indexBuffer = nullptr;

    // Number of Indices in the IndexBuffer
    uint32_t indexCount = 0;

    //std::shared_ptr<Material> material;

    // Model matrix of the object
    Matrix4 modelMatrix = Matrix4(1.0f);
};


class IDrawable
{
public:
    // Create a list of DrawCommands to be submitted to the Render Context.
    // TODO: This should probably pass a vector<DrawCommand>
    // in as a mutable reference, rather than returning a new one each time this is called recursively
    virtual void GatherDrawCommands(std::vector<DrawCommand>& Commands) const {};
};