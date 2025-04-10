
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
    Node* sender;
};

export struct DrawItem {
    wgpu::Buffer vertexBuffer;  // Mesh vertex data
    wgpu::Buffer indexBuffer;   // Mesh index data
    uint32_t indexCount;        // Number of indices to draw
    uint32_t vertexCount;       // Number of vertices to draw
    Matrix4 modelMatrix = Matrix4(1.0f);
    uint32_t drawID;     // Offset into a dynamic uniform buffer (e.g., model matrix)
};

export struct DrawBatch {
    MaterialResource* material = nullptr;
    std::vector<DrawItem> drawItems = {};    // Per-object data (buffers, offsets)
};