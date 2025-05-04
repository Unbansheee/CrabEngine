//
// Created by Vinnie on 21/02/2025.
//
module;
#include "stb_image.h"
#include <tiny_obj_loader.h>
#include "fmt/format.h"

module Engine.Resource.ResourceManager;
import Engine.Resource.Mesh;
import Engine.Node.Node3D;
import Engine.GFX.MeshVertex;
import Engine.Resource.Importer.ImportSettings;
import Engine.Types;
import Engine.WGPU;
import Engine.Resource.ImportManager;
import Engine.Reflection.ClassDB;
import Engine.Assert;
import Engine.Application;
import Engine.Filesystem;
import fmt;

bool ResourceManager::IsSourceFile(const std::filesystem::path& path)
{
    return path.has_extension() && ImportManager::Get().IsFileTypeImportable(path.extension());
}

std::shared_ptr<Resource> ResourceManager::Load(const std::filesystem::path& path)
{
    std::string absolutePath = Filesystem::AbsolutePath(path.string());
    {
        std::lock_guard lock(cacheMutex);
        // Existing resource loading logic
        auto it = GetResourceCache().FetchResourceByPath(path.string());
        if (it) {
            return it;
        }
    }

    if (!std::filesystem::exists(absolutePath)) return nullptr;

    if (IsSourceFile(absolutePath)) {
        auto res = ImportManager::Get().Import(path);
        if (res)
        {

            std::lock_guard lock(cacheMutex);
            GetResourceCache().AddResource(res);
        }
        return res;
    }

    //Assert::Verify(false, "Resource Error", "Failed to load resource at: " + path.string());
    return nullptr;
}

std::shared_ptr<Resource> ResourceManager::FindByID(const UID &id) {
    auto fs = Application::Get().GetFilesystem();
    {
        std::lock_guard lock(cacheMutex);
        auto it = GetResourceCache().FetchResourceByID(id);
        if (it) {
            return it;
        }
    }

    return nullptr;
}

void ResourceManager::PollResourcesForChanges() {
    for (auto res : GetAllResources()) {
        auto absPath = res->GetAbsolutePath();
        auto resTime = std::filesystem::last_write_time(absPath);
        if (resTime != res->resource_time) {
            ResourceManager::ReloadResource(res);
        }
    }
}

void ResourceManager::ReloadResource(const std::shared_ptr<Resource> &resource) {
    auto absPath = Filesystem::AbsolutePath(resource->sourcePath);
    if (IsSourceFile(absPath)) {
        ImportManager::Get().Reimport(resource);
        if (resource)
        {
            std::lock_guard lock(cacheMutex);
            GetResourceCache().AddResource(resource);
        }
    }
}

bool ResourceManager::IsResourceLoaded(const std::filesystem::path &path) {
    std::lock_guard lock(cacheMutex);
    return GetResourceCache().FetchResourceByPath(path.string()) != nullptr;
}

void ResourceManager::SaveToFile(const std::filesystem::path& path, nlohmann::json& json)
{
    auto absPath = Filesystem::AbsolutePath(path.string());

    std::ofstream outFile(absPath);
    outFile << std::setw(4) << json << std::endl;
    outFile.close();
}

void ResourceManager::SaveImportSettings(const std::filesystem::path& sourcePath,
    const std::shared_ptr<ResourceMetadata>& importSettings)
{
    auto absPath = Filesystem::AbsolutePath(sourcePath.string());

    std::ofstream outFile(absPath += ".meta");
    nlohmann::json j;
    importSettings->Serialize(j);
    outFile << std::setw(4) << j << std::endl;
    outFile.close();
}

void ResourceManager::SaveResource(const std::shared_ptr<Resource> &resource, const std::filesystem::path &path) {
    auto savePath = path.empty() ? resource->GetSourcePath() : path;

    if (savePath.extension() == ".res") {
        nlohmann::json j;
        resource->Serialize(j);
        SaveToFile(savePath, j);
    }
    if (auto importSettings = resource->GetImportSettings()) {
        importSettings->ResourceID = resource->GetID();
        SaveImportSettings(savePath, importSettings);
    }

    resource->OnResourceSaved.invoke();
}

