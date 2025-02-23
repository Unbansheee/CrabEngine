//
// Created by Vinnie on 21/02/2025.
//

#pragma once
#include <cstdint>
#include <vector>
#include <filesystem>
#include <optional>
#include <webgpu/webgpu.hpp>

#include "Application.h"

struct VertexData;

class ResourceManager {

public:
    static bool loadGeometryFromObj(const std::filesystem::path& path,
        std::vector<VertexData> &vertexData);

    static wgpu::Texture loadTexture(const std::filesystem::path& path, wgpu::Device device, wgpu::TextureView* pTextureView);

    static wgpu::ShaderModule loadShaderModule(
    const std::filesystem::path& path,
    wgpu::Device device);

    static void populateTextureFrameAttributes(std::vector<VertexData> &vertexData, optional_ref<const std::vector<uint16_t>> indices = std::nullopt);
private:
    static void writeMipMaps(
        wgpu::Device device,
        wgpu::Texture texture,
        wgpu::Extent3D textureSize,
    [[maybe_unused]] uint32_t mipLevelCount, // not used yet
    const unsigned char* pixelData);

    static glm::mat3x3 computeTBN(const VertexData corners[3], const glm::vec3& expectedN);


};



