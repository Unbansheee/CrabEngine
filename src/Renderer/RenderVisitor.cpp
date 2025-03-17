module render_visitor;
import node_mesh_instance_3d;
import uniform_definitions;
import dynamic_uniform_buffer;
import dynamic_offset_uniform_buffer;
import draw_command;

void RenderVisitor::Visit(const NodeMeshInstance3D& node)
{
    if (node.GetMaterial() == nullptr) return;
        
    // Get model matrix offset in the dynamic buffer
    Uniforms::UObjectData data;
    data.LocalMatrix = node.GetTransform().GetLocalModelMatrix();
    data.ModelMatrix = node.GetTransform().GetWorldModelMatrix();
    uint32_t offset = dynamicUniforms.Write(data);
        auto m = node.GetMaterial();
    // Create a DrawCommand with dynamic offset
    DrawCommand cmd = {
        .material = m.get(),
        .vertexBuffer = node.GetMesh()->vertexBuffer,
        .vertexCount = node.GetMesh()->vertexCount,
        .indexBuffer = node.GetMesh()->indexBuffer,
        .indexCount = node.GetMesh()->indexCount,
        .dynamicOffset = offset
    };
    commands.push_back(cmd);
}