std::vector<std::shared_ptr<Resource>> ResourceManager::GetAllResources()
{
    return GetResourceCache().GetAllResources();
}

ResourceCache & ResourceManager::GetResourceCache() {
    static ResourceCache cache;
    return cache;
}

MeshVertex BuildVertex(const tinyobj::index_t& idx, const tinyobj::attrib_t& attrib) {
    MeshVertex vertex;

    vertex.position = {
        attrib.vertices[3 * idx.vertex_index + 0],
        -attrib.vertices[3 * idx.vertex_index + 2],
        attrib.vertices[3 * idx.vertex_index + 1]
    };

    vertex.normal = {
        attrib.normals[3 * idx.normal_index + 0],
        -attrib.normals[3 * idx.normal_index + 2],
        attrib.normals[3 * idx.normal_index + 1]
    };


    vertex.color = {
        attrib.colors[3 * idx.vertex_index + 0],
        attrib.colors[3 * idx.vertex_index + 1],
        attrib.colors[3 * idx.vertex_index + 2]
    };
    vertex.uv = {
        attrib.texcoords[2 * idx.texcoord_index + 0],
        1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
    };
    return vertex;
}

bool ResourceManager::loadGeometryFromObj(const std::filesystem::path &path, std::vector<MeshVertex> &vertexData) {

    auto absPath = Filesystem::AbsolutePath(path.string());

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    // Call the core loading procedure of TinyOBJLoader
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, absPath.c_str());

    // Check errors
    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        return false;
    }

    // Filling in vertexData:
    vertexData.clear();
    for (const auto& shape : shapes) {
        size_t offset = vertexData.size();
        vertexData.resize(offset + shape.mesh.indices.size());

        // Assume triangulated mesh
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            //size_t i0 = shape.mesh.indices[3 * f + 0].vertex_index;
            //size_t i1 = shape.mesh.indices[3 * f + 1].vertex_index;
            //size_t i2 = shape.mesh.indices[3 * f + 2].vertex_index;

            // Reverse winding: i0, i1, i2 → i0, i2, i1
            tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
            tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

            MeshVertex v0 = BuildVertex(idx0, attrib);
            MeshVertex v1 = BuildVertex(idx1, attrib);
            MeshVertex v2 = BuildVertex(idx2, attrib);

            vertexData.push_back(v0);
            vertexData.push_back(v1);
            vertexData.push_back(v2);
        }
    }

    populateTextureFrameAttributes(vertexData, std::nullopt);

    return true;
}

wgpu::Texture ResourceManager::loadTexture(const std::filesystem::path &path, wgpu::Device device, wgpu::TextureView* pTextureView) {
    int width, height, channels;
    unsigned char *pixelData = stbi_load(path.string().c_str(), &width, &height, &channels, 4 /* force 4 channels */);
    if (!pixelData) return nullptr;

    wgpu::TextureDescriptor textureDesc{};
    textureDesc.dimension = wgpu::TextureDimension::_2D;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm; // by convention for bmp, png and jpg file. Be careful with other formats.
    textureDesc.sampleCount = 1;
    textureDesc.size = { (unsigned int)width, (unsigned int)height, 1 };
    textureDesc.mipLevelCount = std::bit_width(std::max(textureDesc.size.width, textureDesc.size.height));
    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = nullptr;
    wgpu::Texture texture = device.createTexture(textureDesc);

    if (pTextureView) {
        wgpu::TextureViewDescriptor textureViewDesc{};
        textureViewDesc.aspect = wgpu::TextureAspect::All;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.baseMipLevel = 0;
        textureViewDesc.mipLevelCount = textureDesc.mipLevelCount;
        textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
        textureViewDesc.format = textureDesc.format;
        wgpu::TextureView view = texture.createView(textureViewDesc);
        *pTextureView = view;
    }

    writeMipMaps(device, texture, textureDesc.size, textureDesc.mipLevelCount, pixelData);

    stbi_image_free(pixelData);

    return texture;
}

