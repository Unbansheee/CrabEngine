//
// Created by Vinnie on 21/02/2025.
//
module;

#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>
#include <optional>
//#include <webgpu/webgpu.hpp> 

//#include "Application.h"
#include <mutex>

//#include "Resource.h"

export module Engine.Resource.ResourceManager;
import Engine.Resource;
import Engine.Resource.Importer.ImportSettings;
import Engine.WGPU;
import Engine.Types;

export struct MeshVertex;
export class ImportManager;

export class ResourceManager {
public:
    static bool IsSourceFile(const std::filesystem::path& path);

    template<typename T>
    static std::shared_ptr<T> Load(const std::filesystem::path& path) {
            return std::static_pointer_cast<T>(Load(path));
        }

    static std::shared_ptr<Resource> Load(const std::filesystem::path& path);

    static bool IsResourceLoaded(const std::filesystem::path& path);

    static void SaveToFile(const std::filesystem::path& path, nlohmann::json& json);
    static void SaveImportSettings(const std::filesystem::path& sourcePath, const std::shared_ptr<ImportSettings>& importSettings);
    static void SaveResource(const std::shared_ptr<Resource>& resource, 
                      const std::filesystem::path& path = {}) {
        auto savePath = path.empty() ? resource->GetSourcePath() : path;

        if (savePath.extension() == ".res") {
            nlohmann::json j;
            resource->Serialize(j);
            SaveToFile(savePath, j);
        }
        if (auto importSettings = resource->GetImportSettings()) {
            SaveImportSettings(savePath,
                 resource->GetImportSettings());
        }
    }

    template<typename T>
    void UnloadUnused() {
            std::lock_guard lock(cacheMutex);
            for(auto it = resourceCache.begin(); it != resourceCache.end(); ) {
                if(it->second.use_count() == 1) { // Only owned by cache
                    it = resourceCache.erase(it);
                } else {
                    ++it;
                }
            }
        }

    static std::vector<std::shared_ptr<Resource>> GetAllResources();
    
private:
    inline static std::mutex cacheMutex;
    inline static std::unordered_map<std::string, std::shared_ptr<Resource>> resourceCache;
    //TODO: Remove
public:
    static bool loadGeometryFromObj(const std::filesystem::path& path,
    std::vector<MeshVertex> &vertexData);

    static wgpu::Texture loadTexture(const std::filesystem::path& path, wgpu::Device device, wgpu::TextureView* pTextureView);
    static wgpu::Texture loadTexture(wgpu::Device device, int width, int height, int channels, unsigned char* pixelData, wgpu::TextureView* pTextureView);
    
    static wgpu::ShaderModule loadShaderModule(
    const std::filesystem::path& path,
    wgpu::Device device);

    static void populateTextureFrameAttributes(std::vector<MeshVertex> &vertexData, optional_ref<const std::vector<uint16_t>> indices = std::nullopt);
    static void writeMipMaps(
        wgpu::Device device,
        wgpu::Texture texture,
        wgpu::Extent3D textureSize,
    [[maybe_unused]] uint32_t mipLevelCount, // not used yet
    const unsigned char* pixelData);

private:
    static glm::mat3x3 computeTBN(const MeshVertex corners[3], const glm::vec3& expectedN);
};


