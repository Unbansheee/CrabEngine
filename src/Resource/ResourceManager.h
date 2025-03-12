//
// Created by Vinnie on 21/02/2025.
//

#pragma once
#include <cstdint>
#include <vector>
#include <filesystem>
#include <optional>
#include <webgpu/webgpu.hpp> 

//#include "Application.h"
#include <mutex>

#include "CrabTypes.h"
#include "Resource.h"
//#include "Resource.h"

class Resource;
struct MeshVertex;

class ResourceManager {
public:
    template<typename T>
    static std::shared_ptr<T> Load(const std::string& path) {
        std::lock_guard lock(cacheMutex);
        
        if(auto it = resourceCache.find(path); it != resourceCache.end()) {
            if(auto existing = std::dynamic_pointer_cast<T>(it->second)) {
                return existing;
            }
        }
        
        auto resource = std::make_shared<T>();
        resource->resourceFilePath = path;
        resourceCache[path] = resource;

        // Marked for lazy loading, actual loading happens on first access
        
        return resource;
    }

    static std::shared_ptr<Resource> Load(const std::string& path, const std::string& type) {
        std::lock_guard lock(cacheMutex);
        
        if(auto it = resourceCache.find(path); it != resourceCache.end()) {
            if(auto existing = std::dynamic_pointer_cast<Resource>(it->second)) {
                return existing;
            }
        }

        auto classData = ClassDB::Get().GetClassByName(type);
        auto r = dynamic_cast<Resource*>(classData->Initializer());
        std::shared_ptr<Resource> resource;
        resource.reset(r);
        resource->resourceFilePath = path;
        resourceCache[path] = resource;

        // Marked for lazy loading, actual loading happens on first access
        
        return resource;
    }
    
    template<typename T>
    static std::shared_ptr<T> Get(const std::string& path) {
        auto res = resourceCache[path];
        if (!res || !res->IsLoaded()) {
            res = LoadImmediately<T>(path);
        }
        return std::dynamic_pointer_cast<T>(res);
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
    template<typename T>
    static std::shared_ptr<T> LoadImmediately(const std::string& path)
    {
        auto res = Load<T>(path);
        res->LoadData();
        return res;
    }

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