wgpu::Texture ResourceManager::loadTexture(wgpu::Device device, int width, int height, int channels,
    unsigned char* pixelData, wgpu::TextureView* pTextureView)
{
    wgpu::TextureDescriptor textureDesc{};
    textureDesc.dimension = wgpu::TextureDimension::_2D;
    
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm; // by convention for bmp, png and jpg file. Be careful with other formats.
    
    textureDesc.sampleCount = 1;
    textureDesc.size = { (unsigned int)width, (unsigned int)height, 1 };
    textureDesc.mipLevelCount = std::bit_width(std::max(textureDesc.size.width, textureDesc.size.height));
    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = nullptr;
    wgpu::Texture texture = device.createTexture(textureDesc);

    if (pTextureView)
    {
        wgpu::TextureViewDescriptor textureViewDesc{};
        textureViewDesc.aspect = wgpu::TextureAspect::All;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.baseMipLevel = 0;
        textureViewDesc.mipLevelCount = textureDesc.mipLevelCount;
        textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
        textureViewDesc.format = textureDesc.format;
        *pTextureView = texture.createView(textureViewDesc);
    }
    
    writeMipMaps(device, texture, textureDesc.size, textureDesc.mipLevelCount, pixelData);
    return texture;
}

wgpu::ShaderModule ResourceManager::loadComputeShaderModule(const std::filesystem::path &path, wgpu::Device device, std::unordered_map<std::string, std::string>& formats)
{
    std::ifstream file(path);
    Assert::Check(file.is_open(), "file.is_open()", path.string());

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string shaderSource(size, ' ');
    file.seekg(0);
    file.read(shaderSource.data(), size);
    
    fmt::replace_all(shaderSource, formats);
    
    wgpu::ShaderSourceWGSL shaderCodeDesc{};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
    shaderCodeDesc.code = {shaderSource.c_str(), shaderSource.length()};
    //std::cout << shaderCodeDesc.code;

    wgpu::ShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    return device.createShaderModule(shaderDesc);
}

void ResourceManager::writeMipMaps(wgpu::Device device, wgpu::Texture texture, wgpu::Extent3D textureSize,
    uint32_t mipLevelCount, const unsigned char *pixelData) {
    wgpu::Queue queue = device.getQueue();

    // Arguments telling which part of the texture we upload to
    wgpu::TexelCopyTextureInfo destination;
    destination.texture = texture;
    destination.origin = { 0, 0, 0 };
    destination.aspect = wgpu::TextureAspect::All;

    // Arguments telling how the C++ side pixel memory is laid out
    wgpu::TexelCopyBufferLayout source;
    source.offset = 0;

    // Create image data
    wgpu::Extent3D mipLevelSize = textureSize;
    std::vector<unsigned char> previousLevelPixels;
    wgpu::Extent3D previousMipLevelSize;
    for (uint32_t level = 0; level < mipLevelCount; ++level) {
        // Pixel data for the current level
        std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
        if (level == 0) {
            // We cannot really avoid this copy since we need this
            // in previousLevelPixels at the next iteration
            memcpy(pixels.data(), pixelData, pixels.size());
        }
        else {
            // Create mip level data
            for (uint32_t i = 0; i < mipLevelSize.width; ++i) {
                for (uint32_t j = 0; j < mipLevelSize.height; ++j) {
                    unsigned char* p = &pixels[4 * (j * mipLevelSize.width + i)];
                    // Get the corresponding 4 pixels from the previous level
                    unsigned char* p00 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
                    unsigned char* p01 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
                    unsigned char* p10 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
                    unsigned char* p11 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
                    // Average
                    p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
                    p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
                    p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
                    p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
                }
            }
        }

        // Upload data to the GPU texture
        destination.mipLevel = level;
        source.bytesPerRow = 4 * mipLevelSize.width;
        source.rowsPerImage = mipLevelSize.height;
        queue.writeTexture(destination, pixels.data(), pixels.size(), source, mipLevelSize);

        previousLevelPixels = std::move(pixels);
        previousMipLevelSize = mipLevelSize;
        mipLevelSize.width /= 2;
        mipLevelSize.height /= 2;
    }

    queue.release();
}

