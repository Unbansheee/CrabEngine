
module obj_mesh_resource;
import resource_manager;
import application;
import mesh_vertex;
//#include <webgpu/webgpu.h>

void OBJMeshResource::Serialize(nlohmann::json& archive)
{
    MeshResource::Serialize(archive);
}

void OBJMeshResource::Deserialize(nlohmann::json& archive)
{
    MeshResource::Deserialize(archive);
}

void OBJMeshResource::LoadData()
{
    MeshResource::LoadData();
    loaded = true;
}

void OBJMeshResource::LoadOBJFromPath(const std::string& path)
{
    std::vector<MeshVertex> vertices;
    ResourceManager::loadGeometryFromObj(path, vertices);

    if (vertexBuffer){
        wgpuBufferDestroy(vertexBuffer);
        wgpuBufferRelease(vertexBuffer);
    }
    if (indexBuffer) {
        wgpuBufferDestroy(indexBuffer);
        wgpuBufferRelease(indexBuffer);
    }
        
    auto device = Application::Get().GetDevice();
    indexCount = 0;
    vertexCount = static_cast<uint32_t>(vertices.size());

    // Create vertex buffer
    WGPUBufferDescriptor vertexBufferDesc = {
        .nextInChain = nullptr,
        .label = "Vertex Buffer",
        .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
        .size = vertices.size() * sizeof(MeshVertex),
        .mappedAtCreation = false
    };

    vertexBuffer = wgpuDeviceCreateBuffer(device, &vertexBufferDesc);
    wgpuQueueWriteBuffer(wgpuDeviceGetQueue(device), vertexBuffer, 0, vertices.data(), vertexBufferDesc.size);
}