glm::mat3x3 ResourceManager::computeTBN(const MeshVertex corners[3], const glm::vec3& expectedN) {
    using namespace glm;
    // What we call e in the figure
    vec3 ePos1 = corners[1].position - corners[0].position;
    vec3 ePos2 = corners[2].position - corners[0].position;

    // What we call \bar e in the figure
    vec2 eUV1 = corners[1].uv - corners[0].uv;
    vec2 eUV2 = corners[2].uv - corners[0].uv;

    vec3 T = normalize(ePos1 * eUV2.y - ePos2 * eUV1.y);
    vec3 B = normalize(ePos2 * eUV1.x - ePos1 * eUV2.x);
    vec3 N = cross(T, B);

    // Fix overall orientation
    if (dot(N, expectedN) < 0.0) {
        T = -T;
        B = -B;
        N = -N;
    }

    // Ortho-normalize the (T, B, expectedN) frame
    // a. "Remove" the part of T that is along expected N
    N = expectedN;
    T = normalize(T - dot(T, N) * N);
    // b. Recompute B from N and T
    B = cross(N, T);

    return mat3x3(T, B, N);
}

void ResourceManager::populateTextureFrameAttributes(std::vector<MeshVertex> &vertexData, optional_ref<const std::vector<uint16_t>> indices) {
    if (!indices) {
        using namespace glm;
        const size_t triangleCount = vertexData.size() / 3;
        // We compute the local texture frame per triangle
        for (int t = 0; t < (int)triangleCount; ++t) {
            MeshVertex* v = &vertexData[3 * t];

            for (int k = 0; k < 3; ++k) {
                mat3x3 TBN = computeTBN(v, v[k].normal);
                v[k].tangent = TBN[0];
                v[k].bitangent = TBN[1];
            }
        }
        return;
    }


    // Reset tangents and bitangents
    for (auto& vertex : vertexData) {
        vertex.tangent = glm::vec3(0.0f);
        vertex.bitangent = glm::vec3(0.0f);
    }

    // Iterate over each triangle
    for (size_t i = 0; i < indices->get().size(); i += 3) {
        MeshVertex& v0 = vertexData[indices->get()[i]];
        MeshVertex& v1 = vertexData[indices->get()[i + 1]];
        MeshVertex& v2 = vertexData[indices->get()[i + 2]];

        // Compute edges
        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;

        // Compute UV delta
        glm::vec2 deltaUV1 = v1.uv - v0.uv;
        glm::vec2 deltaUV2 = v2.uv - v0.uv;

        // Compute tangent and bitangent
        float det = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (glm::abs(det) < 1e-6f) {
            // Prevent division by zero by using a fallback
            det = 1.0f;
        } else {
            det = 1.0f / det;
        }

        glm::vec3 tangent = det * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
        glm::vec3 bitangent = det * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

        // Normalize the vectors
        tangent = glm::normalize(tangent);
        bitangent = glm::normalize(bitangent);

        // Accumulate tangent and bitangent for each vertex
        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;

        v0.bitangent += bitangent;
        v1.bitangent += bitangent;
        v2.bitangent += bitangent;
    }

    // Normalize the accumulated tangents and bitangents
    for (auto& vertex : vertexData) {
        vertex.tangent = glm::normalize(vertex.tangent);
        vertex.bitangent = glm::normalize(vertex.bitangent);
    }


}
